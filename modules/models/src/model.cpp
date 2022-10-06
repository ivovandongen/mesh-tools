#include <meshtools/models/model.hpp>

#include <meshtools/file.hpp>
#include <meshtools/logging.hpp>
#include <meshtools/string.hpp>

#include "./gltf/model.hpp"
#include "./obj/model.hpp"

#include <filesystem>
#include <utility>

namespace meshtools::models {

Model::Model(std::vector<MeshGroup> meshGroups) : meshGroups_(std::move(meshGroups)) {
    auto& scene = scenes_[0];
    scene.reserve(meshGroups_.size());
    for (size_t i = 0; i < meshGroups_.size(); i++) {
        scene.emplace_back(i);
    }
}

Model::Model(std::vector<MeshGroup> meshGroups, std::vector<Node> nodes)
    : meshGroups_(std::move(meshGroups)), scenes_({std::move(nodes)}) {}

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

ModelLoadResult Model::Load(const std::string& contents, bool binary) {
    auto loadResult = gltf::LoadModel(contents, binary);

    if (!loadResult) {
        logging::error("Could not load model {}", loadResult.error);
    }

    return loadResult;
}

void Model::merge(const Model& model) {
    // Images
    auto imageOffset = images_.size();
    images_.insert(images_.end(), model.images().begin(), model.images().end());

    // Samplers
    auto samplerOffset = samplers_.size();
    samplers_.insert(samplers_.end(), model.samplers().begin(), model.samplers().end());

    // textures
    auto texturesOffset = textures_.size();
    for (auto& texture : model.textures()) {
        int sampler = texture.sampler > -1 ? texture.sampler + samplerOffset : -1;
        int source = texture.source > -1 ? texture.source + imageOffset : -1;
        textures_.push_back(Texture{sampler, source});
    }

    // Materials
    auto materialOffset = materials_.size();
    for (auto material : model.materials()) {
        if (material.occlusionTexture > -1) {
            material.occlusionTexture += texturesOffset;
        }
        if (material.pbrMetallicRoughness.baseColorTexture > -1) {
            material.pbrMetallicRoughness.baseColorTexture += texturesOffset;
        }
        materials_.push_back(material);
    }

    // Meshes
    auto meshOffset = meshGroups_.size();
    for (auto& meshGroup : model.meshGroups()) {
        // TODO: Copy meshes?
        auto& meshGroupNew = meshGroups_.emplace_back(meshGroup.name(), meshGroup.meshes(), meshGroup.extra());

        // Update material references
        for (auto& mesh : meshGroupNew.meshes()) {
            if (mesh->materialIdx() > -1) {
                mesh->materialIdx(mesh->materialIdx() + materialOffset);
            }
        }
    }

    // Nodes
    // XXX: scene 0 only for now
    for (auto& node : model.nodes(0)) {
        // Update scene
        auto& newNode = nodes(0).emplace_back(node.mesh(), node.extra(), node.transform());
        newNode.visit([&](Node& node) {
            if (node.mesh()) {
                node.mesh(*node.mesh() + meshOffset);
            }
        });
    }
}

void Model::write(const std::filesystem::path& file) const {
    if (string::endsWith(file.string(), ".obj")) {
        logging::warn("Write not implemented for obj");
    } else if (string::endsWith(file.string(), ".gltf")) {
        auto encoded = text();
        file::writeFile(file, encoded);
    } else if (string::endsWith(file.string(), ".glb")) {
        auto encoded = binary();
        file::writeFile(file, encoded, true);
    }
}

std::string Model::text() const {
    return gltf::text(*this);
}

std::vector<char> Model::binary() const {
    return gltf::binary(*this);
}

} // namespace meshtools::models
