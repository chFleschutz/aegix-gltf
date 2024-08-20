#include "gltf.h"

#include <iostream>
#include <iomanip>

int main()
{
	std::filesystem::path gltfFilePath = PROJECT_DIR "/cube.gltf";
	auto gltf = Aegix::GLTF::load(gltfFilePath);
	if (!gltf)
	{
		std::cerr << "Failed to load GLTF file: " << gltfFilePath << std::endl;
		return 1;
	}

	std::cout << "GLTF file: " << gltfFilePath << "\n\n";
	std::cout << std::fixed << std::setprecision(2);

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

	// Nodes
	std::cout << "Nodes:" << std::endl;
	for (const auto& node : gltf->nodes)
	{
		std::cout << "\tName: \t" << node.name.value_or("No name") << std::endl;
		std::cout << "\tChildren: \t[ ";
		for (const auto& childIndex : node.children)
		{
			std::cout << childIndex << " ";
		}
		std::cout << "]\n";

		std::visit([](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Aegix::GLTF::Mat4>)
			{
				std::cout << "\tTransform: \tMat4\n";
				for (size_t iCol = 0; iCol < 4; ++iCol)
				{
					std::cout << "\t\t";
					for (size_t iRow = 0; iRow < 4; ++iRow)
					{
						std::cout << arg[(iRow * 4) + iCol] << " ";
					}
					std::cout << "\n";
				}
				std::cout << "\n";
			}
			else if constexpr (std::is_same_v<T, Aegix::GLTF::Node::TRS>)
			{
				std::cout << "\tTransform: \tTRS" << std::endl;
				std::cout << "\t\tTranslation: \t[ " << arg.translation[0] << " " << arg.translation[1] << " " << arg.translation[2] << " ]" << std::endl;
				std::cout << "\t\tRotation:    \t[ " << arg.rotation[0] << " " << arg.rotation[1] << " " << arg.rotation[2] << " " << arg.rotation[3] << " ]" << std::endl;
				std::cout << "\t\tScale:       \t[ " << arg.scale[0] << " " << arg.scale[1] << " " << arg.scale[2] << " ]" << std::endl;
			}
			}, node.transform);
		std::cout << "\n";


	}

	return 0;
}