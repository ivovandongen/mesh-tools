#include "model.hpp"

#include <meshtools/logging.hpp>
#include <meshtools/string.hpp>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14
//TODO #define TINYGLTF_ENABLE_DRACO
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tiny_gltf.h>

#include <algorithm>

namespace {

// Stub
bool loadImageDataFunction(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int,
                           void* user_pointer) {
    return true;
}

// Stub
bool writeImageDataFunction(const std::string*, const std::string*, tinygltf::Image*, bool, void*) {
    return true;
}

constexpr size_t componentByteSize(const tinygltf::Accessor& accessor) {
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return sizeof(char);
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return sizeof(short);
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return sizeof(int);
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return sizeof(float);
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return sizeof(double);
        default:
            assert(false);
            return 0;
    }
}

constexpr size_t componentCount(const tinygltf::Accessor& accessor) {
    switch (accessor.type) {
        case TINYGLTF_TYPE_SCALAR:
            return 1;
        case TINYGLTF_TYPE_VEC2:
            return 2;
        case TINYGLTF_TYPE_VEC3:
            return 3;
        case TINYGLTF_TYPE_VEC4:
        case TINYGLTF_TYPE_MAT2:
            return 4;
        case TINYGLTF_TYPE_MAT3:
            return 9;
        case TINYGLTF_TYPE_MAT4:
            return 16;
        default:
            assert(false);
            return 0;
    }
}

} // namespace

namespace meshtools::models::gltf {

void parseNodes(const tinygltf::Model& gltfModel, Model& model) {
    // TODO
    //    gltfModel.scenes[0].nodes;
}

template<class T>
T copy(const unsigned char* data, size_t componentByteSize) {
    assert(componentByteSize <= sizeof(T));
    T result{};
    memcpy(&result, data, componentByteSize);
    return result;
}


template<class T>
T convert(const unsigned char* data, size_t componentByteSize);

template<>
glm::vec2 convert<glm::vec2>(const unsigned char* data, size_t componentByteSize) {
    glm::vec2 result{};
    result[0] = copy<float>(data, componentByteSize);
    result[1] = copy<float>(data + componentByteSize, componentByteSize);
    return result;
}

template<>
glm::vec3 convert<glm::vec3>(const unsigned char* data, size_t componentByteSize) {
    glm::vec3 result{};
    result[0] = copy<float>(data, componentByteSize);
    result[1] = copy<float>(data + componentByteSize, componentByteSize);
    result[2] = copy<float>(data + componentByteSize * 2, componentByteSize);
    return result;
}

template<>
uint32_t convert<uint32_t>(const unsigned char* data, size_t componentByteSize) {
    assert(componentByteSize < sizeof(uint32_t));
    return copy<uint32_t>(data, componentByteSize);
}

template<class T>
std::vector<T> parseAccessor(const tinygltf::Model& gltfModel, int accessor) {
    const auto& gltfAccessor = gltfModel.accessors[accessor];
    const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
    const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];

    std::vector<T> result;

    const auto compByteSize = componentByteSize(gltfAccessor);
    const auto compCnt = componentCount(gltfAccessor);
    const auto attributeSize = compByteSize * compCnt;

    if ((gltfBufferView.byteStride == 0 || gltfBufferView.byteStride == attributeSize) && attributeSize == sizeof(T)) {
        // De-interlaced buffer with the same component size, straight up copy
        result.resize(gltfAccessor.count);
        auto bufferSize = attributeSize * gltfAccessor.count;
        std::memcpy(result.data(), gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, bufferSize);
    } else {
        // Need to parse the buffer
        result.reserve(gltfAccessor.count);

        // Start of the buffer
        const auto start = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
        const auto stride = gltfBufferView.byteStride > 0 ? gltfBufferView.byteStride : attributeSize;
        for (size_t i = 0; i < gltfAccessor.count; i++) {
            result.push_back(convert<T>(start + i * stride, compByteSize));
        }
    }

