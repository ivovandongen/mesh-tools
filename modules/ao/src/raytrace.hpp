#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>
#include <meshtools/result.hpp>
#include <meshtools/size.hpp>

#include <memory>
#include <vector>

namespace meshtools::ao {

struct RaytraceOptions {
    int nsamples;
    uint8_t resultChannels;
    float far;
    float multiply;
};

Result<Image> raytrace(const std::vector<std::shared_ptr<models::Mesh>>& meshes, const Size<uint32_t>& size, RaytraceOptions = {});

} // namespace meshtools::ao