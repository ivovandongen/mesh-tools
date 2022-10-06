#include "model.hpp"

#include <meshtools/file.hpp>
#include <meshtools/logging.hpp>
#include <meshtools/models/model.hpp>


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

        auto mesh = std::make_shared<Mesh>(shape.name,
                                           -1, // TODO: Material
                                           TypedData{DataType::U_INT, 1, copy<unsigned char>(shape.mesh.indices)},
                                           std::move(vertexData));
        meshes.emplace_back(shape.name, std::move(mesh), Extra{});
    }

    result.value = std::make_shared<Model>(std::move(meshes));
    return result;
}

} // namespace meshtools::models::obj
