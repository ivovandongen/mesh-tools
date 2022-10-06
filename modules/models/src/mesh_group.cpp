#include <meshtools/models/mesh_group.hpp>

#include <meshtools/logging.hpp>
#include <meshtools/models/attribute_type.hpp>

namespace meshtools::models {

void MeshGroup::merge(bool discardMaterials) {
    // TODO: name?
    // TODO: extras?
    if (meshes_.size() <= 1) {
        return;
    }

    // Ensure all meshes have the same set of vertex attributes
    if (!std::all_of(meshes_.begin() + 1, meshes_.end(), [&](const std::shared_ptr<Mesh>& mesh) {
            // Check counts
            if (mesh->vertexData().size() != meshes_[0]->vertexData().size()) {
                return false;
            }

            // Check keys
            return std::all_of(meshes_[0]->vertexData().begin(), meshes_[0]->vertexData().end(), [&](auto& entry) {
                return mesh->hasVertexAttribute(entry.first);
            });
        })) {
        logging::warn("Cannot merge meshes for mesh group {} as the meshes have different vertex attributes");
        return;
    }

    // Check material differences
    if (!discardMaterials && !std::all_of(meshes_.begin() + 1, meshes_.end(), [&](const std::shared_ptr<Mesh>& mesh) {
            return mesh->materialIdx() == meshes_[0]->materialIdx();
        })) {
        logging::warn("Cannot merge meshes for mesh group {} as the meshes have different materials");
        return;
    }

#ifndef NDEBUG
    auto totalVertices = std::accumulate(meshes_.begin(), meshes_.end(), 0, [](size_t acc, auto& mesh) {
        return acc + mesh->vertexAttribute(AttributeType::POSITION).size();
    });
    auto totalIndices =
            std::accumulate(meshes_.begin(), meshes_.end(), 0, [](size_t acc, auto& mesh) { return acc + mesh->indices().size(); });
#endif

    // TODO: Optionally allow upgrading uint16 indices to uint32
    // TODO: Check if all indices are the same type
    bool indices16bit = meshes_[0]->indices().dataType() == DataType::U_SHORT;

    std::vector<std::shared_ptr<Mesh>> result{meshes_[0]};
    Mesh* accumulator = &*result[0];
    for (size_t i = 1; i < meshes_.size(); i++) {
        auto& mesh = meshes_[i];
        auto positionCount = mesh->vertexAttribute(AttributeType::POSITION).size();
        auto offset = accumulator->vertexAttribute(AttributeType::POSITION).size();

        // Check we're not exceeding 16 bit index limit
        if (indices16bit && offset + positionCount > std::numeric_limits<uint16_t>::max()) {
            accumulator = &*result.emplace_back(mesh);
            continue;
        }

        // Copy all vertex attributes
        for (const auto& vaIn : mesh->vertexData()) {
            auto& va = accumulator->vertexAttribute(vaIn.first);
            va.append(vaIn.second);
        }

        // Update indices
        if (indices16bit) {
            accumulator->indices().append(TypedData::From(1, meshtools::transform<uint16_t>(mesh->indices<uint16_t>(), [offset](auto& idx) {
                                                              return idx + offset;
                                                          })));
        } else {
            accumulator->indices().append(TypedData::From(1, meshtools::transform<uint32_t>(mesh->indices<uint32_t>(), [offset](auto& idx) {
                                                              return idx + offset;
                                                          })));
        }
    }

#ifndef NDEBUG
    for (auto& va : result[0]->vertexData()) {
        assert(va.second.size() == totalVertices);
    }
    assert(result[0]->indices().size() == totalIndices);
#endif

    meshes_ = result;
}

} // namespace meshtools::models
