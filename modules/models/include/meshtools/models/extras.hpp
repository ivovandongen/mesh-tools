#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace meshtools::models {

// From https://github.com/mapbox/variant/blob/master/include/mapbox/recursive_wrapper.hpp
template<typename T>
class recursive_wrapper {

    T* p_;

    void assign(T const& rhs) {
        this->get() = rhs;
    }

public:
    using type = T;

    recursive_wrapper() : p_(new T) {}

    ~recursive_wrapper() noexcept {
        delete p_;
    }

    recursive_wrapper(recursive_wrapper const& operand) : p_(new T(operand.get())) {}

    recursive_wrapper(T const& operand) : p_(new T(operand)) {}

    recursive_wrapper(recursive_wrapper&& operand) : p_(new T(std::move(operand.get()))) {}

    recursive_wrapper(T&& operand) : p_(new T(std::move(operand))) {}

    inline recursive_wrapper& operator=(recursive_wrapper const& rhs) {
        assign(rhs.get());
        return *this;
    }

    inline recursive_wrapper& operator=(T const& rhs) {
        assign(rhs);
        return *this;
    }

    inline void swap(recursive_wrapper& operand) noexcept {
        T* temp = operand.p_;
        operand.p_ = p_;
        p_ = temp;
    }

    recursive_wrapper& operator=(recursive_wrapper&& rhs) noexcept {
        swap(rhs);
        return *this;
    }

    recursive_wrapper& operator=(T&& rhs) {
        get() = std::move(rhs);
        return *this;
    }

    T& get() {
        assert(p_);
        return *get_pointer();
    }

    T const& get() const {
        assert(p_);
        return *get_pointer();
    }

    T* get_pointer() {
        return p_;
    }

    const T* get_pointer() const {
        return p_;
    }

    operator T const&() const {
        return this->get();
    }

    operator T&() {
        return this->get();
    }

}; // class recursive_wrapper

template<typename T>
inline void swap(recursive_wrapper<T>& lhs, recursive_wrapper<T>& rhs) noexcept {
    lhs.swap(rhs);
}

struct Extra;
using Extras = std::unordered_map<std::string, Extra>;
using ExtraArray = std::vector<Extra>;
using ExtraBase = std::variant<std::monostate, double, int32_t, bool, std::string, std::vector<unsigned char>,
                               recursive_wrapper<ExtraArray>, recursive_wrapper<Extras>>;

struct Extra : ExtraBase {
    using ExtraBase::ExtraBase;
};

} // namespace meshtools::models