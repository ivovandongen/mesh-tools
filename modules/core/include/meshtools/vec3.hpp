#pragma once

#include <array>
#include <cmath>

using vec3 = std::array<float, 3>;

inline vec3 vec3Sub(const vec3& a, const vec3& b) {
    return vec3{{a[0] - b[0], a[1] - b[1], a[2] - b[2]}};
}

inline float vec3Dot(const vec3& a, const vec3& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline double vec3LengthSq(const vec3& a) {
    return vec3Dot(a, a);
}

inline double vec3Length(const vec3& a) {
    auto lsq = vec3LengthSq(a);
    return lsq != 0 ? std::sqrt(lsq) : 0;
}

inline vec3 vec3Scale(const vec3& a, float s) {
    return vec3{{a[0] * s, a[1] * s, a[2] * s}};
}


inline vec3 vec3Normalize(const vec3& a) {
    auto l = vec3Length(a);
    return l != 0 ? vec3Scale(a, 1.0 / l) : a;
}

static vec3 barycentric(const vec3& a, const vec3& b, const vec3& c, const vec3& p) {
    auto ab = vec3Sub(b, a);
    auto ac = vec3Sub(c, a);
    auto ap = vec3Sub(p, a);

    auto dotABAB = vec3Dot(ab, ab);
    auto dotABAC = vec3Dot(ab, ac);
    auto dotACAC = vec3Dot(ac, ac);
    auto dotAPAB = vec3Dot(ap, ab);
    auto dotAPAC = vec3Dot(ap, ac);
    auto denom = dotABAB * dotACAC - dotABAC * dotABAC;

    float v = (dotACAC * dotAPAB - dotABAC * dotAPAC) / denom;
    float w = (dotABAB * dotAPAC - dotABAC * dotAPAB) / denom;
    float u = 1 - v - w;

    return {u, v, w};
}