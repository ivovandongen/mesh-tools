#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>
#include <meshtools/size.hpp>

#include <memory>
#include <vector>

namespace meshtools::ao {

struct BakeOptions {
    int nsamples = 128;
    float multiply = 1.0;
    float maxFar = 5.0;
    uint8_t channels = 1;
};

Result<Image> bake(const std::vector<std::shared_ptr<models::Mesh>>& input, const Size<uint32_t>& mapSize, const BakeOptions& options = {});

} // namespace meshtools::ao
