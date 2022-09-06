#pragma once

#include <memory>
#include <string>

namespace meshtools {

template<class T>
struct Result {
    Result() = default;
    Result(std::shared_ptr<T> val) : value(std::move(val)) {}
    Result(std::string err) : error(std::move(err)) {}

    std::shared_ptr<T> value;
    std::string error;

    operator bool() const {
        return error.empty() && value.operator bool();
    }
};

} // namespace meshtools