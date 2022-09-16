#pragma once

#include <cstddef>

namespace meshtools {

template<typename T>
class span {
public:
    span(T* start, std::size_t len) : start_{start}, len_{len} {}

    T& operator[](int i) {
        return *start_[i];
    }

    T const& operator[](int i) const {
        return *start_[i];
    }

    std::size_t size() const {
        return len_;
    }

    T* begin() {
        return start_;
    }

    T* end() {
        return start_ + len_;
    }

    const T* begin() const {
        return start_;
    }

    const T* end() const {
        return start_ + len_;
    }

private:
    T* start_;
    std::size_t len_;
};

} // namespace meshtools