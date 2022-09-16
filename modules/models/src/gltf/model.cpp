#include "model.hpp"

#include <meshtools/algorithm.hpp>
#include <meshtools/logging.hpp>
#include <meshtools/string.hpp>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14
#define TINYGLTF_ENABLE_DRACO

#include <tiny_gltf.h>

#include <algorithm>
#include <set>
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

constexpr int componentType(const meshtools::models::DataType& dataType) {
    switch (dataType) {
        case meshtools::models::DataType::BYTE:
            return TINYGLTF_COMPONENT_TYPE_BYTE;
        case meshtools::models::DataType::U_BYTE:
            return TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        case meshtools::models::DataType::SHORT:
            return TINYGLTF_COMPONENT_TYPE_SHORT;
        case meshtools::models::DataType::U_SHORT:
            return TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
        case meshtools::models::DataType::INT:
            return TINYGLTF_COMPONENT_TYPE_INT;
        case meshtools::models::DataType::U_INT:
            return TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
        case meshtools::models::DataType::FLOAT:
            return TINYGLTF_COMPONENT_TYPE_FLOAT;
        case meshtools::models::DataType::DOUBLE:
            return TINYGLTF_COMPONENT_TYPE_DOUBLE;
        default:
            assert(false);
            return -1;
    }
}

constexpr meshtools::models::DataType dataType(const tinygltf::Accessor& accessor) {
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
            return meshtools::models::DataType::BYTE;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return meshtools::models::DataType::U_BYTE;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            return meshtools::models::DataType::SHORT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return meshtools::models::DataType::U_SHORT;
        case TINYGLTF_COMPONENT_TYPE_INT:
            return meshtools::models::DataType::INT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return meshtools::models::DataType::U_INT;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return meshtools::models::DataType::FLOAT;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return meshtools::models::DataType::DOUBLE;
        default:
            assert(false);
            return meshtools::models::DataType::UNKNOWN;
    }
}

constexpr meshtools::models::AttributeType attributeType(const std::string& input) {
    if (input == "POSITION") {
        return meshtools::models::AttributeType::POSITION;
    }

    if (input == "NORMAL") {
        return meshtools::models::AttributeType::NORMAL;
    }

    if (input == "TEXCOORD_0") {
        return meshtools::models::AttributeType::TEXCOORD;
    }

    if (input == "COLOR_0") {
        return meshtools::models::AttributeType::COLOR;
    }

    meshtools::logging::warn("Unknown vertex attribute type {}", input.c_str());
    return meshtools::models::AttributeType::UNKNOWN;
}

