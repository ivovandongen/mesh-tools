#include "gltf_impl.hpp"

#include <meshtools/logging.hpp>
#include <meshtools/string.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>

namespace meshtools::models::gltf {

std::pair<bool, tinygltf::Model> LoadModel(const std::filesystem::path& file) {

    using namespace tinygltf;

    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;

    bool result;
    if (string::endsWith(file, ".gltf")) {
        result = loader.LoadASCIIFromFile(&model, &err, &warn, file);
    } else if (string::endsWith(file, ".glb")) {
        result = loader.LoadBinaryFromFile(&model, &err, &warn, file);
    } else {
        logging::error("Unknown file format: {}", file.c_str());
        result = false;
    }


    if (!warn.empty()) {
        logging::warn("Warn: {}", warn.c_str());
    }

    if (!err.empty()) {
        logging::warn("Warn: {}", warn.c_str());
        printf("Err: %s\n", err.c_str());
    }

    return {result, std::move(model)};
}

} // namespace meshtools::models::gltf