#pragma once

#include <meshtools/algorithm.hpp>
#include <meshtools/models/mesh_data.hpp>

#include <string>
#include <unordered_map>

namespace meshtools::models {

enum class AttributeType {
    POSITION,
    NORMAL,
    TEXCOORD,
    COLOR,
    UNKNOWN,
};

class Mesh {
public:
    Mesh(std::string name, size_t originalMeshIndex, int materialIdx, TypedData indices,
         std::unordered_map<AttributeType, TypedData> vertexData)
        : name_(std::move(name)), originalMeshIndex_(originalMeshIndex), materialIdx_(materialIdx), indices_(std::move(indices)),
          vertexData_(std::move(vertexData)) {}

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

    size_t originalMeshIndex() const {
        return originalMeshIndex_;
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

    const std::unordered_map<AttributeType, TypedData>& vertexData() const {
        return vertexData_;
    }

    void vertexData(std::unordered_map<AttributeType, TypedData> vertexData) {
        vertexData_ = std::move(vertexData);
    }

private:
    std::string name_;
    size_t originalMeshIndex_;
    int materialIdx_;
    TypedData indices_;
    std::unordered_map<AttributeType, TypedData> vertexData_;
};

} // namespace meshtools::models