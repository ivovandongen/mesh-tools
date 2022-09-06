#include "raytrace.hpp"

#include "rasterize.hpp"

#include <meshtools/logging.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/vec3.hpp>

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#include <xatlas.h>


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

template<class T>
bool pointInTriangle(std::array<T, 2> pt, std::array<T, 2> v1, std::array<T, 2> v2, std::array<T, 2> v3) {
    T d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

// http://www.altdevblogaday.com/2012/05/03/generating-uniformly-distributed-points-on-sphere/
vec3 random_direction(std::mt19937& rnd) {
    float z = 2.0f * rnd() / std::mt19937::max() - 1.0f;
    float t = 2.0f * rnd() / std::mt19937::max() * M_PI;
    float r = sqrtf(1.0f - z * z);
    return {r * cosf(t), r * sinf(t), z};
}

Result<Image> raytrace(const models::Model& model, const xatlas::Atlas& atlas, RaytraceOptions options) {

    logging::debug("Ray trace - setting up scene");

    // Create the embree device and scene.
    RTCDevice device = rtcNewDevice(nullptr);
    assert(device && "Unable to create embree device.");
    RTCScene scene = rtcNewScene(device);
    assert(scene);

    for (const auto& mesh : model.meshes()) {
        const auto& verts = mesh.positions();
        const vector<uint32_t>& indices = mesh.indices();
        // Populate the embree mesh.
        // TODO: Backface culling
        auto* geometry = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
        assert(geometry);
        auto* vertices =
                (float*) rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(vec3), verts.size());
        assert(vertices);
        memcpy(vertices, verts.data(), sizeof(vec3) * verts.size());

        auto* triangles = (uint32_t*)
                rtcSetNewGeometryBuffer(geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(uint32_t), indices.size() / 3);
        memcpy(triangles, indices.data(), sizeof(uint32_t) * indices.size());

        rtcCommitGeometry(geometry);
        rtcAttachGeometry(scene, geometry);
        rtcReleaseGeometry(geometry);
    }


    rtcCommitScene(scene);

    auto image = std::make_shared<Image>(atlas.width, atlas.height, options.resultChannels);
    const float E = 0.5f;
    std::vector<RTCRay> rays;
    rays.resize(options.nsamples);

    RTCIntersectContext ctx{};
    rtcInitIntersectContext(&ctx);

    // Prep "random dirs"
    std::mt19937 rnd(0);
    std::vector<vec3> randomDirs;
    randomDirs.reserve(options.nsamples);
    for (size_t i = 0; i < options.nsamples; i++) {
        randomDirs.push_back(random_direction(rnd));
    }

    for (size_t meshIdx = 0; meshIdx < atlas.meshCount; meshIdx++) {
        xatlas::Mesh* atlasMesh = atlas.meshes + meshIdx;
        logging::debug("Ray trace - tracing {} triangles", atlasMesh->indexCount / 3);

        for (uint32_t j = 0; j < atlasMesh->indexCount; j += 3) {
            std::array<xatlas::Vertex*, 3> triangle{};
            for (int k = 0; k < 3; k++) {
                triangle[k] = &atlasMesh->vertexArray[atlasMesh->indexArray[j + k]];
            }
            if (!triangle[0]->uv[0] && !triangle[0]->uv[1] && !triangle[1]->uv[0] && !triangle[1]->uv[1] && !triangle[2]->uv[0] &&
                !triangle[2]->uv[1]) {
                continue; // Skip triangles that weren't atlased.
            }

            const auto& uv0v = vec3{triangle[0]->uv[0], triangle[0]->uv[1], 0};
            const auto& uv1v = vec3{triangle[1]->uv[0], triangle[1]->uv[1], 0};
            const auto& uv2v = vec3{triangle[2]->uv[0], triangle[2]->uv[1], 0};

            const auto& modelMesh = model.meshes()[meshIdx];

            auto v0 = *(modelMesh.positions().data() + triangle[0]->xref);
            auto v1 = *(modelMesh.positions().data() + triangle[1]->xref);
            auto v2 = *(modelMesh.positions().data() + triangle[2]->xref);

            auto n0 = *(modelMesh.normals().data() + triangle[0]->xref);
            auto n1 = *(modelMesh.normals().data() + triangle[1]->xref);
            auto n2 = *(modelMesh.normals().data() + triangle[2]->xref);

            std::array<int, 2> uv0{static_cast<int>(triangle[0]->uv[0]), static_cast<int>(triangle[0]->uv[1])};
            std::array<int, 2> uv1{static_cast<int>(triangle[1]->uv[0]), static_cast<int>(triangle[1]->uv[1])};
            std::array<int, 2> uv2{static_cast<int>(triangle[2]->uv[0]), static_cast<int>(triangle[2]->uv[1])};

            const constexpr int buffer = 0;
            RasterizeTriangle<buffer>(uv0, uv1, uv2, [&](auto x, auto y) {
                if (x < 0 || y < 0 || !(x < image->width()) || !(y < image->height())) {
                    return;
                }

                // Interpolate normal
                auto bc = barycentric(uv0v, uv1v, uv2v, vec3{static_cast<float>(x), static_cast<float>(y), 0});
                vec3 normal{
                        n0[0] * bc[0] + n1[0] * bc[1] + n2[0] * bc[2],
                        n0[1] * bc[0] + n1[1] * bc[1] + n2[1] * bc[2],
                        n0[2] * bc[0] + n1[2] * bc[1] + n2[2] * bc[2],
                };
                normal = vec3Normalize(normal);

                // Interpolate origin
                vec3 org{v0[0] * bc[0] + v1[0] * bc[1] + v2[0] * bc[2],
                         v0[1] * bc[0] + v1[1] * bc[1] + v2[1] * bc[2],
                         v0[2] * bc[0] + v1[2] * bc[1] + v2[2] * bc[2]};

                // Prepare rays to shoot through the differential hemisphere.
                for (size_t i = 0; i < rays.size(); i++) {
                    auto& ray = rays[i];
                    ray.org_x = org[0];
                    ray.org_y = org[1];
                    ray.org_z = org[2];
                    ray.tnear = E;
                    ray.tfar = options.far;

                    auto dir = randomDirs[i];
                    if (vec3Dot(dir, normal) < 0.0)
                        dir = vec3Scale(dir, -1.0);

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