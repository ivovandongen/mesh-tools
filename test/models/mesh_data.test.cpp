#include <test.hpp>

#include <meshtools/models/mesh_data.hpp>

#include <filesystem>

using namespace meshtools::models;

TEST(MeshData, TypedData) {
    using T = glm::vec2;
    std::vector<T> data{{0, 1}, {2, 3}, {4, 5}};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(T));
    memcpy(raw.data(), data.data(), raw.size());

    TypedData typedData{DataType::FLOAT, 2, raw};

    ASSERT_EQ(typedData.stride(), sizeof(T));
    ASSERT_EQ(typedData.componentSize(), sizeof(float));
    ASSERT_EQ(typedData.componentCount(), 2);
    ASSERT_EQ(typedData.count(), 3);
}

TEST(MeshData, DataViewIterableIntegral) {
    std::vector<uint32_t> data{0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(uint32_t));
    memcpy(raw.data(), data.data(), raw.size());

    TypedData typedData{DataType::U_INT, 1, raw};
    DataView<uint32_t> dataView{typedData};

    // For loop
    auto idx = 0;
    for (auto& d : dataView) {
        ASSERT_EQ(d, data[idx]);
        idx++;
    }

    // Prefix iterator
    idx = 0;
    for (auto it = dataView.begin(), end = dataView.end(); it != end; ++it) { // NOLINT(modernize-loop-convert)
        const auto d = *it;
        ASSERT_EQ(d, data[idx]);
        idx++;
    }

    // Postfix iterator
    idx = 0;
    for (auto it = dataView.begin(), end = dataView.end(); it != end; it++) { // NOLINT(modernize-loop-convert)
        const auto d = *it;
        ASSERT_EQ(d, data[idx]);
        idx++;
    }
}

TEST(MeshData, DataViewIterableComplexType) {
    std::vector<glm::vec3> data{{0, 1, 2}, {3, 4, 5}};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(glm::vec3));
    memcpy(raw.data(), data.data(), raw.size());

    TypedData typedData{DataType::FLOAT, 3, raw};
    DataView<glm::vec3> dataView{typedData};

    // For loop
    auto idx = 0;
    for (auto& d : dataView) {
        ASSERT_FLOAT_EQ(d[0], idx * 3);
        ASSERT_FLOAT_EQ(d[1], idx * 3 + 1);
        ASSERT_FLOAT_EQ(d[2], idx * 3 + 2);
        idx++;
    }

    // Prefix iterator
    idx = 0;
    for (auto it = dataView.begin(), end = dataView.end(); it != end; ++it) { // NOLINT(modernize-loop-convert)
        const auto d = *it;
        ASSERT_FLOAT_EQ(d[0], idx * 3);
        ASSERT_FLOAT_EQ(d[1], idx * 3 + 1);
        ASSERT_FLOAT_EQ(d[2], idx * 3 + 2);
        idx++;
    }

    // Postfix iterator
    idx = 0;
    for (auto it = dataView.begin(), end = dataView.end(); it != end; it++) { // NOLINT(modernize-loop-convert)
        const auto d = *it;
        ASSERT_FLOAT_EQ(d[0], idx * 3);
        ASSERT_FLOAT_EQ(d[1], idx * 3 + 1);
        ASSERT_FLOAT_EQ(d[2], idx * 3 + 2);
        idx++;
    }
}

TEST(MeshData, DataViewAlgorithms) {
    std::vector<uint32_t> data{0, 5, 3, 9, 2, 7};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(uint32_t));
    memcpy(raw.data(), data.data(), raw.size());

    TypedData typedData{DataType::U_INT, 1, raw};
    DataView<uint32_t> dataView{typedData};

    ASSERT_EQ(*std::min_element(dataView.begin(), dataView.end()), *std::min_element(data.begin(), data.end()));
    ASSERT_EQ(*std::max_element(dataView.begin(), dataView.end()), *std::max_element(data.begin(), data.end()));

    auto minMax = std::minmax_element(dataView.begin(), dataView.end());
    ASSERT_EQ(*minMax.first, *std::min_element(data.begin(), data.end()));
    ASSERT_EQ(*minMax.second, *std::max_element(data.begin(), data.end()));
}

TEST(MeshData, DataViewIndexable) {
    std::vector<glm::vec3> data{{0, 1, 2}, {3, 4, 5}};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(glm::vec3));
    memcpy(raw.data(), data.data(), raw.size());

    TypedData typedData{DataType::FLOAT, 3, raw};

    DataView<glm::vec3> dataView{typedData};
    ASSERT_FLOAT_EQ(dataView[0][0], 0);
    ASSERT_FLOAT_EQ(dataView[0][1], 1);
    ASSERT_FLOAT_EQ(dataView[0][2], 2);
    ASSERT_FLOAT_EQ(dataView[1][0], 3);
    ASSERT_FLOAT_EQ(dataView[1][1], 4);
    ASSERT_FLOAT_EQ(dataView[1][2], 5);
}