#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cmath>

namespace meshtools {

template<class V, class S = float>
V scale(const V& vec, S scalar) {
    return vec * scalar;
}

static glm::vec3 barycentric(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& p) {
    auto ab = b - a;
    auto ac = c - a;
    auto ap = p - a;

    auto dotABAB = glm::dot(ab, ab);
    auto dotABAC = glm::dot(ab, ac);
    auto dotACAC = glm::dot(ac, ac);
    auto dotAPAB = glm::dot(ap, ab);
    auto dotAPAC = glm::dot(ap, ac);
    auto denom = dotABAB * dotACAC - dotABAC * dotABAC;

    float v = (dotACAC * dotAPAB - dotABAC * dotAPAC) / denom;
    float w = (dotABAB * dotAPAC - dotABAC * dotAPAB) / denom;
    float u = 1 - v - w;

    return {u, v, w};
}

} // namespace meshtools