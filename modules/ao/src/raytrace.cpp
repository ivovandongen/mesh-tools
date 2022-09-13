#include "raytrace.hpp"

#include "rasterize.hpp"

#include <meshtools/logging.hpp>
#include <meshtools/math.hpp>
#include <meshtools/models/model.hpp>

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>

#include <array>
#include <cassert>
#include <cmath>
#include <random>
#include <vector>

using namespace std;

namespace meshtools::ao {

template<class T>
T sign(std::array<T, 2> p1, std::array<T, 2> p2, std::array<T, 2> p3) {
    return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1]);
}

// http://www.altdevblogaday.com/2012/05/03/generating-uniformly-distributed-points-on-sphere/
glm::vec3 random_direction(std::mt19937& rnd) {
    float z = 2.0f * rnd() / std::mt19937::max() - 1.0f;
    float t = 2.0f * rnd() / std::mt19937::max() * M_PI;
    float r = sqrtf(1.0f - z * z);
    return {r * cosf(t), r * sinf(t), z};
}

Result<Image> raytrace(const models::Model& model, const Size<uint32_t>& size, RaytraceOptions options) {
    if (std::any_of(model.meshes().begin(), model.meshes().end(), [](const models::Mesh& mesh) {
            return !mesh.hasVertexAttribute(models::AttributeType::TEXCOORD);
        })) {
        return {"Cannot raytrace models without texture coordinates"};
    }

    if (std::any_of(model.meshes().begin(), model.meshes().end(), [](const models::Mesh& mesh) {
            return !mesh.hasVertexAttribute(models::AttributeType::NORMAL);
        })) {
        return {"Cannot raytrace models without normals"};
    }

    logging::debug("Ray trace - setting up scene");

    // Create the embree device and scene.
    RTCDevice device = rtcNewDevice(nullptr);
    assert(device && "Unable to create embree device.");
    RTCScene scene = rtcNewScene(device);
    assert(scene);

    for (const auto& mesh : model.meshes()) {
        auto& positions = mesh.vertexAttribute(models::AttributeType::POSITION);
        // Populate the embree mesh.
        // TODO: Backface culling
        auto* geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
        assert(geometry);
        assert(positions.dataType() == models::DataType::FLOAT);
        assert(positions.componentCount() == 3);
        auto* vertices =
                rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, positions.stride(), positions.count());
        assert(vertices);
        memcpy(vertices, positions.buffer().data(), positions.buffer().size());

        auto indices = mesh.indices<uint32_t>();
        auto* triangles =
                rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof (uint32_t), indices.size() / 3);
        assert(triangles);
        memcpy(triangles, indices.raw(), sizeof(uint32_t) * indices.size());

        rtcCommitGeometry(geometry);
        rtcAttachGeometry(scene, geometry);
        rtcReleaseGeometry(geometry);
    }


    rtcCommitScene(scene);

    auto image = std::make_shared<Image>(size.width, size.width, options.resultChannels);
    const float uscale = size.width;
    const float vscale = size.height;

    const float E = 0.5f;
    std::vector<RTCRay> rays;
    rays.resize(options.nsamples);

    RTCIntersectContext ctx{};
    rtcInitIntersectContext(&ctx);

    // Prep "random dirs"
    std::mt19937 rnd(0);
    std::vector<glm::vec3> randomDirs;
    randomDirs.reserve(options.nsamples);
    for (size_t i = 0; i < options.nsamples; i++) {
        randomDirs.push_back(random_direction(rnd));
    }

    for (const auto& mesh : model.meshes()) {
        assert(mesh.hasVertexAttribute(models::AttributeType::TEXCOORD));
        assert(mesh.vertexAttribute(models::AttributeType::POSITION).count() ==
               mesh.vertexAttribute(models::AttributeType::TEXCOORD).count());
        auto indexCount = mesh.indices().count();
        logging::debug("Ray trace - tracing {} triangles", indexCount / 3);

        auto indexView = mesh.indices<uint32_t>();
        auto positionsView = mesh.vertexAttribute<glm::vec3>(models::AttributeType::POSITION);
        auto normalsView = mesh.vertexAttribute<glm::vec3>(models::AttributeType::NORMAL);
        auto texcoordsView = mesh.vertexAttribute<glm::vec2>(models::AttributeType::TEXCOORD);

        for (uint32_t j = 0; j < indexCount; j += 3) {
            //            logging::debug("Rasterizing {}/{}", j / 3, indexCount / 3);
            struct Vertex {
                Vertex(glm::vec3 pos, glm::vec3 norm, glm::vec2 tex)
                    : position(pos), normal(norm), uv(tex), uv3({tex[0], tex[1], 0.0f}),
                      uv2i({static_cast<int>(tex[0]), static_cast<int>(tex[1])}){

                      };
                Vertex() = default;

                glm::vec3 position;
                glm::vec3 normal;
                glm::vec2 uv;
                glm::vec3 uv3;
                glm::vec<2, int, glm::defaultp> uv2i;
            };
            std::array<Vertex, 3> triangle{};
            for (size_t k = 0; k < 3; k++) {
                auto index = indexView[j + k];
                assert(texcoordsView.size() > index);
                auto uv = texcoordsView[index];
                uv[0] *= uscale;
                uv[1] *= vscale;
                assert(uv[0] <= image->width());
                assert(uv[1] <= image->height());
                triangle[k] = {positionsView[index], normalsView[index], uv};
            }

            if (!triangle[0].uv3[0] && !triangle[0].uv3[1] && !triangle[1].uv3[0] && !triangle[1].uv3[1] && !triangle[2].uv3[0] &&
                !triangle[2].uv3[1]) {
                continue; // Skip triangles that weren't atlased.
            }

            const constexpr int buffer = 0;
            RasterizeTriangle<buffer>(triangle[0].uv2i, triangle[1].uv2i, triangle[2].uv2i, [&](auto x, auto y) {
                if (x < 0 || y < 0 || !(x < image->width()) || !(y < image->height())) {
                    return;
                }

                // Interpolate normal
                auto bc = barycentric(triangle[0].uv3,
                                      triangle[1].uv3,
                                      triangle[2].uv3,
                                      glm::vec3{static_cast<float>(x), static_cast<float>(y), 0});
                glm::vec3 normal{
                        triangle[0].normal[0] * bc[0] + triangle[1].normal[0] * bc[1] + triangle[2].normal[0] * bc[2],
                        triangle[0].normal[1] * bc[0] + triangle[1].normal[1] * bc[1] + triangle[2].normal[1] * bc[2],
                        triangle[0].normal[2] * bc[0] + triangle[1].normal[2] * bc[1] + triangle[2].normal[2] * bc[2],
                };
                normal = glm::normalize(normal);

                // Interpolate origin
                glm::vec3 org{triangle[0].position[0] * bc[0] + triangle[1].position[0] * bc[1] + triangle[2].position[0] * bc[2],
                              triangle[0].position[1] * bc[0] + triangle[1].position[1] * bc[1] + triangle[2].position[1] * bc[2],
                              triangle[0].position[2] * bc[0] + triangle[1].position[2] * bc[1] + triangle[2].position[2] * bc[2]};

                // Prepare rays to shoot through the differential hemisphere.
                for (size_t i = 0; i < rays.size(); i++) {
                    auto& ray = rays[i];
                    ray.org_x = org[0];
                    ray.org_y = org[1];
                    ray.org_z = org[2];
                    ray.tnear = E;
                    ray.tfar = options.far;

                    auto dir = randomDirs[i];
                    if (glm::dot(dir, normal) < 0.0)
                        dir = scale(dir, -1.0f);

                    ray.dir_x = dir[0];
                    ray.dir_y = dir[1];
                    ray.dir_z = dir[2];
                }
                rtcOccluded1M(scene, &ctx, rays.data(), options.nsamples, sizeof(RTCRay));

                int nhits = 0;
                for (auto& ray : rays) {
                    if (ray.tfar == -std::numeric_limits<float>::infinity()) {
                        nhits++;
                    }
                }

                float ao = options.multiply * (1.0f - (float) nhits / (float) options.nsamples);
                uint8_t result = std::min(255.0f, 255.0f * ao);
                std::vector<uint8_t> color{result};
                if (image->channels() == 4) {
                    color[3] = 255;
                }
                SetPixel(image->data().data(), image->width(), x, y, color);
            });
        }
    }

    // Free all embree data.
    rtcReleaseScene(scene);
    rtcReleaseDevice(device);

    return Result<Image>{std::move(image)};
}

} // namespace meshtools::ao