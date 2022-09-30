#include <meshtools/ao/ao.hpp>

#include <meshtools/models/model.hpp>

#include "rasterize.hpp"
#include "raytrace.hpp"

namespace meshtools::ao {

inline RaytraceOptions createOptions(const BakeOptions& options) {
    return {
            .nsamples = options.nsamples,
            .resultChannels = options.channels,
            .far = options.maxFar,
            .multiply = options.multiply,
    };
}

Result<Image> bake(const std::vector<std::shared_ptr<models::Mesh>>& meshes, const Size<uint32_t>& mapSize, const BakeOptions& options) {
    auto raytraceResult = raytrace(meshes, mapSize, createOptions(options));
    if (!raytraceResult) {
        return {std::move(raytraceResult.error)};
    }
    return {std::move(raytraceResult.value)};
}

} // namespace meshtools::ao
