#include <test.hpp>

#include <meshtools/models/model.hpp>

#include <filesystem>

using namespace meshtools::models;

TEST(Model, LoadBasicModel) {
    auto model = Model::Load(getFixturesPath("models/basic.gltf"));
    ASSERT_TRUE(model.value);
}
