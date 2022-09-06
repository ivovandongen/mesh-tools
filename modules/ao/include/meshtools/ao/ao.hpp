#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>
#include <meshtools/uv/atlas.hpp>

namespace meshtools::ao {

struct BakeOptions {
    int sizehint = 32;
    int nsamples = 128;
    bool gbuffer = false;
    bool chartinfo = false;
    float multiply = 1.0;
};

Result<Image> bake(const models::Model& input, const uv::Atlas& atlas, const BakeOptions& options = {});

} // namespace meshtools::ao
