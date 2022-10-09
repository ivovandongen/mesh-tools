#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/logging.hpp>
#include <meshtools/math.hpp>
#include <meshtools/result.hpp>
#include <meshtools/span.hpp>

#include <memory>
#include <vector>

namespace meshtools::models {

namespace detail {

template<typename T, bool = std::is_arithmetic<T>::value>
struct Converter {};

template<typename T>
struct Converter<T, true> {

    T operator()(const uint8_t* data, size_t /*components*/, size_t componentByteSize) {
        assert(componentByteSize <= sizeof(T));
        T result{};
        memcpy(&result, data, componentByteSize);
        return result;
    }
};

template<typename T>
struct Converter<T, false> {

    T operator()(const uint8_t* data, size_t components, size_t componentByteSize) {
        assert(componentByteSize <= sizeof(T));
        T result{};
        for (size_t i = 0; i < components; i++) {
            memcpy(&result[i], data + i * componentByteSize, componentByteSize);
        }
        return result;
    }
};

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

template<class T>
constexpr inline DataType toDataType() {
    if constexpr (std::is_same_v<char, T>) {
        return DataType::BYTE;
    } else if constexpr (std::is_same_v<unsigned char, T>) {
        return DataType::U_BYTE;
    } else if constexpr (std::is_same_v<int16_t, T>) {
        return DataType::SHORT;
    } else if constexpr (std::is_same_v<uint16_t, T>) {
        return DataType::U_SHORT;
    } else if constexpr (std::is_same_v<int32_t, T>) {
        return DataType::INT;
    } else if constexpr (std::is_same_v<uint32_t, T>) {
        return DataType::U_INT;
    } else if constexpr (std::is_same_v<float, T>) {
        return DataType::FLOAT;
    } else if constexpr (std::is_same_v<double, T>) {
        return DataType::DOUBLE;
    } else {
        // Unmapped type
        static_assert(!sizeof(T), "Unknown data type");
    }
}

struct TypedData {
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = span<uint8_t>;
        using pointer = value_type;
        using reference = value_type;

        Iterator(TypedData& typedData, difference_type index) : typedData_(&typedData), index_(index) {}

        reference operator*() const {
            return typedData_->operator[](index_);
        }

        pointer operator->() {
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
        TypedData* typedData_;
        difference_type index_;
    };

    struct ConstIterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::size_t;
        using value_type = span<const uint8_t>;
        using pointer = value_type;
        using reference = value_type;

        ConstIterator(const TypedData& typedData, difference_type index) : typedData_(&typedData), index_(index) {}

        reference operator*() const {
            return typedData_->operator[](index_);
        }

        pointer operator->() {
            return typedData_->operator[](index_);
        }

        ConstIterator& operator++() {
            index_++;
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            index_++;
            return tmp;
        }

        friend bool operator==(const ConstIterator& a, const ConstIterator& b) {
            return a.index_ == b.index_;
        }
        friend bool operator!=(const ConstIterator& a, const ConstIterator& b) {
            return a.index_ != b.index_;
        }

    private:
        // Needs to be a pointer to keep the iterator copyable
        const TypedData* typedData_;
        difference_type index_;
    };

    using iterator = Iterator;
    using const_iterator = ConstIterator;
    using value_type = span<uint8_t>;

    template<class T>
    static TypedData From(DataType dataType, size_t componentCount, const std::vector<T>& data) {
        TypedData result(dataType, componentCount, data.size() * sizeof(T) / bytes(dataType) / componentCount);
        result.copyFrom(data.data());
        return result;
    }

    template<class T>
    static TypedData From(size_t componentCount, const std::vector<T>& data) {
        return From(toDataType<T>(), componentCount, data);
    }

    TypedData(DataType dataType, size_t componentCount, std::vector<uint8_t> data)
        : dataType_(dataType), componentCount_(componentCount), data_(std::move(data)) {}

    TypedData(DataType dataType, size_t componentCount, size_t count) : dataType_(dataType), componentCount_(componentCount) {
        data_.resize(componentCount_ * bytes(dataType_) * count);
    }

    TypedData() = default;

    // Delete copy
    TypedData(const TypedData&) = delete;
    TypedData& operator=(const TypedData&) = delete;

    // Keep move
    TypedData(TypedData&&) = default;
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

    iterator begin() {
        return {*this, 0};
    }

    iterator end() {
        return {*this, size()};
    }

    const_iterator begin() const {
        return {*this, 0};
    }

    const_iterator end() const {
        return {*this, size()};
    }

    void append(const TypedData& other) {
        if (dataType_ != other.dataType_ || componentCount_ != other.componentCount_) {
            // TODO: error handling
            logging::error("Data type or component count does not match");
            assert(false);
        }

        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
    }

    void copyTo(void* out) const {
        std::memcpy(out, data_.data(), data_.size());
    }

    void copyFrom(const void* in) {
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
            detail::Converter<T> converter;
            view_ = transform<T>(data, [&](auto raw) { return converter(raw.begin(), data.componentCount(), data.componentSize()); });
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