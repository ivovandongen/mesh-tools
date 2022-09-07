#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>
#include <meshtools/size.hpp>

namespace meshtools::ao {

struct BakeOptions {
    int sizehint = 32;
    int nsamples = 128;
    bool gbuffer = false;
    bool chartinfo = false;
    float multiply = 1.0;
    float maxFar = 5.0;
    uint8_t channels = 1;
};

Result<Image> bake(const models::Model& input, const Size<uint32_t>& mapSize, const BakeOptions& options = {});

} // namespace meshtools::ao
