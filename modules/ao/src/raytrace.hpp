#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>

#include <vector>
#include <xatlas.h>

namespace meshtools::ao {

struct RaytraceOptions {
    int nsamples;
    uint8_t resultChannels;
    float far;
    float multiply;
};

Result<Image> raytrace(const models::Model& model, const xatlas::Atlas& atlas, RaytraceOptions = {});

} // namespace meshtools::ao