std::string attributeType(const meshtools::models::AttributeType& input) {
    switch (input) {
        case meshtools::models::AttributeType::POSITION:
            return "POSITION";
        case meshtools::models::AttributeType::NORMAL:
            return "NORMAL";
        case meshtools::models::AttributeType::TEXCOORD:
            return "TEXCOORD_0";
        case meshtools::models::AttributeType::COLOR:
            return "COLOR_0";
        case meshtools::models::AttributeType::UNKNOWN:
            return "UNKNOWN";
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

constexpr int typeFromComponentCount(const size_t componentCount) {
    switch (componentCount) {
        case 1:
            return TINYGLTF_TYPE_SCALAR;
        case 2:
            return TINYGLTF_TYPE_VEC2;
        case 3:
            return TINYGLTF_TYPE_VEC3;
        case 4:
            return TINYGLTF_TYPE_VEC4;
            // TODO...       case TINYGLTF_TYPE_MAT2:
        case 9:
            return TINYGLTF_TYPE_MAT3;
        case 16:
            return TINYGLTF_TYPE_MAT4;
        default:
            assert(false);
            return -1;
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
std::array<glm::vec3, 2> minmax(const T& positions) {
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

TypedData parseAccessor(const tinygltf::Model& gltfModel, int accessor) {
    const auto& gltfAccessor = gltfModel.accessors[accessor];
    const auto& gltfBufferView = gltfModel.bufferViews[gltfAccessor.bufferView];
    const auto& gltfBuffer = gltfModel.buffers[gltfBufferView.buffer];


    const auto type = dataType(gltfAccessor);
    const auto compByteSize = bytes(type);
    const auto compCnt = componentCount(gltfAccessor);
    const auto attributeSize = compByteSize * compCnt;

    std::vector<unsigned char> result;

    if ((gltfBufferView.byteStride == 0 || gltfBufferView.byteStride == attributeSize)) {
        // De-interlaced buffer, straight up copy
        result.resize(gltfBufferView.byteLength);
        std::memcpy(result.data(), gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfBufferView.byteLength);
    } else {
        // Need to parse the buffer
        result.reserve(gltfBufferView.byteLength);

        // Start of the buffer
        const auto start = gltfBuffer.data.data() + gltfBufferView.byteOffset + gltfAccessor.byteOffset;
        const auto stride = gltfBufferView.byteStride > 0 ? gltfBufferView.byteStride : attributeSize;
        for (size_t i = 0; i < gltfAccessor.count; i++) {
            std::copy_n(start + i * stride, compByteSize, std::back_inserter(result));
        }
    }

    return TypedData{
            type,
            compCnt,
            std::move(result),
    };
}

template<class Fn>
TypedData parseAttributeOr(const tinygltf::Model& gltfModel, const tinygltf::Primitive& gltfPrimitive, const std::string& attribute,
                           Fn&& orFn) {
    auto it = gltfPrimitive.attributes.find(attribute);
    if (it == gltfPrimitive.attributes.end()) {
        return orFn();
    }

    return parseAccessor(gltfModel, it->second);
};

Mesh parsePrimitive(const tinygltf::Model& gltfModel, const tinygltf::Mesh& gltfMesh, const tinygltf::Primitive& gltfPrimitive) {
    // Parse vertex attributes
    VertexData vertexData;
    for (const auto& attribute : gltfPrimitive.attributes) {
        vertexData.emplace(attributeType(attribute.first), parseAccessor(gltfModel, attribute.second));
    }

    // Parse indices
    auto indices = [&]() {
        if (gltfPrimitive.indices >= 0) {
            return parseAccessor(gltfModel, gltfPrimitive.indices);
        } else {
            // Generate indices
            logging::warn("No indices in primitive, generating");
            assert(vertexData.find(AttributeType::POSITION) != vertexData.end());
            assert(vertexData[AttributeType::POSITION].size() % 3 == 0);
            auto positionCount = vertexData[AttributeType::POSITION].size();
            std::vector<uint32_t> ib;
            ib.reserve(positionCount);
            for (uint32_t i = 0; i < positionCount; i++) {
                ib.push_back(i);
            }
            return TypedData{DataType::U_INT, 1, std::vector<unsigned char>{}};
        }
    }();

    return {
            gltfMesh.name,
            static_cast<size_t>(std::find(gltfModel.meshes.begin(), gltfModel.meshes.end(), gltfMesh) - gltfModel.meshes.begin()),
            gltfPrimitive.material,
            std::move(indices),
            std::move(vertexData),
    };
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

void parseImages(const tinygltf::Model& gltfModel, Model& model) {
    model.images() = transform<std::shared_ptr<Image>>(gltfModel.images, [](const tinygltf::Image& image) {
        auto img = std::make_shared<Image>(static_cast<uint32_t>(image.height),
                                           static_cast<uint32_t>(image.height),
                                           static_cast<uint8_t>(image.component));

        img->data() = image.image;
        img->name() = image.name;
        return img;
    });
}

void parseSamplers(const tinygltf::Model& gltfModel, Model& model) {
    model.samplers() = transform<Sampler>(gltfModel.samplers, [](const tinygltf::Sampler& sampler) {
        return Sampler{
                sampler.minFilter,
                sampler.magFilter,
                sampler.wrapS,
                sampler.wrapT,
        };
    });
}

void parseTextures(const tinygltf::Model& gltfModel, Model& model) {
    model.textures() = transform<Texture>(gltfModel.textures, [](const tinygltf::Texture& texture) {
        return Texture{
                texture.sampler,
                texture.source,
        };
    });
}

void parseMaterials(const tinygltf::Model& gltfModel, Model& model) {
    model.materials() = transform<Material>(gltfModel.materials, [](const tinygltf::Material& material) {
        Material out{};

        out.pbrMetallicRoughness.baseColorTexture = material.pbrMetallicRoughness.baseColorTexture.index;
        out.pbrMetallicRoughness.baseColorFactor = glm::vec4{material.pbrMetallicRoughness.baseColorFactor[0],
                                                             material.pbrMetallicRoughness.baseColorFactor[1],
                                                             material.pbrMetallicRoughness.baseColorFactor[2],
                                                             material.pbrMetallicRoughness.baseColorFactor[3]};
        out.pbrMetallicRoughness.roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        out.pbrMetallicRoughness.metallicFactor = material.pbrMetallicRoughness.metallicFactor;
        out.occlusionTexture = material.occlusionTexture.index;
        out.doubleSided = material.doubleSided;
        out.alphaCutoff = material.alphaCutoff;
        out.alphaMode = material.alphaMode;

        return out;
    });
}

ModelLoadResult LoadModel(const std::filesystem::path& file) {

    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF loader;
    //    loader.SetImageLoader(&loadImageDataFunction, nullptr);
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

    parseImages(gltfModel, *model);
    parseSamplers(gltfModel, *model);
    parseTextures(gltfModel, *model);
    parseMaterials(gltfModel, *model);
    parseMeshes(gltfModel, *model);
    parseNodes(gltfModel, *model);

    return {std::move(model)};
}


void write(tinygltf::Model& gltfModel, const std::filesystem::path& file) {
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

void write(const Model& model, const std::filesystem::path& outFile) {
    tinygltf::Model gltfModel;
    // Define the asset. The version is required
    gltfModel.asset.version = "2.0";
    gltfModel.asset.generator = "tinygltf";

    // Default scene
    auto& gltfScene = gltfModel.scenes.emplace_back();
    gltfScene.name = "Default Scene";

    // Add the buffer to the model
    auto& buffer = gltfModel.buffers.emplace_back();

    // Write meshes
    std::set<size_t> meshIndices;
    std::transform(model.meshes().begin(), model.meshes().end(), std::inserter(meshIndices, meshIndices.end()), [](const Mesh& mesh) {
        return mesh.originalMeshIndex();
    });
    gltfModel.meshes.resize(meshIndices.size());

    for (auto& mesh : model.meshes()) {
        auto& gltfMesh = gltfModel.meshes[mesh.originalMeshIndex()];

        // Add indices to buffer
        auto& indexView = mesh.indices();
        auto indicesBufferRange = appendToBuffer(buffer, indexView.buffer());

        // Add a BufferView for the indices
        auto indicesBufferViewIndex = addBufferView(gltfModel, buffer, indicesBufferRange, TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

        // Add an accessor for the indices
        auto indexAccessorIdx = gltfModel.accessors.size();
        auto& indexAccessor = gltfModel.accessors.emplace_back();
        indexAccessor.bufferView = indicesBufferViewIndex;
        indexAccessor.componentType = componentType(indexView.dataType());
        indexAccessor.count = indexView.size();
        indexAccessor.type = TINYGLTF_TYPE_SCALAR;
        // Min max
        auto indices = mesh.indices<uint32_t>();
        auto indexMinMax = std::minmax_element(indices.begin(), indices.end());
        indexAccessor.maxValues.push_back(*indexMinMax.second);
        indexAccessor.minValues.push_back(*indexMinMax.first);

        // Add a primitive and a mesh
        auto& gltfPrimitive = gltfMesh.primitives.emplace_back();
        gltfPrimitive.indices = indexAccessorIdx;
        gltfPrimitive.mode = TINYGLTF_MODE_TRIANGLES;

        // Add the vertex data
        for (auto& va : mesh.vertexData()) {
            auto attribute = attributeType(va.first);
            auto& typedData = va.second;
            auto bufferRange = appendToBuffer(buffer, typedData.buffer());

            // Add a buffer view
            auto bufferViewIndex = addBufferView(gltfModel, buffer, bufferRange, TINYGLTF_TARGET_ARRAY_BUFFER);

            // Add an accessor
            gltfPrimitive.attributes[attribute] = gltfModel.accessors.size();
            auto& gltfAccessor = gltfModel.accessors.emplace_back();
            gltfAccessor.bufferView = bufferViewIndex;
            gltfAccessor.componentType = componentType(typedData.dataType());
            gltfAccessor.count = typedData.count();
            gltfAccessor.type = typeFromComponentCount(typedData.componentCount());
            //TODO min max
            if (va.first == AttributeType::POSITION) {
                auto minMax = minmax(mesh.vertexAttribute<glm::vec3>(AttributeType::POSITION).vector());
                gltfAccessor.minValues = std::vector<double>{minMax[0][0], minMax[0][1], minMax[0][2]};
                gltfAccessor.maxValues = std::vector<double>{minMax[1][0], minMax[1][1], minMax[1][2]};
            }
        }

        // Material
        if (mesh.materialIdx() >= 0) {
            gltfPrimitive.material = mesh.materialIdx();
        }
    }


    // TODO: Write nodes
    gltfScene.nodes.resize(model.nodes().size());
    std::generate(gltfScene.nodes.begin(), gltfScene.nodes.end(), [n = 0]() mutable { return n++; });

    for (auto& node : model.nodes()) {
        // TODO: child nodes
        // TODO: transform
        tinygltf::Node gltfNode{};
        gltfNode.mesh = model.meshes()[node.meshes()[0]].originalMeshIndex();
        gltfModel.nodes.push_back(gltfNode);
    }

    if (!model.images().empty()) {

        // Images
        auto& imageBuffer = gltfModel.buffers.emplace_back();
        for (auto& image : model.images()) {
            tinygltf::Image gltfImage{};
            gltfImage.width = image->width();
            gltfImage.height = image->height();
            gltfImage.component = image->channels();
            gltfImage.mimeType = "image/png";

            auto imageBufferRange = appendToBuffer(imageBuffer, image->png());
            auto bufferViewIndex = addBufferView(gltfModel, imageBuffer, imageBufferRange);
            gltfImage.bufferView = bufferViewIndex;
            gltfModel.images.emplace_back(gltfImage);
        }

        // Samplers
        for (auto& sampler : model.samplers()) {
            tinygltf::Sampler gltfSampler{};
            if (sampler.minFilter > 0) {
                gltfSampler.minFilter = sampler.minFilter;
            }
            if (sampler.magFilter > 0) {
                gltfSampler.magFilter = sampler.magFilter;
            }
            if (sampler.wrapS > 0) {
                gltfSampler.wrapS = sampler.wrapS;
            }
            if (sampler.wrapT > 0) {
                gltfSampler.wrapT = sampler.wrapT;
            }
            gltfModel.samplers.emplace_back(gltfSampler);
        }

        // Textures
        for (auto& texture : model.textures()) {
            tinygltf::Texture gltfSampler{};
            gltfSampler.source = texture.source;
            gltfSampler.sampler = texture.sampler;
            gltfModel.textures.emplace_back(gltfSampler);
        }
    }

    // Materials
    if (!model.materials().empty()) {
        gltfModel.materials.reserve(model.materials().size());
        for (auto& material : model.materials()) {
            tinygltf::Material mat{};
            mat.pbrMetallicRoughness.baseColorFactor = {
                    material.pbrMetallicRoughness.baseColorFactor[0],
                    material.pbrMetallicRoughness.baseColorFactor[1],
                    material.pbrMetallicRoughness.baseColorFactor[2],
                    material.pbrMetallicRoughness.baseColorFactor[3],
            };
            mat.pbrMetallicRoughness.baseColorTexture.index = material.pbrMetallicRoughness.baseColorTexture;
            mat.pbrMetallicRoughness.metallicFactor = material.pbrMetallicRoughness.metallicFactor;
            mat.pbrMetallicRoughness.roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
            mat.occlusionTexture.index = material.occlusionTexture;
            mat.alphaMode = material.alphaMode;
            mat.alphaCutoff = material.alphaCutoff;
            mat.doubleSided = material.doubleSided;
            gltfModel.materials.push_back(std::move(mat));
        }
    }


    // Write out to disk
    write(gltfModel, outFile);
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
        auto& indexView = mesh.indices();
        auto indicesBufferRange = appendToBuffer(buffer, indexView.buffer());

        // Add a BufferView for the indices
        auto indicesBufferViewIndex = addBufferView(gltfModel, buffer, indicesBufferRange, TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

        // Add an accessor for the indices
        auto indexAccessorIdx = gltfModel.accessors.size();
        auto& indexAccessor = gltfModel.accessors.emplace_back();
        indexAccessor.bufferView = indicesBufferViewIndex;
        indexAccessor.componentType = componentType(indexView.dataType());
        indexAccessor.count = indexView.size();
        indexAccessor.type = TINYGLTF_TYPE_SCALAR;
        // TODO: Min max
        auto indices = mesh.indices<uint32_t>();
        auto indexMinMax = std::minmax_element(indices.begin(), indices.end());
        indexAccessor.maxValues.push_back(*indexMinMax.second);
        indexAccessor.minValues.push_back(*indexMinMax.first);

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
            auto& positionsTypedData = mesh.vertexAttribute(AttributeType::POSITION);
            auto positionBufferRange = appendToBuffer(buffer, positionsTypedData.buffer());

            // Add a BufferView for the positions
            auto positionBufferViewIndex = addBufferView(gltfModel, buffer, positionBufferRange, TINYGLTF_TARGET_ARRAY_BUFFER);

            // Add an accessor for the positions
            primitive.attributes["POSITION"] = gltfModel.accessors.size();
            auto& positionAccessor = gltfModel.accessors.emplace_back();
            positionAccessor.bufferView = positionBufferViewIndex;
            positionAccessor.componentType = componentType(positionsTypedData.dataType());
            positionAccessor.count = positionsTypedData.size();
            positionAccessor.type = typeFromComponentCount(positionsTypedData.componentCount());
            //TODO min max
            auto minMax = minmax(mesh.vertexAttribute<glm::vec3>(AttributeType::POSITION));
            positionAccessor.minValues = std::vector<double>{minMax[0][0], minMax[0][1], minMax[0][2]};
            positionAccessor.maxValues = std::vector<double>{minMax[1][0], minMax[1][1], minMax[1][2]};
        }

        // Add the uv data and albedo and baseColor texture
        if (mesh.hasVertexAttribute(AttributeType::TEXCOORD)) {
            auto& uvsTypedData = mesh.vertexAttribute(AttributeType::TEXCOORD);
            auto uvBufferRange = appendToBuffer(buffer, uvsTypedData.buffer());

            // Add a BufferView for the uvs
            auto uvBufferViewIndex = addBufferView(gltfModel, buffer, uvBufferRange);

            // Add an accessor for the uvs
            primitive.attributes["TEXCOORD_0"] = gltfModel.accessors.size();
            auto& uvAccessor = gltfModel.accessors.emplace_back();
            uvAccessor.bufferView = uvBufferViewIndex;
            uvAccessor.componentType = componentType(uvsTypedData.dataType());
            uvAccessor.count = uvsTypedData.size();
            uvAccessor.type = TINYGLTF_TYPE_VEC2;
        }
    }

    write(gltfModel, file);
}

} // namespace meshtools::models::gltf