    return result;
}

template<class T, class Fn>
std::vector<T> parseAttributeOr(const tinygltf::Model& gltfModel, const tinygltf::Primitive& gltfPrimitive, const std::string& attribute,
                                Fn&& orFn) {
    auto it = gltfPrimitive.attributes.find(attribute);
    if (it == gltfPrimitive.attributes.end()) {
        return orFn();
    }

    return parseAccessor<T>(gltfModel, it->second);
};

Mesh parsePrimitive(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh, const tinygltf::Primitive& gltfPrimitive) {
    // Parse vertex attributes and indices
    auto positions = parseAccessor<glm::vec3>(gltfModel, gltfPrimitive.attributes.at("POSITION"));
    auto normals = parseAttributeOr<glm::vec3>(gltfModel, gltfPrimitive, "NORMAL", [] { return std::vector<glm::vec3>{}; });
    auto uvs = parseAttributeOr<glm::vec2>(gltfModel, gltfPrimitive, "TEXCOORD_0", [] { return std::vector<glm::vec2>{}; });
    auto indices = [&]() {
        if (gltfPrimitive.indices >= 0) {
            return parseAccessor<uint32_t>(gltfModel, gltfPrimitive.indices);
        } else {
            // Generate indices
            logging::warn("No indices in primitive, generating");
            assert(positions.size() % 3 == 0);
            std::vector<uint32_t> ib;
            ib.reserve(positions.size());
            for (uint32_t i = 0; i < positions.size(); i++) {
                ib.push_back(i);
            }
            return ib;
        }
    }();
    return {gltfMesh.name, std::move(indices), std::move(positions), std::move(normals), std::move(uvs)};
}

std::vector<Mesh> parseMesh(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh) {

    std::vector<Mesh> meshes;
    meshes.reserve(gltfMesh.primitives.size());
    std::transform(gltfMesh.primitives.begin(),
                   gltfMesh.primitives.end(),
                   std::back_inserter(meshes),
                   [&](const tinygltf::Primitive& gltfPrimitive) { return parsePrimitive(gltfModel, gltfMesh, gltfPrimitive); });

    return meshes;
}

void parseMeshes(const tinygltf::Model& gltfModel, Model& model) {
    model.meshes().clear();
    model.meshes().reserve(
            std::reduce(gltfModel.meshes.begin(), gltfModel.meshes.end(), size_t{0}, [](size_t sum, const tinygltf::Mesh& mesh) {
                return sum + mesh.primitives.size();
            }));

    for (const auto& gltfMesh : gltfModel.meshes) {
        auto meshes = parseMesh(gltfModel, gltfMesh);
        for (auto& mesh : meshes) {
            model.meshes().push_back(std::move(mesh));
        }
    }
}

ModelLoadResult LoadModel(const std::filesystem::path& file) {

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    loader.SetImageLoader(&loadImageDataFunction, nullptr);
    loader.SetImageWriter(&writeImageDataFunction, nullptr);

    std::string err;
    std::string warn;
    bool result;
    if (string::endsWith(file, ".gltf")) {
        result = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, file);
    } else if (string::endsWith(file, ".glb")) {
        result = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, file);
    } else {
        logging::error("Unknown file format: {}", file.c_str());
        result = false;
    }

    if (!warn.empty()) {
        logging::warn("Warn: {}", warn.c_str());
    }

    if (!result || !err.empty()) {
        logging::error("Err: {}", warn.c_str());
        return {err};
    }

    auto model = std::make_shared<Model>();

    parseMeshes(gltfModel, *model);
    parseNodes(gltfModel, *model);
    // TODO
    // - Images
    // - Textures
    // - Materials

    return {model};
}

void dump(const Model& model, const Image& aoMap, const std::filesystem::path& file) {
    logging::warn("Model dump for glTF not implemented");
    assert(false);
}

} // namespace meshtools::models::gltf