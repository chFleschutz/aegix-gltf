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
	std::cout << std::fixed << std::setprecision(2) << std::boolalpha;

	// Asset
	std::cout << "Asset:" << std::endl;
	std::cout << "\tVersion: \t" << gltf->asset.version << std::endl;
	std::cout << "\tGenerator: \t" << gltf->asset.generator.value_or("No generator") << std::endl;
	std::cout << "\tMin Version: \t" << gltf->asset.minVersion.value_or("No minVersion") << std::endl;
	std::cout << "\tCopyright: \t" << gltf->asset.copyright.value_or("No copyright") << "\n\n";

	// Scenes
	if (gltf->startScene.has_value())
		std::cout << "Start Scene: \t" << gltf->startScene.value() << std::endl;
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
		std::cout << "\tName: \t" << node.name.value_or("None") << std::endl;
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

	// Meshes
	std::cout << "Meshes:\n";
	for (const auto& mesh : gltf->meshes)
	{
		std::cout << "\tName: \t" << mesh.name.value_or("None") << std::endl;
		std::cout << "\tPrimitives:\n";
		for (const auto& primitive : mesh.primitives)
		{
			if (primitive.indices.has_value())
				std::cout << "\t\tIndices: \t" << primitive.indices.value() << std::endl;
			if (primitive.material.has_value())
				std::cout << "\t\tMaterial: \t" << primitive.material.value() << std::endl;
			std::cout << "\t\tMode:     \t" << primitive.mode << std::endl;
			std::cout << "\t\tAttributes:\n";
			for (const auto& [semantic, accessorIndex] : primitive.attributes)
			{
				std::cout << "\t\t\t" << semantic << ": \t" << accessorIndex << std::endl;
			}
			std::cout << "\n";
		}
	}

	// Accessors
	std::cout << "Accessors:\n";
	for (const auto& accessor : gltf->accessors)
	{
		std::cout << "\tName:          \t" << accessor.name.value_or("None") << std::endl;
		if (accessor.bufferView.has_value())
			std::cout << "\tBufferView:    \t" << accessor.bufferView.value() << std::endl;
		std::cout << "\tByteOffset:    \t" << accessor.byteOffset << std::endl;
		std::cout << "\tNormalized:    \t" << accessor.normalized << std::endl;
		std::cout << "\tComponentType: \t" << accessor.componentType << std::endl;
		std::cout << "\tCount:         \t" << accessor.count << std::endl;
		std::cout << "\tType:          \t" << accessor.type << std::endl;
		std::cout << "\tMax:           \t[ ";
		for (const auto& max : accessor.max)
		{
			std::cout << max << " ";
		}
		std::cout << "]" << std::endl;
		std::cout << "\tMin:           \t[ ";
		for (const auto& min : accessor.min)
		{
			std::cout << min << " ";
		}
		std::cout << "]" << std::endl;
		std::cout << "\n";
	}

	// Buffer Views
	std::cout << "Buffer Views:\n";
	for (const auto& bufferView : gltf->bufferViews)
	{
		std::cout << "\tName:          \t" << bufferView.name.value_or("None") << std::endl;
		std::cout << "\tBuffer:        \t" << bufferView.buffer << std::endl;
		std::cout << "\tByteOffset:    \t" << bufferView.byteOffset << std::endl;
		std::cout << "\tByteLength:    \t" << bufferView.byteLength << std::endl;
		if (bufferView.byteStride.has_value())
			std::cout << "\tByteStride:    \t" << bufferView.byteStride.value() << std::endl;
		if (bufferView.target.has_value())
			std::cout << "\tTarget:        \t" << bufferView.target.value() << std::endl;
		std::cout << "\n";
	}

	// Buffers
	std::cout << "Buffers:\n";
	for (const auto& buffer : gltf->buffers)
	{
		std::cout << "\tName:          \t" << buffer.name.value_or("None") << std::endl;
		std::cout << "\tByteLength:    \t" << buffer.byteLength << std::endl;
		if (buffer.uri.has_value())
			std::cout << "\tURI:           \t" << buffer.uri.value() << std::endl;
		std::cout << "\n";
	}

	return 0;
}