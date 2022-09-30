#pragma once

#include <meshtools/math.hpp>
#include <meshtools/models/extras.hpp>

#include <vector>

namespace meshtools::models {

class Node {
public:
    Node() = default;
    explicit Node(std::optional<size_t> mesh, Extra extra = {}, glm::mat4 transform = glm::mat4{1})
        : transform_(std::move(transform)), mesh_(mesh), extra_(std::move(extra)) {}

    std::optional<size_t> mesh() const {
        return mesh_;
    }

    void mesh(size_t mesh) {
        mesh_ = mesh;
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

    const std::vector<Node>& children() const {
        return children_;
    }

    void extra(Extra extra) {
        extra_ = std::move(extra);
    }

    const Extra& extra() const {
        return extra_;
    }

    template<class Visitor>
    void visit(const Visitor& visitor) const {
        visitor(*this);

        for (const auto& child : children()) {
            child.visit(visitor);
        }
    }

    template<class Visitor, class State>
    void visit(const Visitor& visitor, State state) const {
        state = visitor(*this, state);

        for (const auto& child : children()) {
            child.visit(visitor, state);
        }
    }

private:
    glm::mat4 transform_{1};
    std::optional<size_t> mesh_;
    std::vector<Node> children_;
    std::string name_;
    Extra extra_;
};

} // namespace meshtools::models