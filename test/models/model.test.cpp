#include <test.hpp>

#include <meshtools/models/model.hpp>

#include <filesystem>

using namespace meshtools::models;

TEST(Model, LoadBasicModel) {
    auto model = Model::Load(getFixturesPath("models/basic.gltf"));
    ASSERT_TRUE(model.value);
}

TEST(Model, Merge) {
    auto modelResult1 = Model::Load(getFixturesPath("models/basic.gltf"));
    auto modelResult2 = Model::Load(getFixturesPath("models/basic.gltf"));

    auto& model1 = *modelResult1.value;
    auto& model2 = *modelResult2.value;
    model1.merge(model2);

    ASSERT_EQ(model1.nodes(0).size(), 2);
    ASSERT_EQ(*model1.nodes(0)[0].mesh(), 0);
    ASSERT_EQ(*model1.nodes(0)[1].mesh(), 1);
    ASSERT_EQ(model1.meshGroups().size(), 2);
    ASSERT_EQ(model1.meshes(0, false).size(), 2);
}
