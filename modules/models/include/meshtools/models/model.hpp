#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/image.hpp>
#include <meshtools/math.hpp>
#include <meshtools/models/extras.hpp>
#include <meshtools/models/material.hpp>
#include <meshtools/models/mesh.hpp>
#include <meshtools/models/mesh_group.hpp>
#include <meshtools/models/node.hpp>
#include <meshtools/models/sampler.hpp>
#include <meshtools/models/texture.hpp>
#include <meshtools/result.hpp>

#include <filesystem>
#include <vector>

namespace meshtools::models {

using ModelLoadResult = Result<class Model>;

class Model {
public:
    static ModelLoadResult Load(const std::filesystem::path& path);
    static ModelLoadResult Load(const std::string& contents, bool binary);

    Model(std::vector<MeshGroup> meshGroups, std::vector<Node> nodes, Extra extra = {});

    explicit Model(std::vector<MeshGroup> meshGroups, Extra extra = {});

    explicit Model();

    ~Model();

    std::vector<MeshGroup>& meshGroups() {
        return meshGroups_;
    }

    const std::vector<MeshGroup>& meshGroups() const {
        return meshGroups_;
    }

    std::shared_ptr<Mesh>& mesh(size_t index) {
        size_t i = 0;
        for (auto& meshGroup : meshGroups_) {
            auto localIndex = index - i;
            if (localIndex < meshGroup.meshes().size()) {
                return meshGroup.meshes()[localIndex];
            }
            i += meshGroup.meshes().size();
        }

        throw std::runtime_error("No mesh for this index");
        assert(false);
    }

    const std::shared_ptr<Mesh>& mesh(size_t index) const {
        size_t i = 0;
        for (auto& meshGroup : meshGroups_) {
            auto localIndex = index - i;
            if (localIndex < meshGroup.meshes().size()) {
                return meshGroup.meshes()[localIndex];
            }
            i += meshGroup.meshes().size();
        }

        throw std::runtime_error("No mesh for this index");
        assert(false);
    }

    std::vector<std::shared_ptr<Mesh>> meshes(size_t scene, bool applyLocalTransforms,
                                              const glm::mat4& rootTransform = glm::mat4{1}) const {
        const constexpr glm::mat4 identity{1};
        assert(scene < scenes_.size());
        auto& nodes = scenes_[scene];
        std::vector<std::shared_ptr<Mesh>> meshes;
        for (auto& node : nodes) {
            glm::mat4 cumulativeTransform = rootTransform;
            const auto visitor = [&](const Node& node) {
                auto meshIdx = node.mesh();
                if (meshIdx) {
                    const auto& meshGroup = meshGroups_[*meshIdx];
                    // Cascade local transforms
                    cumulativeTransform = cumulativeTransform * node.transform();

                    for (auto& orgMesh : meshGroup.meshes()) {

                        if (!applyLocalTransforms || cumulativeTransform == identity) {
                            meshes.template emplace_back(orgMesh);
                        } else {
                            TypedData indices{
                                    orgMesh->indices().dataType(),
                                    orgMesh->indices().componentCount(),
                                    orgMesh->indices().buffer(),
                            };

                            VertexData vertexData;
                            for (const auto& va : orgMesh->vertexData()) {
                                vertexData[va.first] = {va.second.dataType(), va.second.componentCount(), va.second.buffer()};
                            }

                            // Transform positions
                            DataView<glm::vec3> positions{vertexData[AttributeType::POSITION]};
                            std::vector<glm::vec3> transformed;
                            transformed.reserve(positions.size());
                            for (auto& pos : positions) {
                                transformed.emplace_back(cumulativeTransform * glm::vec4{pos, 1});
                            }
                            vertexData[AttributeType::POSITION].copyFrom(transformed.data());

                            meshes.emplace_back(std::make_shared<Mesh>(orgMesh->name(),
                                                                       orgMesh->materialIdx(),
                                                                       std::move(indices),
                                                                       std::move(vertexData),
                                                                       orgMesh->extra()));
                        }
                    }
                }
            };
            node.visit(visitor);
        }
        return meshes;
    }

    template<class MeshVisitor>
    void visit(const MeshVisitor& visitor) {
        for (auto& meshGroup : meshGroups_) {
            for (auto& mesh : meshGroup.meshes()) {
                visitor(*mesh);
            }
        }
    }

    const std::vector<Node>& nodes(size_t scene) const {
        assert(scene < scenes_.size());
        return scenes_[scene];
    }

    std::vector<Node>& nodes(size_t scene) {
        if (scene >= scenes_.size()) {
            scenes_.resize(scene + 1);
        }
        return scenes_[scene];
    }

    const std::vector<std::shared_ptr<Image>>& images() const {
        return images_;
    }

    std::vector<std::shared_ptr<Image>>& images() {
        return images_;
    }

    const std::vector<Sampler>& samplers() const {
        return samplers_;
    }

    std::vector<Sampler>& samplers() {
        return samplers_;
    }

    const std::vector<Texture>& textures() const {
        return textures_;
    }

    std::vector<Texture>& textures() {
        return textures_;
    }

    const std::vector<Material>& materials() const {
        return materials_;
    }

    std::vector<Material>& materials() {
        return materials_;
    }

    Extra& extra() {
        return extra_;
    }

    const Extra& extra() const {
        return extra_;
    }

    void merge(const Model& model);

    void write(const std::filesystem::path& outFile) const;

    std::string text() const;

    std::vector<char> binary() const;

private:
    std::vector<MeshGroup> meshGroups_;
    std::vector<std::vector<Node>> scenes_;
    std::vector<std::shared_ptr<Image>> images_;
    std::vector<Sampler> samplers_;
    std::vector<Texture> textures_;
    std::vector<Material> materials_;
    Extra extra_;
};

} // namespace meshtools::models