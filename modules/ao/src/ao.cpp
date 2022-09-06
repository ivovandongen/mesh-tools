#include <meshtools/ao/ao.hpp>

#include <meshtools/logging.hpp>
#include <meshtools/models/model.hpp>

#include "rasterize.hpp"
#include "raytrace.hpp"

#include <xatlas.h>

#include <cassert>

namespace meshtools::ao {

Atlas::Atlas() {
    impl = xatlas::Create();
}

Atlas::~Atlas() {
    logging::warn("Destroying atlas");
    xatlas::Destroy(impl);
}

Result<Atlas> createAtlas(const models::Model& model) {
    Result<Atlas> result{std::make_shared<Atlas>()};
    xatlas::Atlas* atlas = result.value->impl;

    // Add meshes to atlas.
    uint32_t totalVertices = 0, totalFaces = 0;
    for (const auto& mesh : model.meshes()) {
        xatlas::MeshDecl meshDecl;
        meshDecl.vertexCount = (uint32_t) mesh.positions().size();
        meshDecl.vertexPositionData = mesh.positions().data();
        meshDecl.vertexPositionStride = sizeof(vec3);
        if (!mesh.normals().empty()) {
            meshDecl.vertexNormalData = mesh.normals().data();
            meshDecl.vertexNormalStride = sizeof(vec3);
        }
        if (!mesh.texcoords().empty()) {
            meshDecl.vertexUvData = mesh.texcoords().data();
            meshDecl.vertexUvStride = sizeof(vec2);
        }
        meshDecl.indexCount = (uint32_t) mesh.indices().size();
        meshDecl.indexData = mesh.indices().data();
        meshDecl.indexFormat = xatlas::IndexFormat::UInt32;

        xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl, (uint32_t) model.meshes().size());
        if (error != xatlas::AddMeshError::Success) {
            xatlas::Destroy(atlas);
            logging::error("Error adding mesh {}: {}", mesh.name().c_str(), xatlas::StringForEnum(error));
            result.error = xatlas::StringForEnum(error);
            return result;
        }
        totalVertices += meshDecl.vertexCount;
        if (meshDecl.faceCount > 0)
            totalFaces += meshDecl.faceCount;
        else
            totalFaces += meshDecl.indexCount / 3; // Assume triangles if MeshDecl::faceCount not specified.
    }

    logging::debug("Generating atlas");
    xatlas::Generate(atlas, {}, {.texelsPerUnit = 0, .resolution = 512, .padding = 2, .blockAlign = true});
    logging::debug("Atlas generated {}x{}, utilization: {}", atlas->width, atlas->height, *atlas->utilization);

    return result;
}

Result<Image> bake(const models::Model& model, const Atlas& atlas, const BakeOptions& options) {
    assert(atlas.impl);
    const constexpr uint8_t channels = 1;
    const constexpr float maxFar = 5;
    RaytraceOptions raytraceOptions{.nsamples = options.nsamples, .resultChannels = channels, .far = maxFar, .multiply = options.multiply};
    auto raytraceResult = raytrace(model, *atlas.impl, raytraceOptions);
    if (!raytraceResult) {
        return {std::move(raytraceResult.error)};
    }

    return {std::move(raytraceResult.value)};
}

} // namespace meshtools::ao
