#pragma once

#include "gltf.h"

#include <iostream>

namespace Aegix::GLTF
{
	std::ostream& operator<<(std::ostream& os, const Mesh::Primitive::Mode& mode)
	{
		switch (mode)
		{
		case Mesh::Primitive::Mode::Points: return os << "Points";
		case Mesh::Primitive::Mode::Lines: return os << "Lines";
		case Mesh::Primitive::Mode::LineLoop: return os << "Line Loop";
		case Mesh::Primitive::Mode::LineStrip: return os << "Line Strip";
		case Mesh::Primitive::Mode::Triangles: return os << "Triangles";
		case Mesh::Primitive::Mode::TriangleStrip: return os << "Triangle Strip";
		case Mesh::Primitive::Mode::TriangleFan: return os << "Triangle Fan";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const Accessor::ComponentType& mode)
	{
		switch (mode)
		{
		case Accessor::ComponentType::Byte: return os << "Byte";
		case Accessor::ComponentType::UnsignedByte: return os << "Unsigned Byte";
		case Accessor::ComponentType::Short: return os << "Short";
		case Accessor::ComponentType::UnsignedShort: return os << "Unsigned Short";
		case Accessor::ComponentType::UnsignedInt: return os << "Unsigned Int";
		case Accessor::ComponentType::Float: return os << "Float";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const Accessor::Type& mode)
	{
		switch (mode)
		{
		case Accessor::Type::Scalar: return os << "Scalar";
		case Accessor::Type::Vec2: return os << "Vec2";
		case Accessor::Type::Vec3: return os << "Vec3";
		case Accessor::Type::Vec4: return os << "Vec4";
		case Accessor::Type::Mat2: return os << "Mat2";
		case Accessor::Type::Mat3: return os << "Mat3";
		case Accessor::Type::Mat4: return os << "Mat4";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const BufferView::Target& mode)
	{
		switch (mode)
		{
		case BufferView::Target::ArrayBuffer: return os << "Array Buffer";
		case BufferView::Target::ElementArrayBuffer: return os << "Element Array Buffer";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const Material::AlphaMode& mode)
	{
		switch (mode)
		{
		case Material::AlphaMode::Opaque: return os << "Opaque";
		case Material::AlphaMode::Mask: return os << "Mask";
		case Material::AlphaMode::AlphaCutoff: return os << "Alpha Cutoff";
		case Material::AlphaMode::Blend: return os << "Blend";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const Sampler::MagFilter& mode)
	{
		switch (mode)
		{
		case Sampler::MagFilter::Nearest: return os << "Nearest";
		case Sampler::MagFilter::Linear: return os << "Linear";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const Sampler::MinFilter& mode)
	{
		switch (mode)
		{
		case Sampler::MinFilter::Nearest: return os << "Nearest";
		case Sampler::MinFilter::Linear: return os << "Linear";
		case Sampler::MinFilter::NearestMipmapNearest: return os << "Nearest Mipmap Nearest";
		case Sampler::MinFilter::LinearMipmapNearest: return os << "Linear Mipmap Nearest";
		case Sampler::MinFilter::NearestMipmapLinear: return os << "Nearest Mipmap Linear";
		case Sampler::MinFilter::LinearMipmapLinear: return os << "Linear Mipmap Linear";
		default: return os << "Unknown";
		}
	}

	std::ostream& operator<<(std::ostream& os, const Sampler::WrapMode& mode)
	{
		switch (mode)
		{
		case Sampler::WrapMode::ClampToEdge: return os << "Clamp To Edge";
		case Sampler::WrapMode::MirroredRepeat: return os << "Mirrored Repeat";
		case Sampler::WrapMode::Repeat: return os << "Repeat";
		default: return os << "Unknown";
		}
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt)
	{
		if (opt.has_value()) {
			os << *opt;
		}
		else {
			os << "std::nullopt";
		}
		return os;
	}

	template<typename T, std::size_t N>
	std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
	{
		os << "[ ";
		for (std::size_t i = 0; i < N; ++i) {
			os << arr[i];
			if (i < N - 1)
				os << ", ";
		}
		os << " ]";
		return os;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
	{
		os << "[ ";
		auto size = vec.size();
		for (std::size_t i = 0; i < size; ++i) {
			os << vec[i];
			if (i < size - 1)
				os << ", ";
		}
		os << " ]";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Asset& asset)
	{
		os << "Version:     \t" << asset.version << "\n";
		os << "Generator:   \t" << asset.generator << "\n";
		os << "Min Version: \t" << asset.minVersion << "\n";
		os << "Copyright:   \t" << asset.copyright << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Scene& scene)
	{
		os << "\tName:  \t" << scene.name << "\n";
		os << "\tNodes: \t" << scene.nodes << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Node& node)
	{
		os << "\tName: \t" << node.name << "\n";
		os << "\tChildren: \t" << node.children << "\n";
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Aegix::GLTF::Mat4>)
			{
				os << "\tTransform: \tMat4\n";
				for (size_t iCol = 0; iCol < 4; ++iCol)
				{
					os << "\t\t";
					for (size_t iRow = 0; iRow < 4; ++iRow)
					{
						os << arg[(iRow * 4) + iCol] << " ";
					}
					os << "\n";
				}
				os << "\n";
			}
			else if constexpr (std::is_same_v<T, Aegix::GLTF::Node::TRS>)
			{
				os << "\tTransform: \tTRS\n";
				os << "\t\tTranslation: \t " << arg.translation << "\n";
				os << "\t\tRotation:    \t " << arg.rotation << "\n";
				os << "\t\tScale:       \t " << arg.scale << "\n\n";
			}
			}, node.transform);
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mesh::Primitive::Attribute& attribute)
	{
		return os << attribute.semantic << ": \t" << attribute.accessor << "\n";
	}

	std::ostream& operator<<(std::ostream& os, const Mesh::Primitive& primitive)
	{
		os << "\t\tIndices:  \t" << primitive.indices << "\n";
		os << "\t\tMaterial: \t" << primitive.material << "\n";
		os << "\t\tMode:     \t" << primitive.mode << "\n";
		os << "\t\tAttributes:\n";
		for (const auto& attribute : primitive.attributes)
		{
			os << "\t\t\t" << attribute;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Mesh& mesh)
	{
		os << "\tName: \t" << mesh.name << "\n";
		os << "\tPrimitives:\n";
		for (const auto& primitive : mesh.primitives)
		{
			os << primitive;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Accessor& accessor)
	{
		os << "\tName:          \t" << accessor.name << "\n";
		os << "\tBufferView:    \t" << accessor.bufferView << "\n";
		os << "\tByteOffset:    \t" << accessor.byteOffset << "\n";
		os << "\tNormalized:    \t" << accessor.normalized << "\n";
		os << "\tComponentType: \t" << accessor.componentType << "\n";
		os << "\tCount:         \t" << accessor.count << "\n";
		os << "\tType:          \t" << accessor.type << "\n";
		os << "\tMax:           \t" << accessor.max << "\n";
		os << "\tMin:           \t" << accessor.min << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const BufferView& bufferView)
	{
		os << "\tName:          \t" << bufferView.name << "\n";
		os << "\tBuffer:        \t" << bufferView.buffer << "\n";
		os << "\tByteOffset:    \t" << bufferView.byteOffset << "\n";
		os << "\tByteLength:    \t" << bufferView.byteLength << "\n";
		os << "\tByteStride:    \t" << bufferView.byteStride << "\n";
		os << "\tTarget:        \t" << bufferView.target << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Buffer& buffer)
	{
		os << "\tName:          \t" << buffer.name << "\n";
		os << "\tByteLength:    \t" << buffer.byteLength << "\n";
		os << "\tURI:           \t" << buffer.uri << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Material::TextureInfo& material)
	{
		os << "\t\tIndex:    \t" << material.index << "\n";
		os << "\t\tTexCoord: \t" << material.texCoord << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Material::NormalTextureInfo& material)
	{
		os << "\t\tIndex:    \t" << material.index << "\n";
		os << "\t\tTexCoord: \t" << material.texCoord << "\n";
		os << "\t\tScale:    \t" << material.scale << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Material::OcclusionTextureInfo& material)
	{
		os << "\t\tIndex:    \t" << material.index << "\n";
		os << "\t\tTexCoord: \t" << material.texCoord << "\n";
		os << "\t\tStrength: \t" << material.strength << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Material::PBRMetallicRoughness& pbr)
	{
		os << "\t\tBaseColorFactor:         \t" << pbr.baseColorFactor << "\n";
		os << "\t\tMetallicFactor:          \t" << pbr.metallicFactor << "\n";
		os << "\t\tRoughnessFactor:         \t" << pbr.roughnessFactor << "\n";
		os << "\t\tBaseColorTexture:        \n" << pbr.baseColorTexture;
		os << "\t\tMetallicRoughnessTexture:\n" << pbr.metallicRoughnessTexture;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Material& material)
	{
		os << "\tName:          \t" << material.name << "\n";
		os << "\tAlphaMode:     \t" << material.alphaMode << "\n";
		os << "\tAlphaCutoff:   \t" << material.alphaCutoff << "\n";
		os << "\tDoubleSided:   \t" << material.doubleSided << "\n";
		os << "\tEmissiveFactor:\t" << material.emissiveFactor << "\n";
		os << "\tPBRMetallicRoughness:\n" << material.pbrMetallicRoughness;
		os << "\tNormalTexture: \n" << material.normalTexture;
		os << "\tOcclusionTexture: \n" << material.occlusionTexture;
		os << "\tEmissiveTexture: \n" << material.emissiveTexture;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Texture& texture)
	{
		os << "\tName:    \t" << texture.name << "\n";
		os << "\tSampler: \t" << texture.sampler << "\n";
		os << "\tSource:  \t" << texture.source << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Image& image)
	{
		os << "\tName:       \t" << image.name << "\n";
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, Aegix::GLTF::Image::UriData>)
			{
				os << "\tURI:        \t" << arg.uri << "\n";
			}
			else if constexpr (std::is_same_v<T, Aegix::GLTF::Image::BufferViewData>)
			{
				os << "\tBufferView: \t" << arg.bufferView << "\n";
				os << "\tMIME Type:  \t" << arg.mimeType << "\n";
			}
			}, image.data);
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const Sampler& sampler)
	{
		os << "\tName:      \t" << sampler.name << "\n";
		os << "\tMagFilter: \t" << sampler.magFilter << "\n";
		os << "\tMinFilter: \t" << sampler.minFilter << "\n";
		os << "\tWrapS:     \t" << sampler.wrapS << "\n";
		os << "\tWrapT:     \t" << sampler.wrapT << "\n";
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const GLTF& gltf)
	{
		os << std::fixed << std::setprecision(2) << std::boolalpha;
		os << "Asset:\n";
		os << gltf.asset;
		os << "\nStart Scene: \t" << gltf.startScene << "\n";
		os << "\nScenes:\n";
		for (const auto& scene : gltf.scenes)
			os << scene << "\n";
		os << "\nNodes:\n";
		for (const auto& node : gltf.nodes)
			os << node << "\n";
		os << "\nMeshes:\n";
		for (const auto& mesh : gltf.meshes)
			os << mesh << "\n";
		os << "\nAccessors:\n";
		for (const auto& accessor : gltf.accessors)
			os << accessor << "\n";
		os << "\nBufferViews:\n";
		for (const auto& bufferView : gltf.bufferViews)
			os << bufferView << "\n";
		os << "\nBuffers:\n";
		for (const auto& buffer : gltf.buffers)
			os << buffer << "\n";
		os << "\nMaterials:\n";
		for (const auto& material : gltf.materials)
			os << material << "\n";
		os << "\nTextures:\n";
		for (const auto& texture : gltf.textures)
			os << texture << "\n";
		os << "\nImages:\n";
		for (const auto& image : gltf.images)
			os << image << "\n";
		os << "\nSamplers:\n";
		for (const auto& sampler : gltf.samplers)
			os << sampler << "\n";
		return os;
	}
}
