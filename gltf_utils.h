#pragma once

#include "gltf.h"

#include <vector>

namespace Aegix::GLTF
{
	template<typename To, typename From>
	static std::vector<To> convertToType(const uint8_t* byteData, size_t elementCount)
	{
		std::vector<To> output{};
		output.reserve(elementCount);

		auto fromPtr = reinterpret_cast<const From*>(byteData);
		for (size_t i = 0; i < elementCount; ++i)
		{
			output.push_back(static_cast<To>(fromPtr[i]));
		}

		return output;
	}

	template<typename T>
	static std::vector<T> convertTo(Accessor::ComponentType type, const uint8_t* byteData, size_t elementCount)
	{
		switch (type)
		{
		case Accessor::ComponentType::Byte:
			return convertToType<T, int8_t>(byteData, elementCount);
		case Accessor::ComponentType::UnsignedByte:
			return convertToType<T, uint8_t>(byteData, elementCount);
		case Accessor::ComponentType::Short:
			return convertToType<T, int16_t>(byteData, elementCount);
		case Accessor::ComponentType::UnsignedShort:
			return convertToType<T, uint16_t>(byteData, elementCount);
		case Accessor::ComponentType::UnsignedInt:
			return convertToType<T, uint32_t>(byteData, elementCount);
		case Accessor::ComponentType::Float:
			return convertToType<T, float>(byteData, elementCount);
		default:
			return {};
		}
	}

	template<typename T>
	static std::vector<T> queryDataAs(size_t accessorIndex, const GLTF& gltf)
	{
		auto& accessor = gltf.accessors[accessorIndex];
		auto& bufferView = gltf.bufferViews[accessor.bufferView];
		auto& buffer = gltf.buffers[bufferView.buffer];

		auto data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
		return convertTo<T>(accessor.componentType, data, accessor.count);
	}

	template<typename T>
	static void copyData(std::vector<T>& destination, size_t accessorIndex, const GLTF& gltf)
	{
		auto& accessor = gltf.accessors[accessorIndex];
		auto& bufferView = gltf.bufferViews[accessor.bufferView];
		auto& buffer = gltf.buffers[bufferView.buffer];

		destination.resize(accessor.count);
		auto dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
		std::memcpy(destination.data(), dataPtr, accessor.count * sizeof(T));
	}
}