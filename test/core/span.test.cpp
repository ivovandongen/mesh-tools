#include <test.hpp>

#include <meshtools/span.hpp>

#include <vector>

using namespace meshtools;

TEST(Span, Basic) {
    std::vector<float> data{1, 2, 3, 4};

    span<float> view{&data[1], data.size() - 1};
    ASSERT_EQ(view.size(), data.size() - 1);
    ASSERT_EQ(*view.begin(), 2);
    ASSERT_EQ(*(view.end() - 1), 4);
    ASSERT_EQ(view[0], 2);
    ASSERT_EQ(view[1], 3);
    ASSERT_EQ(view[2], 4);
}

TEST(Span, Const) {
    const std::vector<float> data{1, 2, 3, 4};

    span<const float> view{&data[1], data.size() - 1};
    ASSERT_EQ(view.size(), data.size() - 1);
    ASSERT_EQ(*view.begin(), 2);
    ASSERT_EQ(*(view.end() - 1), 4);
    ASSERT_EQ(view[0], 2);
    ASSERT_EQ(view[1], 3);
    ASSERT_EQ(view[2], 4);
}

TEST(Span, Assignable) {
    std::vector<float> data{1, 2, 3, 4};

    span<float> view{data.data(), data.size()};
    view[1] = 101;
    ASSERT_EQ(view[1], 101);
    ASSERT_EQ(view[1], data[1]);
}
