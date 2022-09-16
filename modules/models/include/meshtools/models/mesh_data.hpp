#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/math.hpp>
#include <meshtools/result.hpp>
#include <meshtools/span.hpp>

#include <memory>
#include <vector>

namespace meshtools::models {

namespace detail {
template<class T>
inline T copy(const uint8_t* data, size_t componentByteSize) {
    assert(componentByteSize <= sizeof(T));
    T result{};
    memcpy(&result, data, componentByteSize);
    return result;
}


template<class T>
inline T convert(const uint8_t* data, size_t componentByteSize);

template<>
inline glm::vec2 convert<glm::vec2>(const uint8_t* data, size_t componentByteSize) {
    glm::vec2 result{};
    result[0] = copy<float>(data, componentByteSize);
    result[1] = copy<float>(data + componentByteSize, componentByteSize);
    return result;
}

template<>
inline glm::vec3 convert<glm::vec3>(const uint8_t* data, size_t componentByteSize) {
    glm::vec3 result{};
    result[0] = copy<float>(data, componentByteSize);
    result[1] = copy<float>(data + componentByteSize, componentByteSize);
    result[2] = copy<float>(data + componentByteSize * 2, componentByteSize);
    return result;
}

template<>
inline uint32_t convert<uint32_t>(const uint8_t* data, size_t componentByteSize) {
    assert(componentByteSize <= sizeof(uint32_t));
    return copy<uint32_t>(data, componentByteSize);
}

template<>
inline uint16_t convert<uint16_t>(const uint8_t* data, size_t componentByteSize) {
    assert(componentByteSize <= sizeof(uint16_t));
    return copy<uint16_t>(data, componentByteSize);
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
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = span<const uint8_t>;
        using pointer = value_type;
        using reference = value_type;

        Iterator(const TypedData& typedData, difference_type index) : typedData_(&typedData), index_(index) {}

        const reference operator*() const {
            return typedData_->operator[](index_);
        }

        const pointer operator->() {
            return typedData_->operator[](index_);
        }

        Iterator& operator++() {
            index_++;
            return *this;
        }

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
        // Needs to be a pointer to keep the iterator copyable
        const TypedData* typedData_;
        difference_type index_;
    };

    using iterator = Iterator;
    using const_iterator = Iterator;
    using value_type = span<uint8_t>;

    TypedData(DataType dataType, size_t componentCount, std::vector<uint8_t> data)
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

    size_t size() const {
        return data_.size() / stride();
    }

    const std::vector<uint8_t>& buffer() const {
        return data_;
    }

    DataType dataType() const {
        return dataType_;
    }

    span<uint8_t> operator[](size_t pos) {
        return {data_.data() + pos * stride(), stride()};
    }

    const span<const uint8_t> operator[](size_t pos) const {
        return {data_.data() + pos * stride(), stride()};
    }

    Iterator begin() {
        return {*this, 0};
    }

    Iterator end() {
        return {*this, size()};
    }

    Iterator begin() const {
        return {*this, 0};
    }

    Iterator end() const {
        return {*this, size()};
    }

    void copyTo(void* out) const {
        std::memcpy(out, data_.data(), data_.size());
    }

    void copyFrom(void* in) {
        std::memcpy(data_.data(), in, data_.size());
    }

    // TODO view()

private:
    DataType dataType_;
    size_t componentCount_;
    std::vector<uint8_t> data_;
};


template<class T>
struct DataView {
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(const T* start, difference_type index) : start_(start), index_(index) {}

        reference operator*() const {
            return *(start_ + index_);
        }

        pointer operator->() {
            return start_ + index_;
        }

        Iterator& operator++() {
            index_++;
            return *this;
        }

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
        const T* start_;
        difference_type index_;
    };

    using iterator = Iterator;
    using const_iterator = Iterator;

    explicit DataView(const TypedData& data) : data_(data) {
        // optimize for actual format
        // TODO: check alignment
        if (data.stride() != sizeof(T)) {
            view_ = transform<T>(data, [](auto raw) { return detail::convert<T>(raw.begin(), raw.size()); });
        }
    }

    Iterator begin() const {
        return view_.empty() ? Iterator{(T*) data_.buffer().data(), 0} : Iterator{view_.data(), 0};
    }

    Iterator end() const {
        return view_.empty() ? Iterator{(T*) data_.buffer().data(), data_.size()} : Iterator{view_.data(), view_.size()};
    }

    size_t size() const {
        return data_.size();
    }

    size_t stride() const {
        return sizeof(T);
    }

    const T& operator[](size_t pos) const {
        return view_.empty() ? (T&) data_.buffer()[pos * data_.stride()] : view_[pos];
    }

    void copyTo(void* out) const {
        if (view_.empty()) {
            data_.copyTo(out);
        } else {
            std::memcpy(out, view_.data(), view_.size() * sizeof(T));
        }
    }

    // TODO: weird naming
    const TypedData& data() const {
        return data_;
    }

private:
    const TypedData& data_;
    std::vector<T> view_;
};

} // namespace meshtools::models