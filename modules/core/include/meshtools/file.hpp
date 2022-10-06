#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

namespace meshtools::file {

static inline bool parentDirExists(const std::filesystem::path& path) {
    return !path.empty() && std::filesystem::exists(path.parent_path());
}

template<class T>
inline void writeFile(const std::string& fileName, const T& data, bool binary = false) {
    // TMP
    auto flags = std::ios::out;
    if (binary) {
        flags |= std::ios::binary;
    }
    std::ofstream file(fileName, flags);
    if (file.is_open()) {
        file.write(data.data(), data.size());
        file.close();
    } else {
        std::cerr << "Cannot write to file: " << fileName << std::endl;
        abort();
    }
}

} // namespace meshtools::file