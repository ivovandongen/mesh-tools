#pragma once

#include <meshtools/math.hpp>

#include <string>

namespace meshtools::models {

struct PBRMetallicRoughness {
    glm::vec4 baseColorFactor{1, 1, 1, 1};
    // TODO: Ignoring multiple uv channels
    int baseColorTexture = -1;
    float metallicFactor = 1;
    float roughnessFactor = 1;

    // TODO: other items from: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material-pbrmetallicroughness
};

struct Material {
    std::string name;
    PBRMetallicRoughness pbrMetallicRoughness{};
    int occlusionTexture = -1;
    std::string alphaMode = "OPAQUE";
    float alphaCutoff = 0.5;
    bool doubleSided = false;

    // TODO: other items from: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#reference-material
};

} // namespace meshtools::models