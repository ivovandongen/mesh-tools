#pragma once

#include <meshtools/logging.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace meshtools::file {

static inline bool parentDirExists(const std::filesystem::path& path) {
    return !path.empty() && std::filesystem::exists(path.parent_path());
}

template<class T>
inline void writeFile(const std::string& fileName, const T& data, bool binary = false) {
    auto flags = binary ? std::ios::binary : std::ios::out;
    {
        std::ofstream file(fileName, flags);
        if (file.is_open()) {
            file.write((char*) data.data(), data.size());
        } else {
            logging::error("Cannot write to file ${}", fileName.c_str());
            throw std::runtime_error("Cannot write to file");
        }
    }
}

} // namespace meshtools::file