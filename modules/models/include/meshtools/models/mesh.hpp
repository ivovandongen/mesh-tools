#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/models/extras.hpp>
#include <meshtools/models/mesh_data.hpp>

#include <string>
#include <unordered_map>
#include <utility>

namespace meshtools::models {

enum class AttributeType {
    POSITION,
    NORMAL,
    TEXCOORD,
    COLOR,
    UNKNOWN,
};

using VertexData = std::unordered_map<AttributeType, TypedData>;

class Mesh {
public:
    Mesh(std::string name, int materialIdx, TypedData indices, VertexData vertexData, Extra extra = {})
        : name_(std::move(name)), materialIdx_(materialIdx), indices_(std::move(indices)), vertexData_(std::move(vertexData)),
          extra_(std::move(extra)) {}

    // Delete copy
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Keep move
    Mesh(Mesh&&) = default;
    Mesh& operator=(Mesh&&) = default;

    const TypedData& indices() const {
        return indices_;
    }

    template<class T>
    DataView<T> indices() const {
        return DataView<T>{indices_};
    }

    void indices(TypedData indices) {
        assert(indices.dataType() == DataType::U_SHORT || indices.dataType() == DataType::U_INT);
        indices_ = std::move(indices);
    }

    const TypedData& vertexAttribute(AttributeType attributeType) const {
        assert(vertexData_.find(attributeType) != vertexData_.end());
        return vertexData_.find(attributeType)->second;
    }

    TypedData& vertexAttribute(AttributeType attributeType) {
        return vertexData_[attributeType];
    }

    template<class T>
    DataView<T> vertexAttribute(AttributeType attributeType) const {
        assert(vertexData_.find(attributeType) != vertexData_.end());
        return DataView<T>{vertexData_.find(attributeType)->second};
    }

    bool hasVertexAttribute(AttributeType attributeType) const {
        return vertexData_.find(attributeType) != vertexData_.end();
    }

    const std::string& name() const {
        return name_;
    }

    int materialIdx() const {
        return materialIdx_;
    }

    void materialIdx(int materialIdx) {
        materialIdx_ = materialIdx;
    }

    std::vector<AttributeType> vertexAttributes() const {
        return meshtools::transform<AttributeType>(vertexData_, [](const auto& va) { return va.first; });
    }

    void removeAttribute(AttributeType attribute) {
        vertexData_.erase(attribute);
    }

    const VertexData& vertexData() const {
        return vertexData_;
    }

    void vertexData(VertexData vertexData) {
        vertexData_ = std::move(vertexData);
    }

    const Extra& extra() const {
        return extra_;
    }

private:
    std::string name_;
    int materialIdx_;
    TypedData indices_;
    VertexData vertexData_;
    Extra extra_;
};

class MeshGroup {
public:
    MeshGroup(std::string name, std::shared_ptr<Mesh> mesh, Extra extra = {}) : name_(std::move(name)), extra_(std::move(extra)) {
        meshes_.push_back(std::move(mesh));
    }

    MeshGroup(std::string name, std::vector<std::shared_ptr<Mesh>> meshes, Extra extra = {})
        : name_(std::move(name)), meshes_(std::move(meshes)), extra_(std::move(extra)) {}

    // Delete copy
    MeshGroup(const MeshGroup&) = delete;
    MeshGroup& operator=(const MeshGroup&) = delete;

    // Keep move
    MeshGroup(MeshGroup&&) = default;
    MeshGroup& operator=(MeshGroup&&) = default;

    const std::string& name() const {
        return name_;
    }

    std::vector<std::shared_ptr<Mesh>>& meshes() {
        return meshes_;
    }

    const std::vector<std::shared_ptr<Mesh>>& meshes() const {
        return meshes_;
    }

    const Extra& extras() const {
        return extra_;
    }

private:
    std::string name_;
    std::vector<std::shared_ptr<Mesh>> meshes_;
    Extra extra_;
};

} // namespace meshtools::models