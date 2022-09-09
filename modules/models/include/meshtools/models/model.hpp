#pragma once

#include <meshtools/image.hpp>
#include <meshtools/math.hpp>
#include <meshtools/result.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace meshtools::models {

class Node {
public:
    explicit Node() = default;
    explicit Node(size_t mesh, glm::mat4 transform = glm::mat4{1}) : transform_(std::move(transform)), meshes_({mesh}) {}
    Node(std::initializer_list<size_t> meshes, glm::mat4 transform = glm::mat4{1}) : transform_(std::move(transform)), meshes_(meshes) {}

    const std::vector<size_t> meshes() const {
        return meshes_;
    }

    void meshes(std::vector<size_t> meshes) {
        meshes_ = std::move(meshes);
    }

    void transform(glm::mat4 transform) {
        transform_ = std::move(transform);
    }

    const glm::mat4& transform() const {
        return transform_;
    }

    std::vector<Node>& children() {
        return children_;
    }

private:
    glm::mat4 transform_{1};
    std::vector<size_t> meshes_;
    std::vector<Node> children_;
};


class Mesh {
public:
    Mesh(std::string name, size_t originalMeshIndex, std::vector<uint32_t> indices, std::vector<glm::vec3> positions,
         std::vector<glm::vec3> normals, std::vector<glm::vec2> texcoords)
        : name_(std::move(name)), originalMeshIndex_(originalMeshIndex), indices_(std::move(indices)), positions_(std::move(positions)),
          normals_(std::move(normals)), texcoords_(std::move(texcoords)) {}

    const std::vector<uint32_t>& indices() const {
        return indices_;
    };

    void indices(std::vector<uint32_t> indices) {
        indices_ = std::move(indices);
    };

    const std::vector<glm::vec3>& positions() const {
        return positions_;
    };

    void positions(std::vector<glm::vec3> positions) {
        positions_ = std::move(positions);
    };

    const std::vector<glm::vec3>& normals() const {
        return normals_;
    }

    void normals(std::vector<glm::vec3> normals) {
        normals_ = std::move(normals);
    };

    const std::vector<glm::vec2>& texcoords() const {
        return texcoords_;
    }


    void texcoords(std::vector<glm::vec2> uvs) {
        texcoords_ = std::move(uvs);
    }

    const std::string& name() const {
        return name_;
    }

    size_t originalMeshIndex() const {
        return originalMeshIndex_;
    }

private:
    std::string name_;
    size_t originalMeshIndex_;
    std::vector<uint32_t> indices_;
    std::vector<glm::vec3> positions_;
    std::vector<glm::vec3> normals_;
    std::vector<glm::vec2> texcoords_;
};

using ModelLoadResult = Result<class Model>;

class Model {
public:
    static ModelLoadResult Load(const std::filesystem::path& path);

    Model(std::vector<Mesh> meshes, std::vector<Node> nodes) : meshes_(std::move(meshes)), nodes_(std::move(nodes)) {}

    explicit Model(std::vector<Mesh> meshes) : meshes_(std::move(meshes)) {
        nodes_.reserve(meshes_.size());
        for (size_t i = 0; i < meshes_.size(); i++) {
            nodes_.emplace_back(i);
        }
    }

    explicit Model() = default;


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

    void dump(const Image& aoMap, const std::filesystem::path& file) const;

private:
    std::vector<Mesh> meshes_;
    std::vector<Node> nodes_;
};

} // namespace meshtools::models