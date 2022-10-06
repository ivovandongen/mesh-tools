#pragma once

#include <functional>
#include <string>

namespace meshtools::models {

struct AttributeType {
    const std::string name;

    // Common attribute types
    const static AttributeType POSITION;
    const static AttributeType NORMAL;
    const static AttributeType TEXCOORD;
    const static AttributeType COLOR;
};

static bool operator==(const AttributeType& lhs, const AttributeType& rhs) {
    return lhs.name == rhs.name;
}

} // namespace meshtools::models

template<>
struct std::hash<meshtools::models::AttributeType> {
    std::size_t operator()(meshtools::models::AttributeType const& s) const noexcept {
        return std::hash<std::string>{}(s.name);
    }
};