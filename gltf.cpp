#include "gltf.h"

#include "json/json.hpp"

#include <cassert>
#include <fstream>
#include <functional>

// @brief Used to mark required fields in the GLTF file
// @return If the condition is false, an assertion is triggered and the function returns false
// @note The function must return a boolean value
#define REQUIRE(condition, message)                     \
	if (!(condition))                                   \
	{                                                   \
		assert(false && "Invalid GLTF file: " message); \
		return false;                                   \
	}

namespace Aegix::GLTF
{
	namespace base64
	{
		static constexpr uint8_t INVALID_UINT8 = 255;

		// + 1 for the null terminator
		static constexpr std::array<uint8_t, 64 + 1> encodeTable{
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
		};

		static constexpr std::array<uint8_t, 256> decodeTable()
		{
			std::array<uint8_t, 256> table{};
			table.fill(INVALID_UINT8);
			for (size_t i = 0; i < 64; ++i)
			{
				table[encodeTable[i]] = i;
			}
			return table;
		}

		static std::vector<uint8_t> decode(std::string_view input)
		{
			constexpr auto BITS_IN_B64 = 6;
			constexpr auto BITS_IN_BYTE = 8;
			constexpr auto MASK_BYTE = (1 << BITS_IN_BYTE) - 1;
			constexpr auto table = decodeTable();

			std::vector<uint8_t> output{};
			output.reserve(input.size() / 4 * 3);

			uint32_t value = 0;	// Stores actual bits
			uint32_t count = 0; // Used bits in value
			for (const unsigned char c : input)
			{
				if (table[c] == INVALID_UINT8) // Skip invalid characters
					continue;

				value = (value << BITS_IN_B64) + table[c];
				count += BITS_IN_B64;

				if (count >= BITS_IN_BYTE)
				{
					const auto shift = count - BITS_IN_BYTE;
					output.push_back(static_cast<uint8_t>((value >> shift) & MASK_BYTE));
					count -= BITS_IN_BYTE;
				}
			}

			return output;
		}
	}

	static std::vector<uint8_t> loadUriData(std::string_view uri)
	{
		std::string_view marker = "base64,";
		auto pos = uri.find(marker);
		if (pos == std::string::npos)
		{
			assert(false && "Invalid data URI");
			return {};
		}

		auto data = uri.substr(pos + marker.size());
		return base64::decode(data);
	}

	static std::vector<uint8_t> loadBuffer(const std::filesystem::path& basePath, const std::string& uri)
	{
		if (uri.substr(0, 5) == "data:")
			return loadUriData(uri);

		std::ifstream file(basePath / uri, std::ios::in | std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			assert(false && "Failed to open buffer file");
			return {};
		}

		auto size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		file.read(reinterpret_cast<char*>(buffer.data()), size);
		file.close();

		return buffer;
	}

	/// @brief Reads a value from a JSON object by key and stores it in outValue.
	/// @tparam T Type of the value to read.
	/// @return Returns false if the key was not found otherwise true.
	template<typename T>
	static bool tryRead(const nlohmann::json& json, const char* key, T& outValue)
	{
		auto keyIt = json.find(key);
		if (keyIt == json.end())
			return false;

		outValue = keyIt->get<T>();
		return true;
	}

	/// @brief Reads a value from a JSON object by key and stores it in outValue after casting it to U.
	/// @tparam T Type of the value to read. This type must be convertible to U by static_cast.
	/// @tparam U Type of the output value.
	/// @return Return false if the key was not found otherwise true.
	template<typename T, typename U>
	static bool tryReadType(const nlohmann::json& json, const char* key, U& outValue)
	{
		auto keyIt = json.find(key);
		if (keyIt == json.end())
			return false;

		outValue = static_cast<U>(keyIt->get<T>());
		return true;
	}

	/// @brief Reads a value from a JSON object by key and parses it to outValue using the parser function.
	/// @tparam T Type of the value to read.
	/// @tparam U Type of the output value.
	/// @param parser Function that converts T to U.
	/// @return Return false if the key was not found otherwise true.
	template<typename T, typename U>
	static bool tryReadParse(const nlohmann::json& json, const char* key, U& outValue, std::function<U(const T&)> parser)
	{
		auto keyIt = json.find(key);
		if (keyIt == json.end())
			return false;

		outValue = parser(keyIt->get<T>());
		return true;
	}

