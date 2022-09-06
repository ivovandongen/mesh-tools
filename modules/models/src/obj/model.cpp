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
    //    auto loaded = tinyobj::LoadObj(shapes, materials, error, file.c_str(), nullptr, 0u);

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
                            copy<vec3>(shape.mesh.positions),
                            copy<vec3>(shape.mesh.normals),
                            copy<vec2>(shape.mesh.texcoords));
    }

    result.value = std::make_shared<Model>(std::move(meshes));
    return result;
}

void dump(const Model& model, const xatlas::Atlas& atlas, const Image& aoMap, const std::filesystem::path& file) {
    if (!file::parentDirExists(file)) {
        logging::error("Directory for {} does not exist", file.c_str());
        return;
    }

    FILE* outobj = fopen(file.c_str(), "wt");

    size_t vertexCountBase = 0;
    for (size_t i = 0; i < model.meshes().size(); i++) {
        auto* atlasMesh = atlas.meshes + i;
        auto& modelMesh = model.meshes()[i];
        float uscale = 1.f / atlas.width;
        float vscale = 1.f / atlas.height;
        if (modelMesh.name().empty()) {
            fprintf(outobj, "o Object-%zu\n", i);
        } else {
            fprintf(outobj, "o %s\n", modelMesh.name().c_str());
        }

        // Vertex data
        for (int nvert = 0; nvert < atlasMesh->vertexCount; nvert++) {
            const auto& ivert = atlasMesh->vertexArray[nvert];
            const auto& position = modelMesh.positions()[ivert.xref];

            // Position
            fprintf(outobj, "v %f %f %f\n", position[0], position[1], position[2]);

            // Normal
            if (!modelMesh.normals().empty()) {
                const auto& normal = modelMesh.normals()[ivert.xref];
                fprintf(outobj, "vn %f %f %f\n", normal[0], normal[1], normal[2]);
            }

            // UV
            fprintf(outobj, "vt %f %f\n", ivert.uv[0] * uscale, 1 - ivert.uv[1] * vscale);
        }

        // Faces
        fprintf(outobj, "s %d\n", 0);
        for (int nface = 0; nface < atlasMesh->indexCount / 3; nface++) {
            int a = vertexCountBase + atlasMesh->indexArray[nface * 3] + 1;
            int b = vertexCountBase + atlasMesh->indexArray[nface * 3 + 1] + 1;
            int c = vertexCountBase + atlasMesh->indexArray[nface * 3 + 2] + 1;
            if (modelMesh.normals().empty()) {
                fprintf(outobj, "f %d/%d %d/%d %d/%d\n", a, a, b, b, c, c);
            } else {
                fprintf(outobj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", a, a, a, b, b, b, c, c, c);
            }
        }
        vertexCountBase += atlasMesh->vertexCount;
    }
}

} // namespace meshtools::models::obj