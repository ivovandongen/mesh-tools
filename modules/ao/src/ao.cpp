#include <meshtools/ao/ao.hpp>

#include <meshtools/models/model.hpp>

#include "rasterize.hpp"
#include "raytrace.hpp"

namespace meshtools::ao {

Result<Image> bake(const models::Model& model, const uv::Atlas& atlas, const BakeOptions& options) {
    const constexpr uint8_t channels = 1;
    const constexpr float maxFar = 5;
    RaytraceOptions raytraceOptions{.nsamples = options.nsamples, .resultChannels = channels, .far = maxFar, .multiply = options.multiply};
    auto raytraceResult = raytrace(model, atlas, raytraceOptions);
    if (!raytraceResult) {
        return {std::move(raytraceResult.error)};
    }

    return {std::move(raytraceResult.value)};
}

} // namespace meshtools::ao
