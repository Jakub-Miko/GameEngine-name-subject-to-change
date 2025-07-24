#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <stdexcept>
enum class RenderState : unsigned char
{
	UNINITIALIZED = 0, READ = 1, WRITE = 2, COMMON = 3, EMPTY = 4, IN_USE_VERTEX_BUFFER = 5, IN_USE_INDEX_BUFFER = 6, 
	TEXTURE_COLOR_ATTACHMENT = 7, TEXTURE_TRANSFER_SRC = 8, TEXTURE_TRANSFER_DST = 9, TEXTURE_SAMPLE = 10, TEXTURE_GENERAL = 11, TEXTURE_DEPTH_STENCIL_ATTACHMENT = 12,
	TEXTURE_DEPTH_SAMPLE = 13
};

enum class RenderBufferType : unsigned char
{
	DEFAULT = 0, UPLOAD = 1
};

enum class RenderBufferUsage : unsigned char
{
	VERTEX_BUFFER = 0, INDEX_BUFFER = 1, CONSTANT_BUFFER = 2, STAGING = 3, VERTEX_BUFFER_READABLE = 4, INDEX_BUFFER_READABLE = 5
};

enum class TextureUsage : unsigned char
{
	DEFAULT = 0,
	COLOR_ATTACHMENT = (1 << 0),
	SAMPLE = (1 << 1),
	STORAGE = (1 << 2),
	TRANSFER = (1 << 3),
	DEPTH_ATTACHMENT = (1 << 4),
	COPYABLE = (1 << 5),

	//Shortcuts
	SAMPLE_WRITABLE = SAMPLE | TRANSFER,
	COLOR_ATTACHMENT_READABLE = COLOR_ATTACHMENT | SAMPLE,
	DEPTH_ATTACHMENT_READABLE = DEPTH_ATTACHMENT | SAMPLE,

	COLOR_ATTACHMENT_WRITABLE = COLOR_ATTACHMENT | TRANSFER,

	STORAGE_READABLE = STORAGE | SAMPLE,
	STORAGE_WRITABLE = STORAGE | TRANSFER,
	STORAGE_READABLE_WRITABLE = STORAGE | SAMPLE | TRANSFER

};

inline TextureUsage operator|(const TextureUsage& flags_1, const TextureUsage& flags_2) {
	return (TextureUsage)((unsigned char)flags_1 | (unsigned char)flags_2);
}

inline TextureUsage operator&(const TextureUsage& flags_1, const TextureUsage& flags_2) {
	return (TextureUsage)((unsigned char)flags_1 & (unsigned char)flags_2);
}

enum class PipelineStage : char {
	ALL_STAGES, FRAGMENT_SHADER, VERTEX_SHADER, GEOMETRY_SHADER, COMPUTE_SHADER, HOST, CLEAR, 
};

enum class PipelineFlags : uint32_t {
	ENABLE_DEPTH_TEST = (1 << 0),
	ENABLE_STENCIL_TEST = (1 << 1),
	ENABLE_SCISSOR_TEST = (1 << 2),
	ENABLE_BLEND = (1 << 3),
	IS_MULTI_WINDOW = (1 << 4),
	DEFAULT = 0
};

enum class PrimitivePolygonRenderMode : unsigned char {
	DEFAULT = 0, WIREFRAME = 1
};

inline PipelineFlags operator|(const PipelineFlags& flags_1, const PipelineFlags& flags_2) {
	return (PipelineFlags)((uint32_t)flags_1 | (uint32_t)flags_2);
}

inline PipelineFlags operator&(const PipelineFlags& flags_1, const PipelineFlags& flags_2) {
	return (PipelineFlags)((uint32_t)flags_1 & (uint32_t)flags_2);
}

enum class DepthFunction : char {
	NEVER = 0, LESS = 1, EQUAL = 2, LESS_EQUAL = 3, GREATER = 4, NOT_EQUAL = 5, GREATER_EQUAL = 6, ALWAYS = 7
};

enum class BlendFunction : uint32_t {
	ZERO = 0, ONE = 1, SRC_COLOR = 2, ONE_MINUS_SRC_COLOR = 3, DST_COLOR= 4, ONE_MINUS_DST_COLOR = 5, SRC_ALPHA = 6, ONE_MINUS_SRC_ALPHA = 7, DST_ALPHA = 8, ONE_MINUS_DST_ALPHA = 9
};

enum class BlendEquation : char {
	ADD = 0, SUBTRACT = 1, REVERSE_SUBTRACT = 2, MIN = 3, MAX = 4
};

enum class CullMode : char {
	NONE = 0, FRONT = 1, BACK = 2
};

enum class CubemapFace : char {
	POSITIVE_X = 0 ,NEGATIVE_X = 1, POSITIVE_Y = 2, NEGATIVE_Y = 3, POSITIVE_Z = 4, NEGATIVE_Z = 5
};

struct RenderViewport {
	RenderViewport(glm::vec2 offset = {0,0}, glm::vec2 size = { -1,-1 }, float min_depth = 0, float max_depth = 1) 
		: offset(offset), size(size), min_depth(min_depth), max_depth(max_depth) {}

	bool operator==(const RenderViewport& other) const {
		return (offset == other.offset && size == other.size && min_depth == other.min_depth && max_depth == other.max_depth);
	}

	bool operator!=(const RenderViewport& other) const {
		return !this->operator==(other);
	}

