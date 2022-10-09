#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/models/attribute_type.hpp>
#include <meshtools/models/extras.hpp>
#include <meshtools/models/mesh_data.hpp>

#include <string>
#include <unordered_map>
#include <utility>

namespace meshtools::models {

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

    TypedData& indices() {
        return indices_;
    }

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

    VertexData& vertexData() {
        return vertexData_;
    }

    const VertexData& vertexData() const {
        return vertexData_;
    }

    void vertexData(VertexData vertexData) {
        vertexData_ = std::move(vertexData);
    }

    Extra& extra() {
        return extra_;
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

} // namespace meshtools::models