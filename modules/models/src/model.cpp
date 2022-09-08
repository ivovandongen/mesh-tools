#include <meshtools/models/model.hpp>

#include <meshtools/logging.hpp>
#include <meshtools/string.hpp>

#include "./gltf/model.hpp"
#include "./obj/model.hpp"

#include <filesystem>

namespace meshtools::models {

ModelLoadResult Model::Load(const std::filesystem::path& path) {
    auto loadModel = [](const auto& path) -> ModelLoadResult {
        if (string::endsWith(path.string(), ".obj")) {
            return obj::loadModel(path);
        } else if (string::endsWith(path.string(), ".gltf") || string::endsWith(path.string(), ".glb")) {
            return gltf::LoadModel(path);
        }

        return ModelLoadResult{std::string{"Unknown model format: "} + path.c_str()};
    };

    auto loadResult = loadModel(path);

    if (!loadResult) {
        logging::error("Could not load model {}", path.c_str());
    }

    return loadResult;
}

void Model::dump(const Image& aoMap, const std::filesystem::path& file) const {
    if (string::endsWith(file.string(), ".obj")) {
        obj::dump(*this, aoMap, file);
    } else if (string::endsWith(file.string(), ".gltf") || string::endsWith(file.string(), ".glb")) {
        gltf::dump(*this, aoMap, file);
    }
}

} // namespace meshtools::models
