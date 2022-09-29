#include <test.hpp>

#include <meshtools/algorithm.hpp>

using namespace meshtools;

TEST(Algorithm, Find) {
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

TEST(Algorithm, FindConst) {
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

TEST(Algorithm, Flatten) {
    std::vector<std::vector<int>> in{{1, 2, 3}, {4}, {5, 6}, {7}};

    auto result = flatten(in);

    ASSERT_EQ(result.size(), 7);
    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_EQ(i + 1, result[i]);
    }
}
