#pragma once

#include <meshtools/math.hpp>
#include <meshtools/result.hpp>

#include <memory>
#include <vector>

namespace meshtools::models {

namespace detail {
template<class T>
inline T copy(const unsigned char* data, size_t componentByteSize) {
    assert(componentByteSize <= sizeof(T));
    T result{};
    memcpy(&result, data, componentByteSize);
    return result;
}


template<class T>
inline T convert(const unsigned char* data, size_t componentByteSize);

template<>
inline glm::vec2 convert<glm::vec2>(const unsigned char* data, size_t componentByteSize) {
    glm::vec2 result{};
    result[0] = copy<float>(data, componentByteSize);
    result[1] = copy<float>(data + componentByteSize, componentByteSize);
    return result;
}

template<>
inline glm::vec3 convert<glm::vec3>(const unsigned char* data, size_t componentByteSize) {
    glm::vec3 result{};
    result[0] = copy<float>(data, componentByteSize);
    result[1] = copy<float>(data + componentByteSize, componentByteSize);
    result[2] = copy<float>(data + componentByteSize * 2, componentByteSize);
    return result;
}

template<>
inline uint32_t convert<uint32_t>(const unsigned char* data, size_t componentByteSize) {
    assert(componentByteSize <= sizeof(uint32_t));
    return copy<uint32_t>(data, componentByteSize);
}

} // namespace detail


enum class DataType {
    FLOAT,
    DOUBLE,
    BYTE,
    U_BYTE,
    SHORT,
    U_SHORT,
    INT,
    U_INT,
    UNKNOWN,
};

inline size_t bytes(DataType dataType) {
    switch (dataType) {
        case DataType::BYTE:
        case DataType::U_BYTE:
            return sizeof(char);
        case DataType::SHORT:
        case DataType::U_SHORT:
            return sizeof(short);
        case DataType::INT:
        case DataType::U_INT:
            return sizeof(int);
        case DataType::FLOAT:
            return sizeof(float);
        case DataType::DOUBLE:
            return sizeof(double);
        case DataType::UNKNOWN:
            assert(false);
            throw std::runtime_error("Unknown data type");
    }
}

struct TypedData {
    TypedData(DataType dataType, size_t componentCount, std::vector<unsigned char> data)
        : dataType_(dataType), componentCount_(componentCount), data_(std::move(data)) {}

    TypedData(DataType dataType, size_t componentCount, size_t count) : dataType_(dataType), componentCount_(componentCount) {
        data_.resize(componentCount_ * bytes(dataType_) * count);
    }

    TypedData() = default;
    TypedData(const TypedData&) = delete;
    TypedData(TypedData&&) = default;
    TypedData& operator=(const TypedData&) = delete;
    TypedData& operator=(TypedData&&) = default;

    size_t componentCount() const {
        return componentCount_;
    }

    size_t componentSize() const {
        return bytes(dataType_);
    }

    size_t stride() const {
        return componentCount() * componentSize();
    }

    size_t count() const {
        return data_.size() / stride();
    }

    const std::vector<unsigned char>& buffer() const {
        return data_;
    }

    DataType dataType() const {
        return dataType_;
    }

    unsigned char& operator[](size_t pos) {
        return *(data() + pos * stride());
    }

    const unsigned char& operator[](size_t pos) const {
        return *(data() + pos * stride());
    }

    unsigned char* data() {
        return data_.data();
    }

    const unsigned char* data() const {
        return data_.data();
    }

    // TODO view()

private:
    DataType dataType_;
    size_t componentCount_;
    std::vector<unsigned char> data_;
};


template<class T>
struct DataView {

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(const DataView<T>& dataView, difference_type index) : dataView_(&dataView), index_(index) {}

        reference operator*() const {
            return current_ = dataView_->operator[](index_);
        }

        pointer operator->() {
            return &current_ = dataView_->operator[](index_);
        }

        // Prefix increment
        Iterator& operator++() {
            index_++;
            return *this;
        }

        // Postfix increment
        Iterator operator++(int) {
            Iterator tmp = *this;
            index_++;
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.index_ == b.index_;
        }
        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return a.index_ != b.index_;
        }

    private:
        // Needs to be a pointer as to keep this copyable
        const DataView<T>* dataView_;
        difference_type index_;
        mutable T current_;
    };

    using iterator = Iterator;
    using const_iterator = Iterator;
    using value_type = T;

    explicit DataView(const TypedData& data) : data_(data) {}

    Iterator begin() {
        return {*this, 0};
    }

    Iterator end() {
        return {*this, data_.count()};
    }

    Iterator begin() const {
        return {*this, 0};
    }

    Iterator end() const {
        return {*this, data_.count()};
    }

    size_t size() const {
        return data_.count();
    }

    size_t stride() const {
        return sizeof(T);
    }

    T operator[](size_t pos) const {
        // TODO: optimize for case where input and view are equal
        // TODO: cache and return reference instead of value?
        return detail::convert<T>(data_.buffer().data() + pos * data_.stride(), data_.componentSize());
    }

    std::vector<T> vector() {
        return {begin(), end()};
    }

    std::vector<T> vector() const {
        return {begin(), end()};
    }

    const unsigned char* raw() const {
        return data_.buffer().data();
    }

private:
    const TypedData& data_;
};


} // namespace meshtools::models