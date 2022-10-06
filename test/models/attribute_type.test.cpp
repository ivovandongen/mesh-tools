#include <test.hpp>

#include <meshtools/models/attribute_type.hpp>

#include <unordered_map>

using namespace meshtools::models;

TEST(AttributeType, Common) {
    std::unordered_map<AttributeType, size_t> values;
    values[AttributeType::POSITION] = 0;
    values[AttributeType::NORMAL] = 1;
    values[AttributeType::COLOR] = 2;
    values[AttributeType::TEXCOORD] = 3;

    ASSERT_TRUE(values.find(AttributeType::POSITION) != values.end());
    ASSERT_EQ(values[AttributeType::POSITION], 0);
    ASSERT_EQ(values[AttributeType::NORMAL], 1);
    ASSERT_EQ(values[AttributeType::COLOR], 2);
    ASSERT_EQ(values[AttributeType::TEXCOORD], 3);
}

TEST(AttributeType, Custom) {
    std::unordered_map<AttributeType, size_t> values;
    values[AttributeType{"_CUSTOM_0"}] = 1;

    ASSERT_TRUE(values.find(AttributeType{"_CUSTOM_0"}) != values.end());
    ASSERT_EQ(values[AttributeType{"_CUSTOM_0"}], 1);
}
