#pragma once

#include <meshtools/image.hpp>
#include <meshtools/result.hpp>
#include <meshtools/vec2.hpp>
#include <meshtools/vec3.hpp>

#include <filesystem>
#include <memory>
#include <vector>

namespace meshtools::models {

class Mesh {
public:
    Mesh(std::string name, std::vector<uint32_t> indices, std::vector<vec3> positions, std::vector<vec3> normals,
         std::vector<vec2> texcoords)
        : name_(std::move(name)), indices_(std::move(indices)), positions_(std::move(positions)), normals_(std::move(normals)),
          texcoords_(std::move(texcoords)) {}

    const std::vector<uint32_t>& indices() const {
        return indices_;
    };

    const std::vector<vec3>& positions() const {
        return positions_;
    };

    const std::vector<vec3>& normals() const {
        return normals_;
    }

    const std::vector<vec2>& texcoords() const {
        return texcoords_;
    }

    void texcoords(std::vector<vec2> uvs) {
        texcoords_ = std::move(uvs);
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
    std::vector<uint32_t> indices_;
    std::vector<vec3> positions_;
    std::vector<vec3> normals_;
    std::vector<vec2> texcoords_;
};

using ModelLoadResult = Result<class Model>;

class Model {
public:
    static ModelLoadResult Load(const std::filesystem::path& path);

    explicit Model(std::vector<Mesh> meshes) : meshes_(std::move(meshes)) {}

    std::vector<Mesh>& meshes() {
        return meshes_;
    }

    const std::vector<Mesh>& meshes() const {
        return meshes_;
    }

    void dump(const Image& aoMap, const std::filesystem::path& file) const;

private:
    std::vector<Mesh> meshes_;
};

} // namespace meshtools::models