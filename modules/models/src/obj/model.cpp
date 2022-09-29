#include "model.hpp"

#include <meshtools/file.hpp>
#include <meshtools/logging.hpp>

#include "tiny_obj_loader.h"

namespace meshtools::models::obj {

template<class B, class A>
std::vector<B> copy(const std::vector<A>& a) {
    if (a.empty()) {
        return {};
    }
    std::vector<B> b(a.size() * sizeof(A) / sizeof(B));

    memcpy(b.data(), a.data(), a.size() * sizeof(A));

    return b;
}

ModelLoadResult loadModel(const std::filesystem::path& file) {
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;
    auto loaded = tinyobj::LoadObj(shapes, materials, error, file.c_str());

    ModelLoadResult result;

    if (!loaded) {
        result.error = std::move(error);
        return result;
    }


    std::vector<MeshGroup> meshes;
    meshes.reserve(shapes.size());
    for (auto& shape : shapes) {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData{DataType::FLOAT, 3, copy<unsigned char>(shape.mesh.positions)};
        vertexData[AttributeType::NORMAL] = TypedData{DataType::FLOAT, 3, copy<unsigned char>(shape.mesh.normals)};
        vertexData[AttributeType::TEXCOORD] = TypedData{DataType::FLOAT, 2, copy<unsigned char>(shape.mesh.texcoords)};

        Mesh mesh{shape.name,
                  -1, // TODO: Material
                  TypedData{DataType::U_INT, 1, copy<unsigned char>(shape.mesh.indices)},
                  std::move(vertexData)};
        meshes.emplace_back(shape.name, std::move(mesh), Extras{});
    }

    result.value = std::make_shared<Model>(std::move(meshes));
    return result;
}

void dump(const Model& model, const Image& aoMap, const std::filesystem::path& file) {
    if (!file::parentDirExists(file)) {
        logging::error("Directory for {} does not exist", file.c_str());
        return;
    }

    FILE* outobj = fopen(file.c_str(), "wt");

    size_t vertexCountBase = 0;
    for (auto& meshGroup : model.meshGroups()) {
        for (size_t i = 0; i < meshGroup.meshes().size(); i++) {
            auto& modelMesh = meshGroup.meshes()[i];
            if (modelMesh.name().empty()) {
                fprintf(outobj, "o Object-%zu\n", i);
            } else {
                fprintf(outobj, "o %s\n", modelMesh.name().c_str());
            }

            // Vertex data
            auto positions = modelMesh.vertexAttribute<glm::vec3>(AttributeType::POSITION);
            auto normals = modelMesh.hasVertexAttribute(AttributeType::NORMAL)
                                   ? std::optional<DataView<glm::vec3>>{modelMesh.vertexAttribute<glm::vec3>(AttributeType::NORMAL)}
                                   : std::nullopt;
            auto texcoords = modelMesh.hasVertexAttribute(AttributeType::TEXCOORD)
                                     ? std::optional<DataView<glm::vec2>>{modelMesh.vertexAttribute<glm::vec2>(AttributeType::TEXCOORD)}
                                     : std::nullopt;
            for (size_t nvert = 0; nvert < positions.size(); nvert++) {
                const auto& position = positions[nvert];

                // Position
                fprintf(outobj, "v %f %f %f\n", position[0], position[1], position[2]);

                // Normal
                if (normals) {
                    const auto& normal = (*normals)[nvert];
                    fprintf(outobj, "vn %f %f %f\n", normal[0], normal[1], normal[2]);
                }

                // UV
                if (texcoords) {
                    const auto& uv = (*texcoords)[nvert];
                    // Flip y
                    fprintf(outobj, "vt %f %f\n", uv[0], 1 - uv[1]);
                }
            }

            // Faces
            fprintf(outobj, "s %d\n", 0);
            auto indices = modelMesh.indices<uint32_t>();
            for (int nface = 0; nface < indices.size() / 3; nface++) {
                int a = vertexCountBase + indices[nface * 3] + 1;
                int b = vertexCountBase + indices[nface * 3 + 1] + 1;
                int c = vertexCountBase + indices[nface * 3 + 2] + 1;
                if (normals) {
                    fprintf(outobj, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
                } else {
                    fprintf(outobj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", a, a, a, b, b, b, c, c, c);
                }
            }
            vertexCountBase += positions.size();
        }
    }
}

} // namespace meshtools::models::obj