	/// @brief Reads a value form a JSON object by key and stores it in outValue if it exists.
	/// @tparam T Type of the value to read.
	/// @return Return false if the key was not found otherwise true.
	template<typename T>
	static bool tryReadOptional(const nlohmann::json& json, const char* key, std::optional<T>& outValue)
	{
		outValue = std::nullopt;
		auto keyIt = json.find(key);
		if (keyIt == json.end())
			return false;

		outValue = keyIt->get<T>();
		return true;
	}

	/// @brief Reads a value form a JSON object by key and stores it in outValue after casting it to U.
	/// @tparam T Type of the value to read. This type must be convertible to U by static_cast.
	/// @tparam U Type of the output value.
	/// @return Return false if the key was not found otherwise true.
	template<typename T, typename U>
	static bool tryReadOptionalType(const nlohmann::json& json, const char* key, std::optional<U>& outValue)
	{
		outValue = std::nullopt;
		auto keyIt = json.find(key);
		if (keyIt == json.end())
			return false;

		auto valueU = static_cast<U>(keyIt->get<T>());
		outValue = valueU;
		return true;
	}

	/// @brief Reads a vector from a JSON object by key and stores it in outValue.
	/// @tparam T Type of the vector elements.
	/// @return Return false if the key was not found otherwise true.
	template<typename T>
	static bool tryReadVector(const nlohmann::json& json, const char* key, std::vector<T>& outValue)
	{
		auto keyIt = json.find(key);
		if (keyIt == json.end() || !keyIt->is_array())
			return false;

		outValue.clear();
		for (const auto& element : *keyIt)
		{
			outValue.emplace_back(element.get<T>());
		}

		return true;
	}

	/// @brief Reads an array from a JSON object by key and stores it in outValue.
	/// @tparam T Type of the array elements.
	/// @tparam Size Size of the array.
	/// @return Return false if the key was not found otherwise true.
	template<typename T, size_t Size>
	static bool tryReadArray(const nlohmann::json& json, const char* key, std::array<T, Size>& outValue)
	{
		auto keyIt = json.find(key);
		if (keyIt == json.end() || !keyIt->is_array() || keyIt->size() != Size)
			return false;

		for (size_t i = 0; i < Size; ++i)
		{
			outValue[i] = (*keyIt)[i].get<T>();
		}

		return true;
	}

	static Accessor::Type parseAccessorType(const std::string& typeString)
	{
		if (typeString == "SCALAR") return Accessor::Type::Scalar;
		if (typeString == "VEC2") return Accessor::Type::Vec2;
		if (typeString == "VEC3") return Accessor::Type::Vec3;
		if (typeString == "VEC4") return Accessor::Type::Vec4;
		if (typeString == "MAT2") return Accessor::Type::Mat2;
		if (typeString == "MAT3") return Accessor::Type::Mat3;
		if (typeString == "MAT4") return Accessor::Type::Mat4;

		assert(false && "Invalid accessor type");
		return Accessor::Type{};
	}

	///////////////////////////////////////////////////////////////////////////////////////////

	static bool readAsset(Asset& asset, const nlohmann::json& json)
	{
		auto assetIt = json.find("asset");
		REQUIRE(assetIt != json.end(), "GLTF asset is required");
		REQUIRE(tryRead(*assetIt, "version", asset.version), "GLTF asset version is required");

		tryReadOptional<std::string>(*assetIt, "generator", asset.generator);
		tryReadOptional<std::string>(*assetIt, "minVersion", asset.minVersion);
		tryReadOptional<std::string>(*assetIt, "copyright", asset.copyright);

		return true;
	}

	static bool readStartScene(std::optional<size_t>& defaultScene, const nlohmann::json& json)
	{
		tryReadOptional(json, "scene", defaultScene);
		return true;
	}

	static bool readScenes(std::vector<Scene>& scenes, const nlohmann::json& json)
	{
		auto scenesIt = json.find("scenes");
		if (scenesIt == json.end() || !scenesIt->is_array()) // Scenes are optional -> return no error
			return true;

		scenes.reserve(scenesIt->size());
		for (const auto& jsonScene : *scenesIt)
		{
			auto& gltfScene = scenes.emplace_back();
			tryReadOptional<std::string>(jsonScene, "name", gltfScene.name);
			tryReadVector<size_t>(jsonScene, "nodes", gltfScene.nodes);
		}

		return true;
	}

