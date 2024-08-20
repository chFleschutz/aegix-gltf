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

	static bool readAsset(GLTF& gltf, const nlohmann::json& json)
	{
		if (!json.contains("asset")) // asset is required
			return false;
		
		const auto& asset = json["asset"];

		if (!tryRead(asset, "version", gltf.asset.version)) // version is required
			return false;

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
