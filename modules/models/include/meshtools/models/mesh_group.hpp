#pragma once

#include <meshtools/models/extras.hpp>
#include <meshtools/models/mesh.hpp>

#include <string>
#include <unordered_map>

namespace meshtools::models {


class MeshGroup {
public:
    MeshGroup(std::string name, std::shared_ptr<Mesh> mesh, Extra extra = {}) : name_(std::move(name)), extra_(std::move(extra)) {
        meshes_.push_back(std::move(mesh));
    }

    MeshGroup(std::string name, std::vector<std::shared_ptr<Mesh>> meshes, Extra extra = {})
        : name_(std::move(name)), meshes_(std::move(meshes)), extra_(std::move(extra)) {}

    const std::string& name() const {
        return name_;
    }

    std::vector<std::shared_ptr<Mesh>>& meshes() {
        return meshes_;
    }

    const std::vector<std::shared_ptr<Mesh>>& meshes() const {
        return meshes_;
    }

    Extra& extra() {
        return extra_;
    }

    const Extra& extra() const {
        return extra_;
    }

    void merge(bool discardMaterials = true);

private:
    std::string name_;
    std::vector<std::shared_ptr<Mesh>> meshes_;
    Extra extra_;
};

} // namespace meshtools::models