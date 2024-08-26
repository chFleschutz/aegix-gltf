#pragma once

#include "gltf.h"

#include <vector>
#include <cassert>

namespace Aegix::GLTF
{
	/// @brief Reinterpret the binary sourceData as T and copy it to the destination vector
	/// @tparam T Type to reinterpret the binary sourceData as
	/// @tparam U Type of the destination vector
	/// @param destination Vector to copy the data to
	/// @param sourceData Pointer to the binary data to reinterpret
	/// @param elementCount Number of elements to copy (not bytes)
	template<typename T, typename U>
	static void copyDataReinterpretedAsType(std::vector<U>& destination, const uint8_t* sourceData, size_t elementCount)
	{
		destination.reserve(elementCount);

		auto sourcePtr = reinterpret_cast<const T*>(sourceData);
		for (size_t i = 0; i < elementCount; ++i)
		{
			destination.emplace_back(static_cast<U>(sourcePtr[i]));
		}
	}

	/// @brief Reinterpret the binary data as type and copy it to the destination vector
	/// @tparam T Type of the destination vector
	/// @param type Type to reinterpret the binary data as
	/// @param destination Vector to copy the data to
	/// @param data Pointer to the binary data 
	/// @param elementCount Number of elements to copy (not bytes)
	template<typename T>
	static void copyDataReinterpretedAs(Accessor::ComponentType type, std::vector<T>& destination, const uint8_t* data, size_t elementCount)
	{
		switch (type)
		{
		case Accessor::ComponentType::Byte:
			copyDataReinterpretedAsType<int8_t>(destination, data, elementCount);
			return;
		case Accessor::ComponentType::UnsignedByte:
			copyDataReinterpretedAsType<uint8_t>(destination, data, elementCount);
			return;
		case Accessor::ComponentType::Short:
			copyDataReinterpretedAsType<int16_t>(destination, data, elementCount);
			return;
		case Accessor::ComponentType::UnsignedShort:
			copyDataReinterpretedAsType<uint16_t>(destination, data, elementCount);
			return;
		case Accessor::ComponentType::UnsignedInt:
			copyDataReinterpretedAsType<uint32_t>(destination, data, elementCount);
			return;
		case Accessor::ComponentType::Float:
			copyDataReinterpretedAsType<float>(destination, data, elementCount);
			return;
		default:
			assert(false && "Invalid component type");
			return;
		}
	}

	/// @brief Copy data to the destination vector from the buffer accessor reinterpreting it as the ComponentType of the accessor
	/// @tparam T Type of the destination vector
	/// @param destination Vector to copy the data to
	/// @param accessorIndex Index of the buffer accessor to copy the data from
	/// @param gltf GLTF data
	template<typename T>
	static void copyDataReinterpreted(std::vector<T>& destination, size_t accessorIndex, const GLTF& gltf)
	{
		auto& accessor = gltf.accessors[accessorIndex];
		auto& bufferView = gltf.bufferViews[accessor.bufferView];
		auto& buffer = gltf.buffers[bufferView.buffer];

		auto data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
		copyDataReinterpretedAs(accessor.componentType, destination, data, accessor.count);
	}

	/// @brief Copy data to the destination vector from the buffer accessor
	/// @tparam T Type of the destination vector
	/// @param destination Vector to copy the data to
	/// @param accessorIndex Index of the buffer accessor to copy the data from
	/// @note The data is copied as is, no reinterpretation is done
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

	/// @brief Copy the indices of the primitive to the destination vector if they exist
	/// @tparam T Type of the destination vector / indices
	/// @param destination Vector to copy the indices to
	/// @param primitive Primitive to copy the indices from
	template<typename T>
	static void copyIndices(std::vector<T>& destination, const Mesh::Primitive& primitive, const GLTF& gltf)
	{
		if (primitive.indices.has_value())
		{
			copyDataReinterpreted(destination, primitive.indices.value(), gltf);
		}
	}

	/// @brief Copy the attribute with the given name to the destination vector if it exists
	/// @tparam T Type of the destination vector
	/// @param attributeName Name of the attribute to copy
	/// @param destination Vector to copy the attribute to
	/// @param primitive Primitive to copy the attribute from
	/// @note The data is copied as is, no reinterpretation is done
	template<typename T>
	static void copyAttribute(std::string_view attributeName, std::vector<T>& destination, const Mesh::Primitive& primitive, const GLTF& gltf)
	{
		auto attributeIt = primitive.attributes.find(attributeName.data());
		if (attributeIt != primitive.attributes.end())
		{
			copyData(destination, attributeIt->second, gltf);
		}
	}
}