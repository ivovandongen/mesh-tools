#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>

#include <array>
#include <memory>

namespace meshtools::uv {

class Vertex {
public:
    std::array<float, 2> uv;
    uint32_t xref;
};

struct AtlasCreateOptions {
    uint32_t maxSize = 0;
    uint32_t padding = 0;
    float texelsPerUnit = 0.0;
    uint32_t resolution = 0;
    bool bilinear = true;
    bool blockAlign = false;
    bool bruteForce = false;
};

class Atlas {
public:
    Atlas();
    ~Atlas();

    static Result<Atlas> Create(const models::Model& model, const AtlasCreateOptions& = {});

    uint32_t width() const;
    uint32_t height() const;

    void apply(models::Model& model);

    size_t meshCount() const;

    size_t indexCount(size_t meshIdx) const;

    // TODO: Lots of unnecessary copying here...
    Vertex vertex(size_t meshIdx, size_t indexIdx) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace meshtools::uv
