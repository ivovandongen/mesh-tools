#pragma once

#include <meshtools/models/model.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace meshtools::models::gltf {

ModelLoadResult LoadModel(const std::filesystem::path& file);
ModelLoadResult LoadModel(const std::string& contents, bool binary);

std::string text(const Model&);

std::vector<char> binary(const Model&);

} // namespace meshtools::models::gltf