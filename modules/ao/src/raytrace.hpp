#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>
#include <meshtools/uv/atlas.hpp>

#include <vector>

namespace meshtools::ao {

struct RaytraceOptions {
    int nsamples;
    uint8_t resultChannels;
    float far;
    float multiply;
};

Result<Image> raytrace(const models::Model& model, const uv::Atlas& atlas, RaytraceOptions = {});

} // namespace meshtools::ao