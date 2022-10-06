#include <test.hpp>

#include <meshtools/models/mesh_group.hpp>

#include <filesystem>

using namespace meshtools::models;

TEST(MeshGroup, MergeSingle) {
    std::vector<std::shared_ptr<Mesh>> meshes;
    {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData::From(3, std::vector<float>{1, 2, 3, 4, 5, 6});
        meshes.push_back(std::make_shared<Mesh>("mesh", -1, TypedData::From(1, std::vector<uint16_t>{1, 2}), std::move(vertexData)));
    }
    MeshGroup meshGroup{"group", std::move(meshes)};
    meshGroup.merge(true);
    ASSERT_EQ(meshGroup.meshes().size(), 1);
    ASSERT_EQ(meshGroup.meshes()[0]->indices().size(), 2);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute(AttributeType::POSITION).size(), 2);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION).size(), 2);
}

TEST(MeshGroup, MergeTwo) {
    std::vector<std::shared_ptr<Mesh>> meshes;
    {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData::From(3, std::vector<float>{1, 2, 3, 4, 5, 6});
        meshes.push_back(std::make_shared<Mesh>("mesh-1", -1, TypedData::From(1, std::vector<uint16_t>{1, 2}), std::move(vertexData)));
    }
    {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData::From(3, std::vector<float>{7, 8, 9, 10, 11, 12});
        meshes.push_back(std::make_shared<Mesh>("mesh-2", -1, TypedData::From(1, std::vector<uint16_t>{1, 2}), std::move(vertexData)));
    }
    MeshGroup meshGroup{"group", std::move(meshes)};
    meshGroup.merge(true);
    ASSERT_EQ(meshGroup.meshes().size(), 1);
    ASSERT_EQ(meshGroup.meshes()[0]->indices().size(), 4);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute(AttributeType::POSITION).size(), 4);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION).size(), 4);

    for (size_t i = 0; i < meshGroup.meshes()[0]->indices().size(); i++) {
        ASSERT_EQ(i + 1, meshGroup.meshes()[0]->indices<uint16_t>()[i]);
    }

    for (size_t i = 0; i < meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION).size(); i++) {
        auto& pos = meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION)[i];
        ASSERT_EQ(i * 3 + 1, pos.x);
        ASSERT_EQ(i * 3 + 2, pos.y);
        ASSERT_EQ(i * 3 + 3, pos.z);
    }
}

TEST(MeshGroup, MergeMultipleVertexAttibutes) {
    std::vector<std::shared_ptr<Mesh>> meshes;
    {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData::From(3, std::vector<float>{1, 2, 3, 4, 5, 6});
        vertexData[AttributeType::TEXCOORD] = TypedData::From(2, std::vector<float>{1, 2, 3, 4});
        meshes.push_back(std::make_shared<Mesh>("mesh-1", -1, TypedData::From(1, std::vector<uint16_t>{1, 2}), std::move(vertexData)));
    }
    {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData::From(3, std::vector<float>{7, 8, 9, 10, 11, 12});
        vertexData[AttributeType::TEXCOORD] = TypedData::From(2, std::vector<float>{5, 6, 7, 8});
        meshes.push_back(std::make_shared<Mesh>("mesh-2", -1, TypedData::From(1, std::vector<uint16_t>{1, 2}), std::move(vertexData)));
    }
    {
        VertexData vertexData;
        vertexData[AttributeType::POSITION] = TypedData::From(3, std::vector<float>{13, 14, 15, 16, 17, 18});
        vertexData[AttributeType::TEXCOORD] = TypedData::From(2, std::vector<float>{9, 10, 11, 12});
        meshes.push_back(std::make_shared<Mesh>("mesh-3", -1, TypedData::From(1, std::vector<uint16_t>{1, 2}), std::move(vertexData)));
    }
    MeshGroup meshGroup{"group", std::move(meshes)};
    meshGroup.merge(true);
    ASSERT_EQ(meshGroup.meshes().size(), 1);
    ASSERT_EQ(meshGroup.meshes()[0]->indices().size(), 6);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute(AttributeType::POSITION).size(), 6);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute(AttributeType::TEXCOORD).size(), 6);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION).size(), 6);
    ASSERT_EQ(meshGroup.meshes()[0]->vertexAttribute<glm::vec2>(AttributeType::TEXCOORD).size(), 6);

    for (size_t i = 0; i < meshGroup.meshes()[0]->indices().size(); i++) {
        ASSERT_EQ(i + 1, meshGroup.meshes()[0]->indices<uint16_t>()[i]);
    }

    for (size_t i = 0; i < meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION).size(); i++) {
        auto& pos = meshGroup.meshes()[0]->vertexAttribute<glm::vec3>(AttributeType::POSITION)[i];
        auto& uv = meshGroup.meshes()[0]->vertexAttribute<glm::vec2>(AttributeType::TEXCOORD)[i];
        ASSERT_EQ(i * 3 + 1, pos.x);
        ASSERT_EQ(i * 3 + 2, pos.y);
        ASSERT_EQ(i * 3 + 3, pos.z);
        ASSERT_EQ(i * 2 + 1, uv.s);
        ASSERT_EQ(i * 2 + 2, uv.t);
    }
}
