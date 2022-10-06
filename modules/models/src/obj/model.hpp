#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>

#include <filesystem>

namespace meshtools::models::obj {

ModelLoadResult loadModel(const std::filesystem::path& filename);

} // namespace meshtools::models::obj
