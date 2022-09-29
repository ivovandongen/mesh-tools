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
    Mesh(std::string name, int materialIdx, TypedData indices, VertexData vertexData, Extras extras = {})
        : name_(std::move(name)), materialIdx_(materialIdx), indices_(std::move(indices)), vertexData_(std::move(vertexData)) {}

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

    const Extras& extras() const {
        return extras_;
    }

private:
    std::string name_;
    int materialIdx_;
    TypedData indices_;
    VertexData vertexData_;
    Extras extras_;
};

class MeshGroup {
public:
    MeshGroup(std::string name, Mesh mesh, Extras extras = {}) : name_(std::move(name)), extras_(std::move(extras)) {
        meshes_.push_back(std::move(mesh));
    }

    MeshGroup(std::string name, std::vector<Mesh> meshes, Extras extras = {})
        : name_(std::move(name)), meshes_(std::move(meshes)), extras_(std::move(extras)) {}

    const std::string& name() const {
        return name_;
    }

    std::vector<Mesh>& meshes() {
        return meshes_;
    }

    const std::vector<Mesh>& meshes() const {
        return meshes_;
    }

    const Extras& extras() const {
        return extras_;
    }

private:
    std::string name_;
    std::vector<Mesh> meshes_;
    Extras extras_;
};

} // namespace meshtools::models