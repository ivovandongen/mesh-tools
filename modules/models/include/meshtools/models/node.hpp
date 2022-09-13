#pragma once

#include <meshtools/math.hpp>

#include <vector>

namespace meshtools::models {

class Node {
public:
    explicit Node() = default;
    explicit Node(size_t mesh, glm::mat4 transform = glm::mat4{1}) : transform_(std::move(transform)), meshes_({mesh}) {}
    Node(std::initializer_list<size_t> meshes, glm::mat4 transform = glm::mat4{1}) : transform_(std::move(transform)), meshes_(meshes) {}

    const std::vector<size_t>& meshes() const {
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

} // namespace meshtools::models