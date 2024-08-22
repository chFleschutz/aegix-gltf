#include "gltf.h"

#include "json/json.hpp"

#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <type_traits>

namespace Aegix::GLTF
{
	/// @brief Reads a value from a JSON object by key and stores it in outValue.
	/// @tparam T Type of the value to read.
	/// @return Returns false if the key was not found otherwise true.
	template<typename T>
	static bool tryRead(const nlohmann::json& json, const char* key, T& outValue)
	{
		auto it = json.find(key);
		if (it == json.end())
			return false;

		outValue = it->get<T>();
		return true;
	}

	/// @brief Reads a value from a JSON object by key and stores it in outValue after casting it to U.
	/// @tparam T Type of the value to read. This type must be convertible to U by static_cast.
	/// @tparam U Type of the output value.
	/// @return Return false if the key was not found otherwise true.
	template<typename T, typename U>
	static bool tryReadType(const nlohmann::json& json, const char* key, U& outValue)
	{
		auto it = json.find(key);
		if (it == json.end())
			return false;

		outValue = static_cast<U>(it->get<T>());
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
		auto it = json.find(key);
		if (it == json.end())
			return false;

		outValue = parser(it->get<T>());
		return true;
	}

	/// @brief Reads a value form a JSON object by key and stores it in outValue if it exists.
	/// @tparam T Type of the value to read.
	/// @return Return false if the key was not found otherwise true.
	template<typename T>
	static bool tryReadOptional(const nlohmann::json& json, const char* key, std::optional<T>& outValue)
	{
		outValue = std::nullopt;
		auto it = json.find(key);
		if (it == json.end())
			return false;

		outValue = it->get<T>();
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
		auto it = json.find(key);
		if (it == json.end())
			return false;

		auto valueU = static_cast<U>(it->get<T>());
		outValue = valueU;
		return true;
	}