	static bool readNodes(std::vector<Node>& nodes, const nlohmann::json& json)
	{
		auto nodesIt = json.find("nodes");
		if (nodesIt == json.end() || !nodesIt->is_array()) // Nodes are optional -> return no error
			return true;

		nodes.reserve(nodesIt->size());
		for (const auto& jsonNode : *nodesIt)
		{
			auto& gltfNode = nodes.emplace_back();

			Mat4 matrix = MAT4_IDENTITY;
			bool matrixFound = tryReadArray<float, 16>(jsonNode, "matrix", matrix);
			Node::TRS trs;
			bool translationFound = tryReadArray<float, 3>(jsonNode, "translation", trs.translation);
			bool rotationFound = tryReadArray<float, 4>(jsonNode, "rotation", trs.rotation);
			bool scaleFound = tryReadArray<float, 3>(jsonNode, "scale", trs.scale);

			REQUIRE(!matrixFound || (!translationFound && !rotationFound && !scaleFound),
				"Node cannot have both matrix and TRS transform")

				if (matrixFound)
				{
					gltfNode.transform = matrix;
				}
				else
				{
					gltfNode.transform = trs;
				}

			tryReadVector<size_t>(jsonNode, "children", gltfNode.children);
			tryReadOptional<size_t>(jsonNode, "camera", gltfNode.camera);
			tryReadOptional<size_t>(jsonNode, "skin", gltfNode.skin);
			tryReadOptional<size_t>(jsonNode, "mesh", gltfNode.mesh);
			tryReadOptional<std::string>(jsonNode, "name", gltfNode.name);
		}

		return true;
	}

	static bool readAttributes(std::unordered_map<std::string, size_t>& attributes, const nlohmann::json& json)
	{
		auto attributesIt = json.find("attributes");
		REQUIRE(attributesIt != json.end(), "Primitive attributes are required");

		attributes.reserve(attributesIt->size());
		for (const auto& [key, value] : attributesIt->items())
		{
			attributes.emplace(key, value.get<size_t>());
		}

		return true;
	}

	static bool readPrimitives(std::vector<Mesh::Primitive>& primitives, const nlohmann::json& json)
	{
		auto primitivesIt = json.find("primitives");
		REQUIRE(primitivesIt != json.end() || !primitivesIt->is_array(), "Primitives are required");

		primitives.reserve(primitivesIt->size());
		for (const auto& jsonPrimitive : *primitivesIt)
		{
			auto& gltfPrimitive = primitives.emplace_back();

			if (!readAttributes(gltfPrimitive.attributes, jsonPrimitive))
				return false;

			tryReadOptional<size_t>(jsonPrimitive, "indices", gltfPrimitive.indices);
			tryReadOptional<size_t>(jsonPrimitive, "material", gltfPrimitive.material);

			auto mode = static_cast<int>(gltfPrimitive.mode); // Default value
			if (tryRead<int>(jsonPrimitive, "mode", mode))
				gltfPrimitive.mode = static_cast<Mesh::Primitive::Mode>(mode);
		}

		return true;
	}

	static bool readMeshes(std::vector<Mesh>& meshes, const nlohmann::json& json)
	{
		auto meshIt = json.find("meshes");
		if (meshIt == json.end() || !meshIt->is_array()) // Meshes are optional -> return no error
			return true;

		meshes.reserve(meshIt->size());
		for (const auto& jsonMesh : *meshIt)
		{
			auto& gltfMesh = meshes.emplace_back();
			tryReadOptional<std::string>(jsonMesh, "name", gltfMesh.name);
			tryReadVector<float>(jsonMesh, "weights", gltfMesh.weights);

			if (!readPrimitives(gltfMesh.primitives, jsonMesh))
				return false;
		}

		return true;
	}

	static bool readAccessors(std::vector<Accessor>& accessors, const nlohmann::json& json)
	{
		auto accessorIt = json.find("accessors");
		if (accessorIt == json.end() || !accessorIt->is_array()) // Accessors are optional -> return no error
			return true;

		accessors.reserve(accessorIt->size());
		for (const auto& jsonAccessor : *accessorIt)
		{
			auto& gltfAccessor = accessors.emplace_back();

			REQUIRE(tryRead(jsonAccessor, "count", gltfAccessor.count),
				"Accessor count is required");
			REQUIRE(tryReadType<int>(jsonAccessor, "componentType", gltfAccessor.componentType),
				"Accessor componentType is required");
			REQUIRE((tryReadParse<std::string, Accessor::Type>(jsonAccessor, "type", gltfAccessor.type, parseAccessorType)),
				"Accessor type is required");

			tryRead<size_t>(jsonAccessor, "bufferView", gltfAccessor.bufferView);
			tryRead(jsonAccessor, "byteOffset", gltfAccessor.byteOffset);
			tryRead(jsonAccessor, "normalized", gltfAccessor.normalized);
			tryReadVector<float>(jsonAccessor, "min", gltfAccessor.min);
			tryReadVector<float>(jsonAccessor, "max", gltfAccessor.max);
			tryReadOptional<std::string>(jsonAccessor, "name", gltfAccessor.name);
		}

		return true;
	}

