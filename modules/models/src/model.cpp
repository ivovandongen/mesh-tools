#include <meshtools/models/model.hpp>

#include <meshtools/logging.hpp>
#include <meshtools/string.hpp>

#include "./gltf/model.hpp"
#include "./obj/model.hpp"

#include <filesystem>
#include <utility>

namespace meshtools::models {

Model::Model(std::vector<Mesh> meshes) : meshes_(std::move(meshes)) {
    nodes_.reserve(meshes_.size());
    for (size_t i = 0; i < meshes_.size(); i++) {
        nodes_.emplace_back(i);
    }
}

Model::Model(std::vector<Mesh> meshes, std::vector<Node> nodes) : meshes_(std::move(meshes)), nodes_(std::move(nodes)) {}

Model::Model() = default;

Model::~Model() = default;

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

void Model::write(const std::filesystem::path& file) const {
    if (string::endsWith(file.string(), ".obj")) {
        logging::warn("Write not implemented for obj");
    } else if (string::endsWith(file.string(), ".gltf") || string::endsWith(file.string(), ".glb")) {
        gltf::write(*this, file);
    }
}

} // namespace meshtools::models
