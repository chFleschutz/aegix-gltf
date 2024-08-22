#include "gltf.h"

#include <iostream>
#include <iomanip>

int main()
{
	std::filesystem::path gltfFilePath = PROJECT_DIR "/DamagedHelmet.gltf";
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
			std::cout << "\t\tMode:     \t" << static_cast<int>(primitive.mode) << std::endl;
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
		std::cout << "\tComponentType: \t" << static_cast<int>(accessor.componentType) << std::endl;
		std::cout << "\tCount:         \t" << accessor.count << std::endl;
		std::cout << "\tType:          \t" << static_cast<int>(accessor.type) << std::endl;
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
			std::cout << "\tTarget:        \t" << static_cast<int>(bufferView.target.value()) << std::endl;
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

	// Materials
	std::cout << "Materials:\n";
	for (const auto& material : gltf->materials)
	{
		std::cout << "\tName:          \t" << material.name.value_or("None") << std::endl;
		std::cout << "\tAlphaMode:     \t" << static_cast<int>(material.alphaMode) << std::endl;
		std::cout << "\tAlphaCutoff:   \t" << material.alphaCutoff << std::endl;
		std::cout << "\tDoubleSided:   \t" << material.doubleSided << std::endl;
		std::cout << "\tEmissiveFactor:\t[ " << material.emissiveFactor[0] << " " << material.emissiveFactor[1] << " " << material.emissiveFactor[2] << " ]" << std::endl;
		if (material.pbrMetallicRoughness.has_value())
		{
			std::cout << "\tPBRMetallicRoughness:\n";
			std::cout << "\t\tBaseColorFactor:         \t[ " << material.pbrMetallicRoughness->baseColorFactor[0] << " " << material.pbrMetallicRoughness->baseColorFactor[1] << " " << material.pbrMetallicRoughness->baseColorFactor[2] << " " << material.pbrMetallicRoughness->baseColorFactor[3] << " ]" << std::endl;
			std::cout << "\t\tMetallicFactor:          \t" << material.pbrMetallicRoughness->metallicFactor << std::endl;
			std::cout << "\t\tRoughnessFactor:         \t" << material.pbrMetallicRoughness->roughnessFactor << std::endl;
			if (material.pbrMetallicRoughness->baseColorTexture.has_value())
				std::cout << "\t\tBaseColorTexture:        \t" << material.pbrMetallicRoughness->baseColorTexture->index << std::endl;
			if (material.pbrMetallicRoughness->metallicRoughnessTexture.has_value())
				std::cout << "\t\tMetallicRoughnessTexture:\t" << material.pbrMetallicRoughness->metallicRoughnessTexture->index << std::endl;
		}
		if (material.normalTexture.has_value())
		{
			std::cout << "\tNormalTexture: \n";
			std::cout << "\t\tIndex:    \t" << material.normalTexture->index << std::endl;
			std::cout << "\t\tTexCoord: \t" << material.normalTexture->texCoord << std::endl;
			std::cout << "\t\tScale:    \t" << material.normalTexture->scale << std::endl;
		}
		if (material.occlusionTexture.has_value())
		{
			std::cout << "\tOcclusionTexture: \n";
			std::cout << "\t\tIndex:    \t" << material.occlusionTexture->index << std::endl;
			std::cout << "\t\tTexCoord: \t" << material.occlusionTexture->texCoord << std::endl;
			std::cout << "\t\tStrength: \t" << material.occlusionTexture->strength << std::endl;
		}
		if (material.emissiveTexture.has_value())
		{
			std::cout << "\tEmissiveTexture: \n";
			std::cout << "\t\tIndex:    \t" << material.emissiveTexture->index << std::endl;
			std::cout << "\t\tTexCoord: \t" << material.emissiveTexture->texCoord << std::endl;
		}
	}

	// Textures
	std::cout << "Textures:\n";
	for (const auto& texture : gltf->textures)
	{
		std::cout << "\tName:          \t" << texture.name.value_or("None") << std::endl;
		if (texture.sampler.has_value())
			std::cout << "\tSampler:       \t" << texture.sampler.value() << std::endl;
		if (texture.source.has_value())
			std::cout << "\tSource:        \t" << texture.source.value() << std::endl;
		std::cout << "\n";
	}

	// Images
	std::cout << "Images:\n";
	for (const auto& image : gltf->images)
	{
		std::cout << "\tName:          \t" << image.name.value_or("None") << std::endl;
		std::visit([](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Aegix::GLTF::Image::UriData>)
			{
				std::cout << "\tURI:           \t" << arg.uri << std::endl;
			}
			else if constexpr (std::is_same_v<T, Aegix::GLTF::Image::BufferViewData>)
			{
				std::cout << "\tBufferView:    \t" << arg.bufferView << std::endl;
				std::cout << "\tMIME Type:     \t" << arg.mimeType << std::endl;
			}
			}, image.data);

		std::cout << "\n";
	}

	// Samplers
	std::cout << "Samplers:\n";
	for (const auto& sampler : gltf->samplers)
	{
		std::cout << "\tName:          \t" << sampler.name.value_or("None") << std::endl;
		if (sampler.magFilter.has_value())
			std::cout << "\tMagFilter:     \t" << static_cast<int>(sampler.magFilter.value()) << std::endl;
		if (sampler.minFilter.has_value())
			std::cout << "\tMinFilter:     \t" << static_cast<int>(sampler.minFilter.value()) << std::endl;
		std::cout << "\tWrapS:         \t" << static_cast<int>(sampler.wrapS) << std::endl;
		std::cout << "\tWrapT:         \t" << static_cast<int>(sampler.wrapT) << std::endl;
		std::cout << "\n";
	}

	return 0;
}