#include <test.hpp>

#include <meshtools/algorithm.hpp>

using namespace meshtools;

TEST(Model, Find) {
    struct Value {
        int a;
    };

    std::vector<Value> values{{1}, {2}, {2}, {3}};

    auto result = find(values, [](auto& val) { return val.a == 2; });

    ASSERT_EQ(result.size(), 2);
    for (auto& val : result) {
        ASSERT_EQ(val.get().a, 2);
    }
}

TEST(Model, FindConst) {
    struct Value {
        int a;
    };

    const std::vector<Value> values{{1}, {2}, {2}, {3}};

    auto result = find(values, [](const auto& val) { return val.a == 2; });

    ASSERT_EQ(result.size(), 2);
    for (auto& val : result) {
        ASSERT_EQ(val.get().a, 2);
    }
}