	glm::vec2 offset;
	glm::vec2 size;
	float min_depth;
	float max_depth;
};

struct RenderScissorRect {
	RenderScissorRect(glm::vec2 offset = { 0,0 }, glm::vec2 size = {-1,-1}) : offset(offset), size(size) {}


	bool operator==(const RenderScissorRect& other) const {
		return (offset == other.offset && size == other.size);
	}

	bool operator!=(const RenderScissorRect& other) const {
		return !this->operator==(other);
	}

	glm::vec2 offset;
	glm::vec2 size;
};


enum class RenderPrimitiveType : unsigned char
{
    FLOAT = 0, INT = 1, UNSIGNED_INT = 2, CHAR = 3, UNSIGNED_CHAR = 4, VEC2 = 5, VEC3 = 6, VEC4 = 7, MAT3 = 8, MAT4 = 9, UNKNOWN = 10
};

enum class TextureFormat : unsigned char {
	RGBA_UNSIGNED_CHAR = 0, RGB_UNSIGNED_CHAR = 1, DEPTH24_STENCIL8_UNSIGNED_CHAR = 2, RGB_32FLOAT = 3, RGBA_32FLOAT = 4, R_UNSIGNED_INT = 5, 
	R_UNSIGNED_CHAR = 6, R_8FLOAT = 7, R_UNSIGNED_CHAR_NORM = 8, DEFAULT_DEPTH = 9, DEFAULT_DEPTH_STENCIL = 10, BGRA_SRGB = 11, UNDEFINED = 12
};

enum class TextureAddressMode : unsigned char {
	WRAP = 0, MIRROR = 1, CLAMP = 2, BORDER = 3, MIRROR_ONCE = 4
};

enum class TextureFilter : unsigned char {
	POINT_MIN_MAG = 0, LINEAR_MIN_MAG = 1, POINT_MIN_MAG_MIP = 2, POINT_MIN_MAG_LINEAR_MIP = 3, LINEAR_MIN_MAG_MIP = 4 , LINEAR_MIN_MAG_POINT_MIP = 5
};

enum class DepthComparisonMode : char {
	DISABLED = 0, NEVER = 1, LESS = 2, EQUAL = 3, LESS_EQUAL = 4, GREATER = 5, NOT_EQUAL = 6, GREATER_EQUAL = 7, ALWAYS = 8
};

enum class RenderQueueTypes : unsigned char
{
    DirectQueue = 0, ComputeQueue = 1, CopyQueue = 2
};

using RootBinding = unsigned int;

enum class RootParameterType : unsigned char {
	UNDEFINED = 0, CONSTANT_BUFFER = 1, TEXTURE_2D = 2, MATERIAL = 3, TEXTURE_2D_ARRAY = 4, TEXTURE_2D_CUBEMAP = 5
};

enum class RootDescriptorType : unsigned char {
	CONSTANT_BUFFER = 0, TEXTURE_2D = 1, TEXTURE_2D_ARRAY = 2, TEXTURE_2D_CUBEMAP = 3
};

struct VertexLayoutElement {
	VertexLayoutElement() = default;
	VertexLayoutElement(RenderPrimitiveType type, uint32_t size, const std::string& name, bool normalized = false) : type(type), size(size), name(name), normalized(normalized){}
	bool normalized = false;
	std::string name = "Undefined";
	int offset = -1;
	RenderPrimitiveType type = RenderPrimitiveType::UNKNOWN;
	uint32_t size = 0;
};

struct VertexLayout {

	VertexLayout() : layout(), stride(0) {}

	VertexLayout(const std::vector<VertexLayoutElement>& layout) : layout(layout), stride(0) {
		CalculateStride();
	}

	VertexLayout(std::vector<VertexLayoutElement>&& layout) : layout(std::move(layout)), stride(0) {
		CalculateStride();
	}

	VertexLayout(const VertexLayout& layout) : layout(layout.layout), stride(layout.stride), num_of_uv_channels(layout.num_of_uv_channels) {

	}

	VertexLayout(VertexLayout&& layout) noexcept : layout(std::move(layout.layout)), stride(layout.stride), num_of_uv_channels(layout.num_of_uv_channels) {

	}

	~VertexLayout() {

	}

	bool has_normal() const {
		for (auto& element : layout) {
			if (element.name == "normal") {
				return true;
			}
		}
		return false;
	}

	bool has_tangent() const {
		for (auto& element : layout) {
			if (element.name == "tangent") {
				return true;
			}
		}
		return false;
	}

	bool has_position() const {
		for (auto& element : layout) {
			if (element.name == "position") {
				return true;
			}
		}
		return false;
	}

	bool has_uv() const {
		return num_of_uv_channels != 0;
	}

	int GetUvCount() const {
		return num_of_uv_channels;
	}
	
	const VertexLayoutElement& GetElement(const std::string& name) const {
		for (auto& element : layout) {
			if (element.name == name) {
				return element;
			}
		}
		throw std::runtime_error("Incompatible VertexLayout");
	}

	VertexLayout& operator=(const VertexLayout& ref) {
		layout = ref.layout;
		stride = ref.stride;
		return*this;
	}

	VertexLayout& operator=(VertexLayout&& ref) noexcept {
		layout = std::move(ref.layout);
		stride = ref.stride;
		return *this;
	}

	std::vector<VertexLayoutElement> layout;
	int stride;
	int num_of_uv_channels = 0;
private:

	void CalculateStride();

};