	static bool readBufferViews(std::vector<BufferView>& bufferViews, const nlohmann::json& json)
	{
		auto bufferViewIt = json.find("bufferViews");
		if (bufferViewIt == json.end() || !bufferViewIt->is_array()) // BufferViews are optional -> return no error
			return true;

		bufferViews.reserve(bufferViewIt->size());
		for (const auto& jsonBufferView : *bufferViewIt)
		{
			auto& gltfBufferView = bufferViews.emplace_back();

			REQUIRE(tryRead(jsonBufferView, "buffer", gltfBufferView.buffer),
				"BufferView buffer is required");
			REQUIRE(tryRead(jsonBufferView, "byteLength", gltfBufferView.byteLength),
				"BufferView byteLength is required");
			tryRead(jsonBufferView, "byteOffset", gltfBufferView.byteOffset);
			tryReadOptional<size_t>(jsonBufferView, "byteStride", gltfBufferView.byteStride);
			tryReadOptionalType<int>(jsonBufferView, "target", gltfBufferView.target);
			tryReadOptional<std::string>(jsonBufferView, "name", gltfBufferView.name);
		}

		return true;
	}

	static bool readBuffers(std::vector<Buffer>& buffers, const nlohmann::json& json)
	{
		auto bufferIt = json.find("buffers");
		if (bufferIt == json.end() || !bufferIt->is_array()) // Buffers are optional -> return no error
			return true;

		buffers.reserve(bufferIt->size());
		for (const auto& jsonBuffer : *bufferIt)
		{
			auto& gltfBuffer = buffers.emplace_back();

			REQUIRE(tryRead(jsonBuffer, "byteLength", gltfBuffer.byteLength),
				"Buffer byteLength is required");
			tryReadOptional<std::string>(jsonBuffer, "uri", gltfBuffer.uri);
			tryReadOptional<std::string>(jsonBuffer, "name", gltfBuffer.name);
		}

		return true;
	}

	static bool readPBR(Material& material, const nlohmann::json& json)
	{
		auto pbrIt = json.find("pbrMetallicRoughness");
		if (pbrIt == json.end()) // PBR is optional -> return no error
			return true;

		Material::PBRMetallicRoughness pbr{};
		tryReadArray<float, 4>(*pbrIt, "baseColorFactor", pbr.baseColorFactor);
		tryRead(*pbrIt, "metallicFactor", pbr.metallicFactor);
		tryRead(*pbrIt, "roughnessFactor", pbr.roughnessFactor);

		auto baseColorTextureIt = pbrIt->find("baseColorTexture");
		if (baseColorTextureIt != pbrIt->end())
		{
			Material::TextureInfo baseColor{};
			REQUIRE(tryRead(*baseColorTextureIt, "index", baseColor.index),
				"Base color texture index is required");
			tryRead(*baseColorTextureIt, "texCoord", baseColor.texCoord);
			pbr.baseColorTexture = baseColor;
		}

		auto metallicRoughnessIt = pbrIt->find("metallicRoughnessTexture");
		if (metallicRoughnessIt != pbrIt->end())
		{
			Material::TextureInfo textureInfo{};
			REQUIRE(tryRead(*metallicRoughnessIt, "index", textureInfo.index),
				"Metallic roughness texture index is required");
			tryRead(*metallicRoughnessIt, "texCoord", textureInfo.texCoord);
			pbr.metallicRoughnessTexture = textureInfo;
		}

		material.pbrMetallicRoughness = pbr;
		return true;
	}

	static bool readNormal(Material& material, const nlohmann::json& json)
	{
		auto normalIt = json.find("normalTexture");
		if (normalIt == json.end()) // Normal texture is optional -> return no error
			return true;

		Material::NormalTextureInfo normal{};

		REQUIRE(tryRead(*normalIt, "index", normal.index),
			"Normal texture index is required");
		tryRead(*normalIt, "texCoord", normal.texCoord);
		tryRead(*normalIt, "scale", normal.scale);

		material.normalTexture = normal;
		return true;
	}

