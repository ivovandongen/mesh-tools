#include "meshtools/uv/atlas.hpp"

#include <meshtools/logging.hpp>
#include <meshtools/models/model.hpp>

#include <xatlas.h>

namespace meshtools::uv {

class Atlas::Impl {
public:
    Impl() {
        atlas_ = xatlas::Create();
    }

    ~Impl() {
        xatlas::Destroy(atlas_);
    }

    xatlas::Atlas* atlas_;
};

Atlas::Atlas() : impl_(std::make_unique<Atlas::Impl>()) {}

Atlas::~Atlas() = default;

Result<Atlas> Atlas::Create(const models::Model& model, const AtlasCreateOptions& options) {
    Result<Atlas> result{std::make_shared<Atlas>()};
    xatlas::Atlas* atlas = result.value->impl_->atlas_;

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
    xatlas::Generate(atlas,
                     {},
                     {.maxChartSize = options.maxSize,
                      .padding = options.padding,
                      .texelsPerUnit = options.texelsPerUnit,
                      .resolution = options.resolution,
                      .bilinear = options.bilinear,
                      .blockAlign = options.blockAlign,
                      .bruteForce = options.bruteForce});
    logging::debug("Atlas generated {}x{}, utilization: {}", atlas->width, atlas->height, *atlas->utilization);

    return result;
}

void Atlas::apply(models::Model& model) {
    const float uscale = 1.f / width();
    const float vscale = 1.f / height();

    for (size_t i = 0; i < model.meshes().size(); i++) {
        auto& atlasMesh = impl_->atlas_->meshes[i];
        auto& modelMesh = model.meshes()[i];
        std::vector<vec3> positions;
        std::vector<vec3> normals;
        std::vector<vec2> uvs;
        positions.reserve(atlasMesh.vertexCount);
        normals.reserve(atlasMesh.vertexCount);
        uvs.reserve(atlasMesh.vertexCount);
        for (size_t vert = 0; vert < atlasMesh.vertexCount; vert++) {
            const auto& ivert = atlasMesh.vertexArray[vert];
            positions.push_back(modelMesh.positions()[ivert.xref]);
            normals.push_back(modelMesh.normals()[ivert.xref]);
            uvs.push_back({ivert.uv[0] * uscale, 1 - ivert.uv[1] * vscale});
        }
        modelMesh.indices({atlasMesh.indexArray, atlasMesh.indexArray + atlasMesh.indexCount});
        modelMesh.positions(std::move(positions));
        modelMesh.normals(std::move(normals));
        modelMesh.texcoords(std::move(uvs));
    }
}

uint32_t Atlas::width() const {
    return impl_->atlas_->width;
}

uint32_t Atlas::height() const {
    return impl_->atlas_->height;
}

size_t Atlas::meshCount() const {
    return impl_->atlas_->meshCount;
}

size_t Atlas::indexCount(size_t meshIdx) const {
    return (impl_->atlas_->meshes + meshIdx)->indexCount;
}

Vertex Atlas::vertex(size_t meshIdx, size_t indexIdx) const {
    auto* atlasMesh = (impl_->atlas_->meshes + meshIdx);
    auto* xVertex = &atlasMesh->vertexArray[atlasMesh->indexArray[indexIdx]];
    return {{xVertex->uv[0], xVertex->uv[1]}, xVertex->xref};
}

} // namespace meshtools::uv
