#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>

// XXX: Remove
#include <xatlas.h>

namespace meshtools::ao {

struct BakeOptions {
    int sizehint = 32;
    int nsamples = 128;
    bool gbuffer = false;
    bool chartinfo = false;
    float multiply = 1.0;
};

struct Atlas {
    Atlas();
    ~Atlas();

    xatlas::Atlas* impl;
};

Result<Atlas> createAtlas(const models::Model& model);

Result<Image> bake(const models::Model& input, const Atlas& atlas, const BakeOptions& options = {});

} // namespace meshtools::ao
