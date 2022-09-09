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
    std::vector<B> b{a.size() * sizeof(A) / sizeof(B)};

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


    std::vector<Mesh> meshes;
    meshes.reserve(shapes.size());
    for (auto& shape : shapes) {
        meshes.emplace_back(shape.name,
                            std::move(shape.mesh.indices),
                            copy<glm::vec3>(shape.mesh.positions),
                            copy<glm::vec3>(shape.mesh.normals),
                            copy<glm::vec2>(shape.mesh.texcoords));
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
    for (size_t i = 0; i < model.meshes().size(); i++) {
        auto& modelMesh = model.meshes()[i];
        if (modelMesh.name().empty()) {
            fprintf(outobj, "o Object-%zu\n", i);
        } else {
            fprintf(outobj, "o %s\n", modelMesh.name().c_str());
        }

        // Vertex data
        for (size_t nvert = 0; nvert < modelMesh.positions().size(); nvert++) {
            const auto& position = modelMesh.positions()[nvert];

            // Position
            fprintf(outobj, "v %f %f %f\n", position[0], position[1], position[2]);

            // Normal
            if (!modelMesh.normals().empty()) {
                const auto& normal = modelMesh.normals()[nvert];
                fprintf(outobj, "vn %f %f %f\n", normal[0], normal[1], normal[2]);
            }

            // UV
            if (!modelMesh.texcoords().empty()) {
                const auto& uv = modelMesh.texcoords()[nvert];
                // Flip y
                fprintf(outobj, "vt %f %f\n", uv[0], 1 - uv[1]);
            }
        }

        // Faces
        fprintf(outobj, "s %d\n", 0);
        for (int nface = 0; nface < modelMesh.indices().size() / 3; nface++) {
            int a = vertexCountBase + modelMesh.indices()[nface * 3] + 1;
            int b = vertexCountBase + modelMesh.indices()[nface * 3 + 1] + 1;
            int c = vertexCountBase + modelMesh.indices()[nface * 3 + 2] + 1;
            if (modelMesh.normals().empty()) {
                fprintf(outobj, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
            } else {
                fprintf(outobj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", a, a, a, b, b, b, c, c, c);
            }
        }
        vertexCountBase += modelMesh.positions().size();
    }
}

} // namespace meshtools::models::obj