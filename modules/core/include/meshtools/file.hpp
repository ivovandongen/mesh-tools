#pragma once

#include <filesystem>

namespace meshtools::file {

static inline bool parentDirExists(const std::filesystem::path& path) {
    return !path.empty() && std::filesystem::exists(path.parent_path());
}

} // namespace meshtools::file