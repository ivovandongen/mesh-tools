#include <meshtools/image.hpp>

#include <meshtools/file.hpp>
#include <meshtools/logging.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <cassert>
#include <cmath>

namespace meshtools {

Image::Image(uint32_t width, uint32_t height, uint8_t channels) : width_(width), height_(height), channels_(channels), type_(Type::RAW) {
    assert(width_ > 0);
    assert(height_ > 0);
    assert(channels_ > 0);
    data_.resize(width * height * channels);
}

Image::Image(uint32_t width, uint32_t height, uint8_t channels, Type type, std::vector<uint8_t> data)
    : width_(width), height_(height), channels_(channels), type_(type), data_(std::move(data)) {
    assert(width_ > 0);
    assert(height_ > 0);
    assert(channels_ > 0);
}

Image Image::png() const {
    std::vector<unsigned char> result{};
    result.reserve(data_.size()); // conservative
    stbi_write_png_to_func(
            [](void* context, void* data, int size) {
                auto out = (std::vector<unsigned char>*) context;
                std::copy_n((unsigned char*) data, size, std::back_inserter(*out));
            },
            &result,
            width(),
            height(),
            channels(),
            data().data(),
            width() * channels());
    return Image{width_, height_, channels_, Type::PNG, std::move(result)};
}

Image Image::jpg(uint8_t quality) const {
    std::vector<unsigned char> result{};
    result.reserve(data_.size()); // conservative
    stbi_write_jpg_to_func(
            [](void* context, void* data, int size) {
                auto out = (std::vector<unsigned char>*) context;
                std::copy_n((unsigned char*) data, size, std::back_inserter(*out));
            },
            &result,
            width(),
            height(),
            channels(),
            data().data(),
            quality);
    return Image{width_, height_, channels_, Type::JPG, std::move(result)};
}

void Image::blur(uint8_t blurKernelSize) {
    // Do bilateral blur pass
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            auto srcIdx = y * width() + x;

            double sum = 0.0;
            const double sigma = 4.0;
            size_t minIdx = 0;
            size_t maxIdx = data_.size() - 1;

            double color = 0;

            for (int sy = -blurKernelSize; sy <= blurKernelSize; sy++) {
                for (int sx = -blurKernelSize; sx <= blurKernelSize; sx++) {
                    auto hypotSq = (sx * sx + sy * sy);
                    auto div = 2.0 * sigma * sigma;
                    auto weight = std::exp(-hypotSq / div) / (div * M_PI);
                    auto idx = std::clamp<size_t>((y + sy) * width() + (x + sx), minIdx, maxIdx);

                    auto val = data_[idx * channels() + 0];
                    if (val == 0)
                        continue;
                    color += val * weight;
                    sum += weight;
                }
            }

            color /= sum;

            for (size_t i = 0; i < channels(); i++) {
                data()[srcIdx * channels() + i] = (uint8_t) (std::min(color, 255.0));
            }
            if (channels() == 4) {
                data()[srcIdx * 4 + 3] = 255;
            }
        }
    }
}

} // namespace meshtools