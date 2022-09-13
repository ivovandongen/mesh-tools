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
        auto positionsView = mesh.vertexAttribute<glm::vec3>(models::AttributeType::POSITION);
        std::vector<glm::vec3> positions{positionsView.begin(), positionsView.end()};
        meshDecl.vertexCount = (uint32_t) positionsView.size();
        meshDecl.vertexPositionData = positions.data();
        meshDecl.vertexPositionStride = positionsView.stride();
        if (mesh.hasVertexAttribute(models::AttributeType::NORMAL)) {
            auto normalsView = mesh.vertexAttribute<glm::vec3>(models::AttributeType::NORMAL);
            std::vector<glm::vec3> normals{normalsView.begin(), normalsView.end()};
            meshDecl.vertexNormalData = normals.data();
            meshDecl.vertexNormalStride = normalsView.stride();
        }
        if (mesh.hasVertexAttribute(models::AttributeType::TEXCOORD)) {
            auto uvsView = mesh.vertexAttribute<glm::vec3>(models::AttributeType::NORMAL);
            std::vector<glm::vec3> uvs{uvsView.begin(), uvsView.end()};
            meshDecl.vertexUvData = uvs.data();
            meshDecl.vertexUvStride = uvsView.stride();
        }

        auto& indexData = mesh.indices();
        meshDecl.indexCount = (uint32_t) indexData.count();
        meshDecl.indexData = indexData.buffer().data();
        meshDecl.indexFormat = indexData.dataType() == models::DataType::U_INT ? xatlas::IndexFormat::UInt32 : xatlas::IndexFormat::UInt16;

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

    if (atlas->chartCount == 0) {
        return {"Something went wrong generating the atlas"};
    }

    logging::debug("Atlas generated {}x{}, utilization: {}", atlas->width, atlas->height, *atlas->utilization);
    return result;
}

void Atlas::apply(models::Model& model) {
    const float uscale = 1.f / width();
    const float vscale = 1.f / height();

    for (size_t i = 0; i < model.meshes().size(); i++) {
        auto& atlasMesh = impl_->atlas_->meshes[i];
        auto& modelMesh = model.meshes()[i];
        auto& positionVA = modelMesh.vertexAttribute(models::AttributeType::POSITION);
        auto& normalVA = modelMesh.vertexAttribute(models::AttributeType::NORMAL);

        // Update all vertex attributes (except Texcoords)
        std::unordered_map<models::AttributeType, models::TypedData> vertexData;
        for (auto key : modelMesh.vertexAttributes()) {
            if (key == models::AttributeType::TEXCOORD) {
                continue;
            }
            auto& org = modelMesh.vertexAttribute(key);
            vertexData[key] = {org.dataType(), org.componentCount(), atlasMesh.vertexCount};
        }

        std::vector<glm::vec2> uvs;
        uvs.reserve(atlasMesh.vertexCount);
        for (size_t vert = 0; vert < atlasMesh.vertexCount; vert++) {
            const auto& ivert = atlasMesh.vertexArray[vert];
            for (auto& va : vertexData) {
                auto* src = &modelMesh.vertexAttribute(va.first)[ivert.xref];
                memcpy(&va.second[vert], src, va.second.stride());
            }
            uvs.emplace_back(ivert.uv[0] * uscale, ivert.uv[1] * vscale);
        }

        // Set the UVs
        auto& texcoords = vertexData[models::AttributeType::TEXCOORD] = models::TypedData{models::DataType::FLOAT, 2, uvs.size()};
        // TODO: get raw view on vector and copy in one go
        for (size_t index = 0; index < uvs.size(); index++) {
            memcpy(&texcoords[index], &uvs[index], texcoords.stride());
        }

        // Set new index buffer
        // TODO: get raw view on vector and copy in one go
        models::TypedData indices{models::DataType::U_INT, 1, atlasMesh.indexCount};
        for (size_t index = 0; index < atlasMesh.indexCount; index++) {
            memcpy(&indices[index], atlasMesh.indexArray + index, sizeof(int32_t));
        }
        modelMesh.indices(std::move(indices));

        // Set all the updated vertex data
        modelMesh.vertexData(std::move(vertexData));

#ifndef NDEBUG
        assert(modelMesh.indices().count() == atlasMesh.indexCount);
        for (auto& va: modelMesh.vertexData()) {
            assert(va.second.count() == atlasMesh.vertexCount);
        }
#endif
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