	static bool readOcclusion(Material& material, const nlohmann::json& json)
	{
		auto occlusionIt = json.find("occlusionTexture");
		if (occlusionIt == json.end()) // Occlusion texture is optional -> return no error
			return true;

		Material::OcclusionTextureInfo occlusion{};

		REQUIRE(tryRead(*occlusionIt, "index", occlusion.index),
			"Occlusion texture index is required");
		tryRead(*occlusionIt, "texCoord", occlusion.texCoord);
		tryRead(*occlusionIt, "strength", occlusion.strength);

		material.occlusionTexture = occlusion;
		return true;
	}

	static bool readEmissive(Material& material, const nlohmann::json& json)
	{
		auto emissiveIt = json.find("emissiveTexture");
		if (emissiveIt == json.end()) // Emissive texture is optional -> return no error
			return true;

		Material::TextureInfo emissive{};

		REQUIRE(tryRead(*emissiveIt, "index", emissive.index),
			"Emissive texture index is required");
		tryRead(*emissiveIt, "texCoord", emissive.texCoord);

		material.emissiveTexture = emissive;
		return true;
	}

	static bool readMaterials(std::vector<Material>& materials, const nlohmann::json& json)
	{
		auto materialIt = json.find("materials");
		if (materialIt == json.end() || !materialIt->is_array()) // Materials are optional -> return no error
			return true;

		materials.reserve(materialIt->size());
		for (const auto& jsonMaterial : *materialIt)
		{
			auto& material = materials.emplace_back();

			REQUIRE(readPBR(material, jsonMaterial), "PBR metallic roughness is required");
			REQUIRE(readNormal(material, jsonMaterial), "Normal texture info is required");
			REQUIRE(readOcclusion(material, jsonMaterial), "Occlusion texture info is required");
			REQUIRE(readEmissive(material, jsonMaterial), "Emissive texture info");

			tryReadOptional<std::string>(jsonMaterial, "name", material.name);
			tryReadArray<float, 3>(jsonMaterial, "emissiveFactor", material.emissiveFactor);
			tryReadType<int>(jsonMaterial, "alphaMode", material.alphaMode);
			tryRead(jsonMaterial, "alphaCutoff", material.alphaCutoff);
			tryRead(jsonMaterial, "doubleSided", material.doubleSided);
		}

		return true;
	}

	static bool readTextures(std::vector<Texture>& textures, const nlohmann::json& json)
	{
		auto textureIt = json.find("textures");
		if (textureIt == json.end() || !textureIt->is_array()) // Textures are optional -> return no error
			return true;

		textures.reserve(textureIt->size());
		for (const auto& jsonTexture : *textureIt)
		{
			auto& texture = textures.emplace_back();
			tryReadOptional(jsonTexture, "sampler", texture.sampler);
			tryReadOptional(jsonTexture, "source", texture.source);
			tryReadOptional(jsonTexture, "name", texture.name);
		}

		return true;
	}

	static bool readImages(std::vector<Image>& images, const nlohmann::json& json)
	{
		auto imageIt = json.find("images");
		if (imageIt == json.end() || !imageIt->is_array()) // Images are optional -> return no error
			return true;

		images.reserve(imageIt->size());
		for (const auto& jsonImage : *imageIt)
		{
			auto& image = images.emplace_back();

			Image::UriData uri;
			auto uriFound = tryRead(jsonImage, "uri", uri.uri);

			Image::BufferViewData bufferView;
			auto bufferViewFound = tryRead(jsonImage, "bufferView", bufferView.bufferView);
			auto mimeTypeFound = tryRead(jsonImage, "mimeType", bufferView.mimeType);

			REQUIRE(!uriFound || !bufferViewFound, "Image cannot have both uri and bufferView");
			REQUIRE(uriFound || bufferViewFound, "Image requires uri or bufferView");
			REQUIRE(mimeTypeFound || !bufferViewFound, "Image bufferView mimeType is required when bufferView is defined");

			if (uriFound)
			{
				image.data = uri;
			}
			else
			{
				image.data = bufferView;
			}

			tryReadOptional(jsonImage, "name", image.name);
		}

		return true;
	}

