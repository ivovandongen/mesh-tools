#pragma once

#include <filesystem>
#include <vector>

namespace meshtools {

struct Image {
    enum class Type { RAW, PNG, JPG };

    Image(uint32_t width, uint32_t height, uint8_t channels);

    Image(uint32_t width, uint32_t height, uint8_t channels, Type, std::vector<uint8_t> data);

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

    const Type type() const {
        return type_;
    }

    void blur(uint8_t blurKernelSize = 5);

    Image png() const;

    Image jpg(uint8_t quality = 50) const;

private:
    std::string name_;
    uint32_t width_;
    uint32_t height_;
    uint8_t channels_;
    Type type_;
    std::vector<uint8_t> data_;
};

} // namespace meshtools