	/// @brief Reads a vector from a JSON object by key and stores it in outValue.
	/// @tparam T Type of the vector elements.
	/// @return Return false if the key was not found otherwise true.
	template<typename T>
	static bool tryReadVector(const nlohmann::json& json, const char* key, std::vector<T>& outValue)
	{
		auto it = json.find(key);
		if (it == json.end() || !it->is_array())
			return false;

		outValue.clear();
		for (const auto& element : *it)
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
		auto it = json.find(key);
		if (it == json.end() || !it->is_array() || it->size() != Size)
			return false;

		for (size_t i = 0; i < Size; ++i)
		{
			outValue[i] = (*it)[i].get<T>();
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
		else assert(false && "Invalid accessor type");
		return Accessor::Type{};
	}

	///////////////////////////////////////////////////////////////////////////////////////////

	static bool readAsset(Asset& asset, const nlohmann::json& json)
	{
		auto assetIt = json.find("asset");
		if (assetIt == json.end()) // Asset is required -> return error
		{
			assert(false && "GLTF asset is required");
			return false;
		}

		if (!tryRead(*assetIt, "version", asset.version))
		{
			assert(false && "GLTF asset version is required");
			return false;
		}

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

			if (matrixFound && (translationFound || rotationFound || scaleFound))
			{
				assert(false && "Node has both matrix and TRS transform");
				return false;
			}

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

	static bool readAttributes(std::vector<Mesh::Primitive::Attribute>& attributes, const nlohmann::json& json)
	{
		auto attributesIt = json.find("attributes");
		if (attributesIt == json.end()) // Attributes are required -> return error
			return false;

		attributes.reserve(attributesIt->size());
		for (const auto& [key, value] : attributesIt->items())
		{
			auto& attribute = attributes.emplace_back();
			attribute.semantic = key;
			attribute.accessor = value.get<size_t>();
		}

		return true;
	}

	static bool readPrimitives(std::vector<Mesh::Primitive>& primitives, const nlohmann::json& json)
	{
		auto primitivesIt = json.find("primitives");
		if (primitivesIt == json.end() || !primitivesIt->is_array()) // Primitives are required -> return error
			return false;

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

			if (!tryRead(jsonAccessor, "count", gltfAccessor.count))
			{
				assert(false && "Accessor count is required");
				return false;
			}

			if (!tryReadType<int>(jsonAccessor, "componentType", gltfAccessor.componentType))
			{
				assert(false && "Accessor componentType is required");
				return false;
			}

			if (!tryReadParse<std::string, Accessor::Type>(jsonAccessor, "type", gltfAccessor.type, parseAccessorType))
			{
				assert(false && "Accessor type is required");
				return false;
			}

			tryReadOptional<size_t>(jsonAccessor, "bufferView", gltfAccessor.bufferView);
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

			if (!tryRead(jsonBufferView, "buffer", gltfBufferView.buffer))
			{
				assert(false && "BufferView buffer is required");
				return false;
			}

			if (!tryRead(jsonBufferView, "byteLength", gltfBufferView.byteLength))
			{
				assert(false && "BufferView byteLength is required");
				return false;
			}

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

			if (!tryRead(jsonBuffer, "byteLength", gltfBuffer.byteLength))
			{
				assert(false && "Buffer byteLength is required");
				return false;
			}

			tryReadOptional<std::string>(jsonBuffer, "uri", gltfBuffer.uri);
			tryReadOptional<std::string>(jsonBuffer, "name", gltfBuffer.name);
		}

		return true;
	}

	static bool readPBR(Material& material, const nlohmann::json& json)
	{
		auto pbrIt = json.find("pbrMetallicRoughness");
		if (pbrIt == json.end())  // PBR is optional -> return no error
			return true;

		Material::PBRMetallicRoughness pbr{};
		tryReadArray<float, 4>(*pbrIt, "baseColorFactor", pbr.baseColorFactor);
		tryRead(*pbrIt, "metallicFactor", pbr.metallicFactor);
		tryRead(*pbrIt, "roughnessFactor", pbr.roughnessFactor);

		auto baseColorTextureIt = pbrIt->find("baseColorTexture");
		if (baseColorTextureIt != pbrIt->end())
		{
			Material::TextureInfo baseColor{};
			if (!tryRead(*baseColorTextureIt, "index", baseColor.index))
			{
				assert(false && "Texture index is required");
				return false;
			}
			tryRead(*baseColorTextureIt, "texCoord", baseColor.texCoord);
			pbr.baseColorTexture = baseColor;
		}

		auto metallicRoughnessTextureIt = pbrIt->find("metallicRoughnessTexture");
		if (metallicRoughnessTextureIt != pbrIt->end())
		{
			Material::TextureInfo metallicRoughness{};
			if (!tryRead(*metallicRoughnessTextureIt, "index", metallicRoughness.index))
			{
				assert(false && "Metallic roughness texture index is required");
				return false;
			}
			tryRead(*metallicRoughnessTextureIt, "texCoord", metallicRoughness.texCoord);
			pbr.metallicRoughnessTexture = metallicRoughness;
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
		if (!tryRead(*normalIt, "index", normal.index))
		{
			assert(false && "Normal texture index is required");
			return false;
		}
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
		if (!tryRead(*occlusionIt, "index", occlusion.index))
		{
			assert(false && "Occlusion texture index is required");
			return false;
		}
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
		if (!tryRead(*emissiveIt, "index", emissive.index))
		{
			assert(false && "Emissive texture index is required");
			return false;
		}
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

			tryReadOptional<std::string>(jsonMaterial, "name", material.name);
			tryReadArray<float, 3>(jsonMaterial, "emissiveFactor", material.emissiveFactor);
			tryReadType<int>(jsonMaterial, "alphaMode", material.alphaMode);
			tryRead(jsonMaterial, "alphaCutoff", material.alphaCutoff);
			tryRead(jsonMaterial, "doubleSided", material.doubleSided);

			if (!readPBR(material, jsonMaterial))
				return false;

			if (!readNormal(material, jsonMaterial))
				return false;

			if (!readOcclusion(material, jsonMaterial))
				return false;

			if (!readEmissive(material, jsonMaterial))
				return false;
		}

		return true;
	}

	static std::optional<GLTF> parseGLTF(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::in);
		if (!file.is_open())
			return std::nullopt;

		nlohmann::json jsonData = nlohmann::json::parse(file);
		file.close();

		GLTF gltf;
		if (!readAsset(gltf.asset, jsonData) ||
			!readStartScene(gltf.startScene, jsonData) ||
			!readScenes(gltf.scenes, jsonData) ||
			!readNodes(gltf.nodes, jsonData) ||
			!readMeshes(gltf.meshes, jsonData) ||
			!readAccessors(gltf.accessors, jsonData) ||
			!readBufferViews(gltf.bufferViews, jsonData) ||
			!readBuffers(gltf.buffers, jsonData) ||
			!readMaterials(gltf.materials, jsonData)
			)
		{
			return std::nullopt;
		}

		return gltf;
	}

	static std::optional<GLTF> parseGLB(const std::filesystem::path& path)
	{
		// TODO: Implement GLB parsing
		return std::nullopt;
	}

	std::optional<GLTF> load(const std::filesystem::path& path)
	{
		if (path.extension() == ".gltf")
		{
			return parseGLTF(path);
		}
		else if (path.extension() == ".glb")
		{
			return parseGLB(path);
		}
		else // Unsupported file format
		{
			return std::nullopt;
		}
	}
}
