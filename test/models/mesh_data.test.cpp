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
    ASSERT_EQ(typedData.size(), 3);

    {
        auto index = 0;
        for (auto el : typedData) {
            ASSERT_EQ(el.size(), sizeof(T));
            T value = *((T*) el.begin());
            ASSERT_EQ(value[0], data[index][0]);
            ASSERT_EQ(value[1], data[index][1]);
            index++;
        }
    }

    for (size_t index = 0; index < data.size(); index++) {
        auto bytes = typedData[index];
        ASSERT_EQ(bytes.size(), sizeof(T));
        T value = *((T*) bytes.begin());
        ASSERT_EQ(value[0], data[index][0]);
        ASSERT_EQ(value[1], data[index][1]);
    }
}

TEST(MeshData, DataView) {
    using T = glm::vec2;
    std::vector<T> data{{0, 1}, {2, 3}, {4, 5}};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(T));
    memcpy(raw.data(), data.data(), raw.size());

    TypedData typedData{DataType::FLOAT, 2, raw};
    DataView<T> dataView{typedData};

    ASSERT_EQ(dataView.size(), data.size());
    ASSERT_EQ(dataView.stride(), sizeof(T));
    ASSERT_EQ(std::distance(dataView.begin(), dataView.end()), data.size());
    for (size_t index = 0; index < data.size(); index++) {
        ASSERT_EQ(data[index][0], dataView[index][0]);
        ASSERT_EQ(data[index][1], dataView[index][1]);
    }
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

TEST(MeshData, DataViewIterableIntegralConversion) {
    std::vector<uint16_t> data{0, 1, 2, 3, 4, 5, 6, 7};
    TypedData typedData = TypedData::From(1, data);
    DataView<uint32_t> dataView{typedData};

    // For loop
    auto idx = 0;
    for (auto& d : dataView) {
        ASSERT_EQ(d, data[idx]);
        idx++;
    }
}

TEST(MeshData, DataViewIterableComplexTypeConversion) {
    std::vector<uint16_t> data{0, 1, 2, 3, 4, 5};
    TypedData typedData = TypedData::From(3, data);
    DataView<glm::u32vec3> dataView{typedData};

    // For loop
    auto idx = 0;
    for (auto& d : dataView) {
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

TEST(MeshData, Copy) {
    using T = uint16_t;
    std::vector<T> data{0, 1, 2, 3, std::numeric_limits<T>::max()};
    std::vector<unsigned char> raw;
    raw.resize(data.size() * sizeof(T));
    memcpy(raw.data(), data.data(), raw.size());

    // Copy 1:1
    {
        TypedData typedData{DataType::U_SHORT, 1, raw};
        DataView<T> dataView{typedData};
        std::vector<unsigned char> result(dataView.size() * dataView.stride());
        dataView.copyTo(result.data());

        for (size_t index = 0; index < result.size(); index++) {
            ASSERT_EQ(raw[index], result[index]);
        }
    }

    // Copy with a view
    {
        TypedData typedData{DataType::U_SHORT, 1, raw};
        DataView<uint32_t> dataView{typedData};
        std::vector<unsigned char> result(dataView.size() * dataView.stride());
        dataView.copyTo(result.data());

        for (size_t index = 0; index < data.size(); index++) {
            ASSERT_EQ(data[index], *((uint32_t*) (result.data() + index * sizeof(uint32_t))));
        }
    }
}

TEST(MeshData, FromBuiltIn) {
    std::vector<uint32_t> data{0, 1, 2, 3, 4, 5};
    auto typedData = TypedData::From(1, data);
    ASSERT_EQ(data.size(), typedData.size());
    for (size_t i = 0; i < typedData.size(); i++) {
        ASSERT_EQ(*((uint32_t*) typedData[i].begin()), data[i]);
    }
    typedData = TypedData::From(3, data);
    ASSERT_EQ(typedData.size(), data.size() / 3);
}

TEST(MeshData, FromUserDefined) {
    std::vector<glm::vec3> data{{0, 1, 2}, {3, 4, 5}};
    auto typedData = TypedData::From(DataType::FLOAT, 3, data);
    ASSERT_EQ(data.size(), typedData.size());
    for (size_t i = 0; i < typedData.size(); i++) {
        ASSERT_EQ(*((glm::vec3*) typedData[i].begin()), data[i]);
    }
}
