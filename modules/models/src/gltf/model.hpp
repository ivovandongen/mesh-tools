#pragma once

#include <meshtools/models/model.hpp>

#include <filesystem>

namespace meshtools::models::gltf {

ModelLoadResult LoadModel(const std::filesystem::path& file);

void dump(const Model& model, const Image& aoMap, const std::filesystem::path& file);

} // namespace meshtools::models::gltf