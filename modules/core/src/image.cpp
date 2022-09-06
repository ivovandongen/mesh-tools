#include <meshtools/image.hpp>

#include <meshtools/file.hpp>
#include <meshtools/logging.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cmath>

namespace meshtools {

void writePng(const Image& image, const std::filesystem::path& file) {
    if (!file::parentDirExists(file)) {
        logging::error("Directory for {} does not exist", file.c_str());
        return;
    }
    stbi_write_png(file.c_str(), image.width(), image.height(), image.channels(), image.data().data(), image.width() * image.channels());
}

void blur(Image& image, uint8_t blurKernelSize) {
    // Do bilateral blur pass
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            auto srcIdx = y * image.width() + x;

            double sum = 0.0;
            const double sigma = 4.0;

            double color = 0;

            for (int sy = -blurKernelSize; sy <= blurKernelSize; sy++) {
                for (int sx = -blurKernelSize; sx <= blurKernelSize; sx++) {
                    auto hypotSq = (sx * sx + sy * sy);
                    auto div = 2.0 * sigma * sigma;
                    auto weight = std::exp(-hypotSq / div) / (div * M_PI);
                    auto idx = (y + sy) * image.width() + (x + sx);

                    auto val = image.data()[idx * image.channels() + 0];
                    if (val == 0)
                        continue;
                    color += val * weight;
                    sum += weight;
                }
            }

            color /= sum;

            for (size_t i = 0; i < image.channels(); i++) {
                image.data()[srcIdx * image.channels() + i] = (uint8_t) (std::min(color, 255.0));
            }
            if (image.channels() == 4) {
                image.data()[srcIdx * 4 + 3] = 255;
            }
        }
    }
}

} // namespace meshtools