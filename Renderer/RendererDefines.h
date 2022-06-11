#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>
#include <stdexcept>
enum class RenderState : unsigned char
{
	UNINITIALIZED = 0, READ = 1, WRITE = 2, COMMON = 3, EMPTY = 4, IN_USE_VERTEX_BUFFER = 5, IN_USE_INDEX_BUFFER = 6
};

enum class RenderBufferType : unsigned char
{
	DEFAULT = 0, UPLOAD = 1
};

enum class RenderBufferUsage : unsigned char
{
	VERTEX_BUFFER = 0, INDEX_BUFFER = 1, CONSTANT_BUFFER = 2
};

enum class PipelineFlags : uint32_t {
	ENABLE_DEPTH_TEST = (1 << 0),
	ENABLE_STENCIL_TEST = (1 << 1),
	ENABLE_SCISSOR_TEST = (1 << 2),
	ENABLE_BLEND = (1 << 3),
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

enum class BlendFunction : uint32_t {
	ZERO = 0, ONE = 1, SRC_COLOR = 2, ONE_MINUS_SRC_COLOR = 3, DST_COLOR= 4, ONE_MINUS_DST_COLOR = 5, SRC_ALPHA = 6, ONE_MINUS_SRC_ALPHA = 7, DST_ALPHA = 8, ONE_MINUS_DST_ALPHA = 9
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
    FLOAT = 0, INT = 1, UNSIGNED_INT = 2, CHAR = 3, UNSIGNED_CHAR = 4, UNKNOWN = 5
};

enum class TextureFormat : unsigned char {
	RGBA_UNSIGNED_CHAR = 0, RGB_UNSIGNED_CHAR = 1, DEPTH24_STENCIL8_UNSIGNED_CHAR = 2
};

enum class TextureAddressMode : unsigned char {
	WRAP = 0, MIRROR = 1, CLAMP = 2, BORDER = 3, MIRROR_ONCE = 4
};

enum class TextureFilter : unsigned char {
	POINT_MIN_MAG_MIP = 0, POINT_MIN_MAG_LINEAR_MIP = 1, LINEAR_MIN_MAG_MIP = 2 , LINEAR_MIN_MAG_POINT_MIP = 3, ANISOTROPIC = 4
};

enum class RenderQueueTypes : unsigned char
{
    DirectQueue = 0, ComputeQueue = 1, CopyQueue = 2
};

using RootBinding = unsigned int;

enum class RootParameterType : unsigned char {
	UNDEFINED = 0, CONSTANT_BUFFER = 1, TEXTURE_2D = 2, DESCRIPTOR_TABLE = 3
};

enum class RootDescriptorType : unsigned char {
	CONSTANT_BUFFER = 0, TEXTURE_2D = 1
};

struct RootMappingEntry {
	RootMappingEntry() : binding_id(0), type(RootParameterType::UNDEFINED) {}
	RootMappingEntry(RootBinding binding_id, RootParameterType type) : binding_id(binding_id), type(type) {}
	RootBinding binding_id;
	RootParameterType type;
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

	VertexLayout(const VertexLayout& layout) : layout(layout.layout), stride(layout.stride) {

	}

	VertexLayout(VertexLayout&& layout) noexcept : layout(std::move(layout.layout)), stride(layout.stride) {

	}

	bool has_normal() const {
		for (auto& element : layout) {
			if (element.name == "normal") {
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