#pragma once

#include <tiny_gltf.h>

#include <filesystem>

namespace meshtools::models::gltf {

std::pair<bool, tinygltf::Model> LoadModel(const std::filesystem::path& file);

} // namespace meshtools::models::gltf