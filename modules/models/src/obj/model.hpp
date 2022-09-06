#pragma once

#include <meshtools/image.hpp>
#include <meshtools/models/model.hpp>

#include <xatlas.h>

#include <filesystem>

namespace meshtools::models::obj {

ModelLoadResult loadModel(const std::filesystem::path& filename);

void dump(const Model& model, const xatlas::Atlas& atlas, const Image& aoMap, const std::filesystem::path& file);

} // namespace meshtools::models::obj
