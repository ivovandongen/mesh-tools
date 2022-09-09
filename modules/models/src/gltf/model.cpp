#include "model.hpp"

#include <meshtools/algorithm.hpp>
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
#include <vector>

namespace {

std::string text(tinygltf::Model& model) {
    std::stringstream os;
    tinygltf::TinyGLTF tiny;
    tiny.SetSerializeDefaultValues(false);
    tiny.WriteGltfSceneToStream(&model, os, true, false);
    return os.str();
}

std::vector<char> binary(tinygltf::Model& model) {
    std::stringstream os;
    tinygltf::TinyGLTF tiny;
    tiny.SetSerializeDefaultValues(false);
    tiny.WriteGltfSceneToStream(&model, os, false, true);
    auto str = os.str();
    return std::vector<char>{str.begin(), str.end()};
}

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

template<size_t N, class T, class Fn>
std::vector<unsigned char> pack(T input, Fn&& fn) {
    std::vector<unsigned char> result;
    result.reserve(N * input.size());
    for (auto& i : input) {
        unsigned char* converted = fn(i);
        result.insert(result.end(), &converted[0], &converted[N]);
    }
    return result;
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

std::array<glm::vec3, 2> minmax(const std::vector<glm::vec3>& positions) {
    constexpr const static auto max = std::numeric_limits<float>::max();
    constexpr const static auto min = -std::numeric_limits<float>::max();
    return std::accumulate(positions.begin(),
                           positions.end(),
                           std::array<glm::vec3, 2>{glm::vec3{max, max, max}, glm::vec3{min, min, min}},
                           [](std::array<glm::vec3, 2>& acc, const glm::vec3& val) {
                               acc[0][0] = std::min(acc[0][0], val[0]);
                               acc[0][1] = std::min(acc[0][1], val[1]);
                               acc[0][2] = std::min(acc[0][2], val[2]);

                               acc[1].x = std::max(acc[1][0], val[0]);
                               acc[1].y = std::max(acc[1][1], val[1]);
                               acc[1].z = std::max(acc[1][2], val[2]);

                               return acc;
                           });
}

struct BufferRange {
    size_t start;
    size_t end;
    size_t endPadded;

    size_t length() const {
        return end - start;
    }

    size_t lengthPadded() const {
        return endPadded - start;
    }
};

BufferRange appendToBuffer(tinygltf::Buffer& buffer, const std::vector<unsigned char>& data) {
    auto startIdx = buffer.data.size();
    buffer.data.insert(buffer.data.end(), data.begin(), data.end());

    // Pad if needed
    if (auto padding = buffer.data.size() % 4) {
        buffer.data.resize(buffer.data.size() + padding, 0);
    }

    return {startIdx, startIdx + data.size(), buffer.data.size()};
}

size_t addBufferView(tinygltf::Model& model, const tinygltf::Buffer& buffer, const BufferRange& bufferRange, int target = 0) {
    auto& bufferView = model.bufferViews.emplace_back();
    bufferView.buffer = std::find(model.buffers.begin(), model.buffers.end(), buffer) - model.buffers.begin();
    bufferView.byteOffset = bufferRange.start;
    bufferView.byteLength = bufferRange.length();
    bufferView.target = target;
    return model.bufferViews.size() - 1;
}

} // namespace

namespace meshtools::models::gltf {

void parseNodes(const tinygltf::Model& gltfModel, Model& model) {
    auto convertNode = [&](const tinygltf::Model&, const tinygltf::Node& in, Node& out) {
        if (in.mesh > -1) {
            // Use the original mesh indices to assign all the mesh indices for this node
            out.meshes(find_indices(model.meshes(), [&](const auto& mesh) { return mesh.originalMeshIndex() == in.mesh; }));
        }

        if (in.matrix.size() == 16) {
            out.transform(glm::make_mat4x4(in.matrix.data()));
        } else {

            // Translation
            auto translation = in.translation.size() == 3
                                       ? glm::translate(glm::mat4(1), glm::vec3(in.translation[0], in.translation[1], in.translation[2]))
                                       : glm::mat4(1);

            // Rotation (glm v gltf -> wxyz / xyzw
            auto rotation = in.rotation.size() == 4
                                    ? glm::mat4_cast(glm::quat(in.rotation[3], in.rotation[0], in.rotation[1], in.rotation[2]))
                                    : glm::mat4(1);

            // Scale
            auto scale = in.scale.size() == 3 ? glm::scale(glm::mat4(1), glm::vec3(in.scale[0], in.scale[1], in.scale[2])) : glm::mat4(1);

            out.transform(translation * rotation * scale);
        }
    };

    std::function<void(const tinygltf::Model&, const tinygltf::Node&, Node*)> processNode =
            [&](const tinygltf::Model& gltfModel, const tinygltf::Node& gltfNode, Node* parent) {
                auto& converted = parent == nullptr ? model.nodes().emplace_back() : parent->children().emplace_back();
                convertNode(gltfModel, gltfNode, converted);

                for (const auto& child : gltfNode.children) {
                    processNode(gltfModel, gltfModel.nodes[child], &converted);
                }
            };

    assert(!gltfModel.scenes.empty());
    for (auto nodeIdx : gltfModel.scenes[0].nodes) {
        processNode(gltfModel, gltfModel.nodes[nodeIdx], nullptr);
    }
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

    return {gltfMesh.name,
            static_cast<size_t>(std::find(gltfModel.meshes.begin(), gltfModel.meshes.end(), gltfMesh) - gltfModel.meshes.begin()),
            std::move(indices),
            std::move(positions),
            std::move(normals),
            std::move(uvs)};
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
    if (model.meshes().empty()) {
        logging::warn("No meshes to dump into {}", file.c_str());
        return;
    }

    tinygltf::Model gltfModel;
    // Define the asset. The version is required
    gltfModel.asset.version = "2.0";
    gltfModel.asset.generator = "tinygltf";

    // Default scene
    auto& scene = gltfModel.scenes.emplace_back();

    // Add the buffer to the model
    auto& buffer = gltfModel.buffers.emplace_back();

    // Add AO image/texture/sampler
    size_t aoTextureIndex = [&]() {
        // Add the image to the buffer
        auto imageBufferRange = appendToBuffer(buffer, aoMap.png());

        // Add a BufferView for the image data
        auto aoImageBufferViewIndex = addBufferView(gltfModel, buffer, imageBufferRange);

        // Add an image
        auto aoImageIndex = gltfModel.images.size();
        auto& aoImage = gltfModel.images.emplace_back();
        aoImage.bufferView = aoImageBufferViewIndex;
        aoImage.mimeType = "image/png";

        // Add the sampler
        auto aoSamplerIndex = gltfModel.samplers.size();
        auto& aoSampler = gltfModel.samplers.emplace_back();
        aoSampler.name = "AO";
        aoSampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
        aoSampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;

        // Add the texture
        auto& aoTexture = gltfModel.textures.emplace_back();
        aoTexture.source = aoImageIndex;
        aoTexture.sampler = aoSamplerIndex;
        return gltfModel.textures.size() - 1;
    }();


    for (auto& mesh : model.meshes()) {
        // Add a node and mesh
        gltfModel.meshes.emplace_back();
        auto& node = gltfModel.nodes.emplace_back();
        node.mesh = gltfModel.meshes.size() - 1;
        scene.nodes.push_back(gltfModel.nodes.size() - 1);

        // Add indices to buffer
        auto indicesBufferRange = appendToBuffer(buffer, pack<sizeof(uint32_t)>(mesh.indices(), [](auto& index) {
                                                     return static_cast<unsigned char*>(static_cast<void*>(&index));
                                                 }));

        // Add a BufferView for the indices
        auto indicesBufferViewIndex = addBufferView(gltfModel, buffer, indicesBufferRange, TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

        // Add an accessor for the indices
        auto indexAccessorIdx = gltfModel.accessors.size();
        auto& indexAccessor = gltfModel.accessors.emplace_back();
        indexAccessor.bufferView = indicesBufferViewIndex;
        indexAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
        indexAccessor.count = mesh.indices().size();
        indexAccessor.type = TINYGLTF_TYPE_SCALAR;
        indexAccessor.maxValues.push_back(*std::max_element(mesh.indices().begin(), mesh.indices().end()));
        indexAccessor.minValues.push_back(*std::min_element(mesh.indices().begin(), mesh.indices().end()));

        // Add a primitive and a mesh
        auto& primitive = gltfModel.meshes.back().primitives.emplace_back();
        primitive.indices = indexAccessorIdx;
        primitive.mode = TINYGLTF_MODE_TRIANGLES;

        // Add a material
        primitive.material = gltfModel.materials.size();
        auto& material = gltfModel.materials.emplace_back();
        material.name = mesh.name();
        material.occlusionTexture.index = aoTextureIndex;

        // Add the position data
        {
            auto positionBufferRange = appendToBuffer(buffer, pack<sizeof(glm::vec3)>(mesh.positions(), [](auto& pos) {
                                                          return static_cast<unsigned char*>(static_cast<void*>(&pos));
                                                      }));

            // Add a BufferView for the positions
            auto positionBufferViewIndex = addBufferView(gltfModel, buffer, positionBufferRange, TINYGLTF_TARGET_ARRAY_BUFFER);

            // Add an accessor for the positions
            primitive.attributes["POSITION"] = gltfModel.accessors.size();
            auto& positionAccessor = gltfModel.accessors.emplace_back();
            positionAccessor.bufferView = positionBufferViewIndex;
            positionAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            positionAccessor.count = mesh.positions().size();
            positionAccessor.type = TINYGLTF_TYPE_VEC3;
            auto minMax = minmax(mesh.positions());
            positionAccessor.minValues = std::vector<double>{minMax[0][0], minMax[0][1], minMax[0][2]};
            positionAccessor.maxValues = std::vector<double>{minMax[1][0], minMax[1][1], minMax[1][2]};
        }

        // Add the uv data and albedo and baseColor texture
        if (!mesh.texcoords().empty()) {
            auto uvBufferRange = appendToBuffer(buffer, pack<sizeof(glm::vec2)>(mesh.texcoords(), [](auto& uv) {
                                                    return static_cast<unsigned char*>(static_cast<void*>(&uv));
                                                }));

            // Add a BufferView for the uvs
            auto uvBufferViewIndex = addBufferView(gltfModel, buffer, uvBufferRange);

            // Add an accessor for the uvs
            primitive.attributes["TEXCOORD_0"] = gltfModel.accessors.size();
            auto& uvAccessor = gltfModel.accessors.emplace_back();
            uvAccessor.bufferView = uvBufferViewIndex;
            uvAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            uvAccessor.count = mesh.positions().size();
            uvAccessor.type = TINYGLTF_TYPE_VEC2;
        }
    }

    // Write out
    if (string::endsWith(file.string(), ".gltf")) {
        std::ofstream out(file);
        out << text(gltfModel);
        out.close();
    } else {
        auto bin = binary(gltfModel);
        std::ofstream output(file, std::ios::binary);
        std::copy(bin.begin(), bin.end(), std::ostreambuf_iterator<char>(output));
        output.close();
    }
}

} // namespace meshtools::models::gltf