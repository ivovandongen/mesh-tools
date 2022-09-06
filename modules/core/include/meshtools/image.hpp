#pragma once

#include <cassert>
#include <filesystem>
#include <vector>

namespace meshtools {

struct Image {
    Image(uint32_t width, uint32_t height, uint8_t channels) : width_(width), height_(height), channels_(channels) {
        assert(width_ > 0);
        assert(height_ > 0);
        assert(channels_ > 0);
        data_.resize(width * height * channels);
    }

    ~Image() = default;

    uint32_t width() const {
        return width_;
    }

    uint32_t height() const {
        return height_;
    }

    uint8_t channels() const {
        return channels_;
    }

    std::vector<uint8_t>& data() {
        return data_;
    }

    const std::vector<uint8_t>& data() const {
        return data_;
    }

private:
    uint32_t width_;
    uint32_t height_;
    uint8_t channels_;
    std::vector<uint8_t> data_;
};

void writePng(const Image& image, const std::filesystem::path& file);

void blur(Image& image, uint8_t blurKernelSize = 5);

} // namespace meshtools