	static bool readSamplers(std::vector<Sampler>& samplers, const nlohmann::json& json)
	{
		auto samplerIt = json.find("samplers");
		if (samplerIt == json.end() || !samplerIt->is_array()) // Samplers are optional -> return no error
			return true;

		samplers.reserve(samplerIt->size());
		for (const auto& jsonSampler : *samplerIt)
		{
			auto& sampler = samplers.emplace_back();
			tryReadOptionalType<int>(jsonSampler, "magFilter", sampler.magFilter);
			tryReadOptionalType<int>(jsonSampler, "minFilter", sampler.minFilter);
			tryReadType<int>(jsonSampler, "wrapS", sampler.wrapS);
			tryReadType<int>(jsonSampler, "wrapT", sampler.wrapT);
			tryReadOptional(jsonSampler, "name", sampler.name);
		}

		return true;
	}

	static std::optional<GLTF> loadGLTF(const nlohmann::json& header, const std::filesystem::path& parentDir)
	{
		GLTF gltf{};

		// Read GLTF header data
		if (!readAsset(gltf.asset, header) ||
			!readStartScene(gltf.startScene, header) ||
			!readScenes(gltf.scenes, header) ||
			!readNodes(gltf.nodes, header) ||
			!readMeshes(gltf.meshes, header) ||
			!readAccessors(gltf.accessors, header) ||
			!readBufferViews(gltf.bufferViews, header) ||
			!readBuffers(gltf.buffers, header) ||
			!readMaterials(gltf.materials, header) ||
			!readTextures(gltf.textures, header) ||
			!readImages(gltf.images, header) ||
			!readSamplers(gltf.samplers, header))
		{
			return std::nullopt;
		}

		return gltf;
	}

	static std::optional<GLTF> readFileGLTF(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::in);
		if (!file.is_open())
			return std::nullopt;

		nlohmann::json jsonData = nlohmann::json::parse(file);
		file.close();

		auto gltf = loadGLTF(jsonData, path.parent_path());

		// Load buffers
		for (auto& buffer : gltf->buffers)
		{
			if (buffer.uri.has_value())
				buffer.data = loadBuffer(path.parent_path(), buffer.uri.value());
		}

		return gltf;
	}

	static std::optional<GLTF> readFileGLB(const std::filesystem::path& path)
	{
		std::ifstream glbFile(path, std::ios::in | std::ios::binary);
		if (!glbFile.is_open())
			return std::nullopt;

		// GLB Files are structured as follows:
		// Header | Chunk 0 Json | Chunk 1 Binary

		HeaderGLB header{};
		glbFile.read(reinterpret_cast<char*>(&header), sizeof(HeaderGLB));
		if (header.magic != GLB_MAGIC || header.version < GLB_VERSION)
		{
			assert(false && "Invalid GLB header, magic or version mismatch");
			return std::nullopt;
		}

		ChunkGLB jsonChunk{};
		glbFile.read(reinterpret_cast<char*>(&jsonChunk), sizeof(ChunkGLB));
		if (jsonChunk.type != GLB_CHUNK_JSON)
		{
			assert(false && "Invalid GLB chunk, JSON chunk expected");
			return std::nullopt;
		}

		std::vector<char> jsonChunkData(jsonChunk.length);
		glbFile.read(jsonChunkData.data(), jsonChunk.length);

		nlohmann::json json = nlohmann::json::parse(jsonChunkData);
		auto gltf = loadGLTF(json, path.parent_path());

		// Load buffers
		for (auto& buffer : gltf->buffers)
		{
			if (!buffer.uri.has_value())
			{
				ChunkGLB binChunk{};
				glbFile.read(reinterpret_cast<char*>(&binChunk), sizeof(ChunkGLB));
				if (binChunk.type != GLB_CHUNK_BIN)
				{
					assert(false && "Invalid GLB chunk, BIN chunk expected");
					return std::nullopt;
				}

				buffer.data.resize(binChunk.length);
				glbFile.read(reinterpret_cast<char*>(buffer.data.data()), binChunk.length);
			}
			else
			{
				buffer.data = loadBuffer(path.parent_path(), buffer.uri.value());
			}
		}

		glbFile.close();
		return gltf;
	}

	std::optional<GLTF> load(const std::filesystem::path& path)
	{
		if (path.extension() == ".gltf")
			return readFileGLTF(path);

		if (path.extension() == ".glb")
			return readFileGLB(path);

		assert(false && "Unsupported file format");
		return std::nullopt;
	}
}
