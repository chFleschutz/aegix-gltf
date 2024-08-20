#include "gltf.h"

#include <iostream>

int main()
{
	std::filesystem::path gltfFilePath = PROJECT_DIR "/cube.gltf";
	auto gltf = Aegix::GLTF::load(gltfFilePath);
	if (!gltf)
	{
		std::cerr << "Failed to load GLTF file: " << gltfFilePath << std::endl;
		return 1;
	}

	// Asset
	std::cout << "Asset:" << std::endl;
	std::cout << "\tVersion: \t" << gltf->asset.version << std::endl;
	std::cout << "\tGenerator: \t" << gltf->asset.generator.value_or("No generator") << std::endl;
	std::cout << "\tMin Version: \t" << gltf->asset.minVersion.value_or("No minVersion") << std::endl;
	std::cout << "\tCopyright: \t" << gltf->asset.copyright.value_or("No copyright") << "\n\n";

	// Scenes
	if (gltf->defaultScene.has_value())
		std::cout << "Default Scene: \t" << gltf->defaultScene.value() << std::endl;
	std::cout << "Scenes:" << std::endl;
	for (const auto& scene : gltf->scenes)
	{
		std::cout << "\tName: \t" << scene.name.value_or("No name") << std::endl;
		std::cout << "\tNodes: \t[ ";
		for (const auto& nodeIndex : scene.nodes)
		{
			std::cout << nodeIndex << " ";
		}
		std::cout << "]\n\n";
	}

	return 0;
}