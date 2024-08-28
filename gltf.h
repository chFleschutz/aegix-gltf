#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Aegix::GLTF
{
	using Vec3 = std::array<float, 3>;
	using Vec4 = std::array<float, 4>;
	using Quat = std::array<float, 4>;
	using Mat4 = std::array<float, 16>;

	constexpr Mat4 MAT4_IDENTITY = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
	};


	constexpr uint32_t GLB_MAGIC = 0x46546C67;		// ASCII: "glTF"
	constexpr uint32_t GLB_VERSION = 2;
	constexpr uint32_t GLB_CHUNK_JSON = 0x4E4F534A;	// ASCII: "JSON"
	constexpr uint32_t GLB_CHUNK_BIN = 0x004E4942;	// ASCII: "BIN "

	struct HeaderGLB
	{
		uint32_t magic = 0;		// Must be 0x46546C67 (ASCII for glTF)
		uint32_t version = 0;
		uint32_t length = 0;
	};

	struct ChunkGLB
	{
		uint32_t length = 0;
		uint32_t type = 0;
	};

	struct Asset
	{
		std::string version;	// Required
		std::optional<std::string> generator;
		std::optional<std::string> minVersion;
		std::optional<std::string> copyright;
	};

	struct Scene
	{
		std::vector<size_t> nodes;
		std::optional<std::string> name;
	};

	struct Node
	{
		struct TRS
		{
			Vec3 translation{ 0.0f, 0.0f, 0.0f };
			Quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
			Vec3 scale{ 1.0f, 1.0f, 1.0f };
		};

		using Transform = std::variant<Mat4, TRS>;

		Transform transform = MAT4_IDENTITY;
		std::vector<size_t> children;
		std::optional<size_t> camera;
		std::optional<size_t> skin;
		std::optional<size_t> mesh;
		std::optional<std::string> name;
		//std::vector<float> weights; // TODO: Morph targets
	};

	struct Mesh
	{
		struct Primitive
		{
			enum class Mode
			{
				Points = 0,
				Lines = 1,
				LineLoop = 2,
				LineStrip = 3,
				Triangles = 4,
				TriangleStrip = 5,
				TriangleFan = 6
			};

			std::unordered_map<std::string, size_t> attributes; // Required
			std::optional<size_t> indices;
			std::optional<size_t> material;
			Mode mode = Mode::Triangles;
			//std::vector<MorphTarget> targets; // TODO: Morph targets
		};

		std::vector<Primitive> primitives;	// Required
		std::vector<float> weights;
		std::optional<std::string> name;
	};

	struct Accessor
	{
		enum class ComponentType
		{
			Byte = 5120,
			UnsignedByte = 5121,
			Short = 5122,
			UnsignedShort = 5123,
			UnsignedInt = 5125,
			Float = 5126
		};

		enum class Type
		{
			Scalar,
			Vec2,
			Vec3,
			Vec4,
			Mat2,
			Mat3,
			Mat4
		};

		size_t bufferView = 0;		 // If undefined, accessor must be initialized with zeros but sparse could override this
		size_t byteOffset = 0;
		size_t count;				 // Required
		ComponentType componentType; // Required
		Type type;					 // Required
		bool normalized = false;

		std::vector<float> min;		// Size depends on type [1, 2, 3, 4, 9, 16]
		std::vector<float> max;		// Size depends on type [1, 2, 3, 4, 9, 16]
		//std::optional<size_t> sparse; // TODO: Sparse accessor

		std::optional<std::string> name;
	};

	struct BufferView
	{
		enum class Target
		{
			ArrayBuffer = 34962,
			ElementArrayBuffer = 34963
		};

		size_t buffer = 0;		// Required
		size_t byteLength = 0;	// Required
		size_t byteOffset = 0;
		std::optional<size_t> byteStride;
		std::optional<Target> target;
		std::optional<std::string> name;
	};

	struct Buffer
	{
		size_t byteLength;	// Required
		std::optional<std::string> uri; // Empty for glb
		std::optional<std::string> name;
		std::vector<uint8_t> data;
	};

	struct Material
	{
		struct TextureInfo
		{
			size_t index;	// Required
			size_t texCoord = 0;
		};

		struct NormalTextureInfo
		{
			size_t index;	// Required
			size_t texCoord = 0;
			float scale = 1.0f;
		};

		struct OcclusionTextureInfo
		{
			size_t index;	// Required
			size_t texCoord = 0;
			float strength = 1.0f;
		};

		struct PBRMetallicRoughness
		{
			Vec4 baseColorFactor{ 1.0f, 1.0f, 1.0f, 1.0f };
			std::optional<TextureInfo> baseColorTexture;
			std::optional<TextureInfo> metallicRoughnessTexture;
			float metallicFactor = 1.0f;
			float roughnessFactor = 1.0f;
		};

		enum class AlphaMode
		{
			Opaque,
			Mask,
			AlphaCutoff,
			Blend
		};

		std::optional<std::string> name;
		std::optional<PBRMetallicRoughness> pbrMetallicRoughness;
		std::optional<NormalTextureInfo> normalTexture;
		std::optional<OcclusionTextureInfo> occlusionTexture;
		std::optional<TextureInfo> emissiveTexture;
		Vec3 emissiveFactor{ 0.0f, 0.0f, 0.0f };
		AlphaMode alphaMode = AlphaMode::Opaque;
		float alphaCutoff = 0.5f;
		bool doubleSided = false;
	};

	struct Texture
	{
		// Spec: When undefined, a sampler with repeat wrapping and auto filtering SHOULD be used
		std::optional<size_t> sampler;
		// Spec: When undefined, an extension or other mechanism SHOULD supply an alternate texture source, otherwise behavior is undefined.
		std::optional<size_t> source;
		std::optional<std::string> name;
	};

	struct Image
	{
		struct UriData
		{
			std::string uri;	// Required
		};

		struct BufferViewData
		{
			std::string mimeType;	// Required
			size_t bufferView;		// Required
		};

		std::variant<UriData, BufferViewData> data;
		std::optional<std::string> name;
	};

	struct Sampler
	{
		enum class MagFilter
		{
			Nearest = 9728,
			Linear = 9729
		};

		enum class MinFilter
		{
			Nearest = 9728,
			Linear = 9729,
			NearestMipmapNearest = 9984,
			LinearMipmapNearest = 9985,
			NearestMipmapLinear = 9986,
			LinearMipmapLinear = 9987
		};

		enum class WrapMode
		{
			ClampToEdge = 33071,
			MirroredRepeat = 33648,
			Repeat = 10497
		};

		std::optional<MagFilter> magFilter;
		std::optional<MinFilter> minFilter;
		WrapMode wrapS = WrapMode::Repeat;
		WrapMode wrapT = WrapMode::Repeat;
		std::optional<std::string> name;
	};

	struct GLTF
	{
		Asset asset;
		std::optional<size_t> startScene;
		std::vector<Scene> scenes;
		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		std::vector<Accessor> accessors;
		std::vector<BufferView> bufferViews;
		std::vector<Buffer> buffers;
		std::vector<Material> materials;
		std::vector<Texture> textures;
		std::vector<Image> images;
		std::vector<Sampler> samplers;
	};



	/// @brief Loads a GLTF file from the specified path
	/// @param path Path to the .gltf file
	/// @return The parsed GLTF file, or std::nullopt if an error occurred
	std::optional<GLTF> load(const std::filesystem::path& path);
}
