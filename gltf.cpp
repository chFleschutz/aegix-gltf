#include "gltf.h"

#include "json/json.hpp"

#include <fstream>
#include <iostream>

namespace Aegix::GLTF
{
	template<typename T>
	static bool tryRead(const nlohmann::json& json, const char* key, T& outValue)
	{
		auto it = json.find(key);
		if (it == json.end())
			return false;

		outValue = it->get<T>();
		return true;
	}

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

	static bool readAsset(GLTF& gltf, const nlohmann::json& json)
	{
		if (!json.contains("asset"))
		{
			assert(false && "GLTF asset is required");
			return false;
		}
		
		const auto& asset = json["asset"];

		if (!tryRead(asset, "version", gltf.asset.version))
		{
			assert(false && "GLTF asset version is required");
			return false;
		}

		tryReadOptional<std::string>(asset, "generator", gltf.asset.generator);
		tryReadOptional<std::string>(asset, "minVersion", gltf.asset.minVersion);
		tryReadOptional<std::string>(asset, "copyright", gltf.asset.copyright);

		return true;
	}

	static bool readScenes(GLTF& gltf, const nlohmann::json& json)
	{
		// scene is optional
		tryReadOptional(json, "scene", gltf.defaultScene); 

		// Spec: "MAY contain zero or more scenes"
		if (json.contains("scenes") && json["scenes"].is_array())
		{
			for (const auto& jsonScene : json["scenes"])
			{
				auto& gltfScene = gltf.scenes.emplace_back();
				tryReadOptional<std::string>(jsonScene, "name", gltfScene.name);
				tryReadVector<size_t>(jsonScene, "nodes", gltfScene.nodes);
			}
		}

		return true;
	}

	static bool readNodes(GLTF& gltf, const nlohmann::json& json)
	{
		// Spec: "MAY define nodes"
		auto nodesIt = json.find("nodes");
		if (nodesIt == json.end() || !nodesIt->is_array())
			return true;

		for (const auto& jsonNode : *nodesIt)
		{
			auto& gltfNode = gltf.nodes.emplace_back();

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

	static bool readMeshes(GLTF& gltf, const nlohmann::json& json)
	{

		return true;
	}

	static bool readAccessors(GLTF& gltf, const nlohmann::json& json)
	{

		return true;
	}

	static bool readBufferViews(GLTF& gltf, const nlohmann::json& json)
	{

		return true;
	}

	static bool readBuffers(GLTF& gltf, const nlohmann::json& json)
	{

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
		if (!readAsset(gltf, jsonData) ||
			!readScenes(gltf, jsonData) ||
			!readNodes(gltf, jsonData) ||
			!readMeshes(gltf, jsonData) ||
			!readAccessors(gltf, jsonData) ||
			!readBufferViews(gltf, jsonData) ||
			!readBuffers(gltf, jsonData))
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
