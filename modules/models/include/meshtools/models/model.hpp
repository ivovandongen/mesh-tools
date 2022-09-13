#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/image.hpp>
#include <meshtools/math.hpp>
#include <meshtools/models/material.hpp>
#include <meshtools/models/mesh.hpp>
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
    class Impl;

    static ModelLoadResult Load(const std::filesystem::path& path);

    Model(std::vector<Mesh> meshes, std::vector<Node> nodes);

    explicit Model(std::vector<Mesh> meshes);

    explicit Model();

    ~Model();

    std::vector<Mesh>& meshes() {
        return meshes_;
    }

    const std::vector<Mesh>& meshes() const {
        return meshes_;
    }

    const std::vector<Node>& nodes() const {
        return nodes_;
    }

    std::vector<Node>& nodes() {
        return nodes_;
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

    void dump(const Image& aoMap, const std::filesystem::path& file) const;

    void write(const std::filesystem::path& outFile) const;

private:
    std::vector<Mesh> meshes_;
    std::vector<Node> nodes_;
    std::vector<std::shared_ptr<Image>> images_;
    std::vector<Sampler> samplers_;
    std::vector<Texture> textures_;
    std::vector<Material> materials_;
};

} // namespace meshtools::models