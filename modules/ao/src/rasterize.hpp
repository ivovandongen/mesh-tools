#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>


template<uint8_t Channels = 4>
static void SetPixel(uint8_t* dest, int destWidth, int x, int y, const std::array<uint8_t, Channels>& color) {
    uint8_t* pixel = &dest[x * Channels + y * (destWidth * Channels)];
    for (size_t i = 0; i < Channels; i++) {
        pixel[i] = color[i];
    }
}


static void SetPixel(uint8_t* dest, int destWidth, int x, int y, const std::vector<uint8_t>& color) {
    const uint8_t channels = color.size();
    uint8_t* pixel = &dest[x * channels + y * (destWidth * channels)];
    for (size_t i = 0; i < channels; i++) {
        pixel[i] = color[i];
    }
}

// https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
template<int buffer = 2, class Fn>
static void RasterizeTriangle(std::array<int, 2> t0, std::array<int, 2> t1, std::array<int, 2> t2, Fn&& fn) {
    // Sort left to right
    if (t0[0] > t1[0])
        std::swap(t0, t1);
    if (t0[0] > t2[0])
        std::swap(t0, t2);
    if (t1[0] > t2[0])
        std::swap(t1, t2);


    // Expand width
    auto minX = t0[0];
    auto maxX = t2[0];
    t0[0] -= buffer;
    if (t1[0] == minX) {
        t1[0] = std::max(0, t1[0] - buffer);
    } else if (t1[0] == maxX) {
        t1[0] += buffer;
    }
    t2[0] += buffer;

    // Sort bottom to top
    if (t0[1] > t1[1])
        std::swap(t0, t1);
    if (t0[1] > t2[1])
        std::swap(t0, t2);
    if (t1[1] > t2[1])
        std::swap(t1, t2);

    // Expand height
    auto minY = t0[1];
    auto maxY = t2[1];
    t0[1] = std::max(0, t0[1] - buffer);
    if (t1[1] == minY) {
        t1[1] = std::max(0, t1[1] - buffer);
    } else if (t1[1] == maxY) {
        t1[1] += buffer;
    }
    t2[1] += buffer;

    int total_height = t2[1] - t0[1];
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1[1] - t0[1] || t1[1] == t0[1];
        int segment_height = second_half ? t2[1] - t1[1] : t1[1] - t0[1];
        float alpha = (float) i / total_height;
        float beta = (float) (i - (second_half ? t1[1] - t0[1] : 0)) / segment_height;
        std::array<int, 2> A, B;
        for (int j = 0; j < 2; j++) {
            A[j] = int(t0[j] + (t2[j] - t0[j]) * alpha);
            B[j] = int(second_half ? t1[j] + (t2[j] - t1[j]) * beta : t0[j] + (t1[j] - t0[j]) * beta);
        }
        if (A[0] > B[0])
            std::swap(A, B);
        for (int j = A[0]; j <= B[0]; j++) {
            fn(j, t0[1] + i);
        }
    }
}
