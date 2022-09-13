#pragma once

#include <filesystem>
#include <vector>

namespace meshtools {

struct Image {
    Image(uint32_t width, uint32_t height, uint8_t channels);

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

    std::string& name() {
        return name_;
    }

    const std::string& name() const {
        return name_;
    }

    void blur(uint8_t blurKernelSize = 5);

    void png(const std::filesystem::path& file) const;

    std::vector<unsigned char> png() const;

private:
    std::string name_;
    uint32_t width_;
    uint32_t height_;
    uint8_t channels_;
    std::vector<uint8_t> data_;
};

} // namespace meshtools