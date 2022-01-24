#pragma once
#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>

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
	VertexLayoutElement(RenderPrimitiveType type, uint32_t size) : type(type), size(size) {}
	RenderPrimitiveType type;
	uint32_t size;
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
private:

	void CalculateStride();

};