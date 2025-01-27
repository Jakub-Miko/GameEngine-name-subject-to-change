#pragma once
#include <Core/UnitConverter.h>
#include <Renderer/RendererDefines.h>
#include <stdexcept>
#include <Vulkan/vulkan.h>
#include "VulkanRenderContext.h"
#include "VulkanRenderCommandList.h"
#include "VulkanShaderManager.h"
#include "shaderc/shaderc.hpp"

class VulkanUnitConverter {
private:
	static VulkanUnitConverter* instance;
	VulkanUnitConverter();

private:
	VkFormat default_depth_format = VkFormat::VK_FORMAT_D32_SFLOAT;
	VkFormat default_depth_stencil_format = VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
	VkFormat default_color_format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;

public:
	static void Init();
	static void Shutdown();
	static VulkanUnitConverter* Get();

	static VkFormat PrimitiveToVulkan(RenderPrimitiveType type) {
		switch (type) {
		case RenderPrimitiveType::CHAR:				return VK_FORMAT_R8_SINT;
		case RenderPrimitiveType::FLOAT:			return VK_FORMAT_R32_SFLOAT;
		case RenderPrimitiveType::INT:				return VK_FORMAT_R32_SINT;
		case RenderPrimitiveType::UNSIGNED_CHAR:	return VK_FORMAT_R8_UINT;
		case RenderPrimitiveType::UNSIGNED_INT:		return VK_FORMAT_R32_UINT;
		case RenderPrimitiveType::VEC2:				return VK_FORMAT_R32_SFLOAT;
		case RenderPrimitiveType::VEC3:				return VK_FORMAT_R32_SFLOAT;
		case RenderPrimitiveType::VEC4:				return VK_FORMAT_R32_SFLOAT;
		default: 
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkAccessFlagBits2 DependencyToVkAccess(VulkanCommandListDependencyType dependency) {
		switch (dependency) {
		case VulkanCommandListDependencyType::READ:				return VK_ACCESS_2_MEMORY_READ_BIT;
		case VulkanCommandListDependencyType::WRITE:			return VK_ACCESS_2_MEMORY_WRITE_BIT;
		case VulkanCommandListDependencyType::INVALID:			return VK_ACCESS_2_NONE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkFormat PrimitiveAndSizeToVulkan(RenderPrimitiveType type, int size, bool normalized = false) {

		switch (size)
		{
		case 1:
			switch (type) {
			case RenderPrimitiveType::CHAR:				return VK_FORMAT_R8_SINT;
			case RenderPrimitiveType::FLOAT:			return VK_FORMAT_R32_SFLOAT;
			case RenderPrimitiveType::INT:				return VK_FORMAT_R32_SINT;
			case RenderPrimitiveType::UNSIGNED_CHAR:	return normalized ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_UINT;
			case RenderPrimitiveType::UNSIGNED_INT:		return VK_FORMAT_R32_UINT;
			default:
				throw std::runtime_error("Conversion failed");
			}
			break;
		case 2:
			switch (type) {
			case RenderPrimitiveType::CHAR:				return VK_FORMAT_R8G8_SINT;
			case RenderPrimitiveType::FLOAT:			return VK_FORMAT_R32G32_SFLOAT;
			case RenderPrimitiveType::INT:				return VK_FORMAT_R32G32_SINT;
			case RenderPrimitiveType::UNSIGNED_CHAR:	return normalized ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8G8_UINT;
			case RenderPrimitiveType::UNSIGNED_INT:		return VK_FORMAT_R32G32_UINT;
			default:
				throw std::runtime_error("Conversion failed");
			}
			break;
		case 3:
			switch (type) {
			case RenderPrimitiveType::CHAR:				return VK_FORMAT_R8G8B8_SINT;
			case RenderPrimitiveType::FLOAT:			return VK_FORMAT_R32G32B32_SFLOAT;
			case RenderPrimitiveType::INT:				return VK_FORMAT_R32G32B32_SINT;
			case RenderPrimitiveType::UNSIGNED_CHAR:	return normalized ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8_UINT;;
			case RenderPrimitiveType::UNSIGNED_INT:		return VK_FORMAT_R32G32B32_UINT;
			default:
				throw std::runtime_error("Conversion failed");
			}
			break;
		case 4:
			switch (type) {
			case RenderPrimitiveType::CHAR:				return VK_FORMAT_R8G8B8A8_SINT;
			case RenderPrimitiveType::FLOAT:			return VK_FORMAT_R32G32B32A32_SFLOAT;
			case RenderPrimitiveType::INT:				return VK_FORMAT_R32G32B32A32_SINT;
			case RenderPrimitiveType::UNSIGNED_CHAR:	return normalized ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UINT;
			case RenderPrimitiveType::UNSIGNED_INT:		return VK_FORMAT_R32G32B32A32_UINT;
			default:
				throw std::runtime_error("Conversion failed");
			}
			break;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static bool IsPrimitiveInteger(RenderPrimitiveType type) {
		switch (type) {
		case RenderPrimitiveType::CHAR:				return true;
		case RenderPrimitiveType::FLOAT:			return false;
		case RenderPrimitiveType::INT:				return true;
		case RenderPrimitiveType::UNSIGNED_CHAR:	return true;
		case RenderPrimitiveType::UNSIGNED_INT:		return true;
		case RenderPrimitiveType::VEC2:				return false;
		case RenderPrimitiveType::VEC3:				return false;
		case RenderPrimitiveType::VEC4:				return false;
		case RenderPrimitiveType::MAT3:				return false;
		case RenderPrimitiveType::MAT4:				return false;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static int PrimitiveSize(RenderPrimitiveType type) {
		return UnitConverter::PrimitiveSize(type);
	}

	static VkFormat TextureFormatToVulkanInternalformat(TextureFormat type, bool normalized = true) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return normalized ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UINT;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return normalized ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UINT; //vulkan doesnt support RGB only RGBA for allignment reasons
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return VK_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::RGB_32FLOAT:					return VK_FORMAT_R32G32B32A32_SFLOAT;
		case TextureFormat::RGBA_32FLOAT:					return VK_FORMAT_R32G32B32A32_SFLOAT;
		case TextureFormat::R_UNSIGNED_INT:					return VK_FORMAT_R32_UINT;
		case TextureFormat::R_UNSIGNED_CHAR:                return normalized ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_UINT;
		case TextureFormat::R_UNSIGNED_CHAR_NORM:           return VK_FORMAT_R8_UNORM;
		case TextureFormat::R_8FLOAT:						return VK_FORMAT_R8_UNORM;
		case TextureFormat::UNDEFINED:						return VK_FORMAT_UNDEFINED;
		case TextureFormat::DEFAULT_DEPTH:					return instance->default_depth_format;
		case TextureFormat::DEFAULT_DEPTH_STENCIL:			return instance->default_depth_stencil_format;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static bool IsTextureFormatDepth(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return false;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return false;
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return true;
		case TextureFormat::RGB_32FLOAT:					return false;
		case TextureFormat::RGBA_32FLOAT:					return false;
		case TextureFormat::R_UNSIGNED_INT:					return false;
		case TextureFormat::R_UNSIGNED_CHAR:                return false;
		case TextureFormat::R_UNSIGNED_CHAR_NORM:           return false;
		case TextureFormat::R_8FLOAT:						return false;
		case TextureFormat::UNDEFINED:						return false;
		case TextureFormat::DEFAULT_DEPTH:					return true;
		case TextureFormat::DEFAULT_DEPTH_STENCIL:			return true;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static TextureUsage TextureFormatToVulkanDefaultImageUsage(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::RGB_UNSIGNED_CHAR:				return TextureUsage::SAMPLE_WRITABLE; //vulkan doesnt support RGB only RGBA for allignment reasons
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return TextureUsage::DEPTH_ATTACHMENT;
		case TextureFormat::RGB_32FLOAT:					return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::RGBA_32FLOAT:					return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::R_UNSIGNED_INT:					return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::R_UNSIGNED_CHAR:                return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::R_UNSIGNED_CHAR_NORM:           return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::R_8FLOAT:						return TextureUsage::SAMPLE_WRITABLE;
		case TextureFormat::DEFAULT_DEPTH:					return TextureUsage::DEPTH_ATTACHMENT;
		case TextureFormat::DEFAULT_DEPTH_STENCIL:			return TextureUsage::DEPTH_ATTACHMENT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkImageLayout RenderStateToTextureLayout(RenderState type) {
		switch (type) {
		case RenderState::TEXTURE_COLOR_ATTACHMENT:				return VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case RenderState::TEXTURE_SAMPLE:						return VkImageLayout::VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		case RenderState::TEXTURE_GENERAL:						return VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
		case RenderState::TEXTURE_TRANSFER_DST:					return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case RenderState::TEXTURE_TRANSFER_SRC:					return VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case RenderState::TEXTURE_DEPTH_STENCIL_ATTACHMENT:		return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case RenderState::TEXTURE_DEPTH_SAMPLE:					return VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		case RenderState::UNINITIALIZED:						return VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkPipelineStageFlagBits2 PipelineStageToVulkanPipelineStage(PipelineStage stage) {
		switch (stage)
		{
		case PipelineStage::ALL_STAGES:							return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		case PipelineStage::FRAGMENT_SHADER:					return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		case PipelineStage::VERTEX_SHADER:						return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
		case PipelineStage::GEOMETRY_SHADER:					return VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
		case PipelineStage::COMPUTE_SHADER:						return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		case PipelineStage::HOST:								return VK_PIPELINE_STAGE_2_HOST_BIT;
		case PipelineStage::CLEAR:								return VK_PIPELINE_STAGE_2_CLEAR_BIT;
		default:
			throw std::runtime_error("Conversion failed.");
		}
	}

	static VkCompareOp  DepthFunctionToVulkanCompareFunc(DepthFunction mode) {
		switch (mode) {
		case DepthFunction::ALWAYS:					return VK_COMPARE_OP_ALWAYS;
		case DepthFunction::EQUAL:					return VK_COMPARE_OP_EQUAL;
		case DepthFunction::GREATER:				return VK_COMPARE_OP_GREATER;
		case DepthFunction::GREATER_EQUAL:			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case DepthFunction::LESS:					return VK_COMPARE_OP_LESS;
		case DepthFunction::LESS_EQUAL:				return VK_COMPARE_OP_LESS_OR_EQUAL;
		case DepthFunction::NEVER:					return VK_COMPARE_OP_NEVER;
		case DepthFunction::NOT_EQUAL:				return VK_COMPARE_OP_NOT_EQUAL;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkCompareOp DepthComparisonModeToVulkanCompareFunc(DepthComparisonMode mode) {
		switch (mode) {
		case DepthComparisonMode::ALWAYS:					return VK_COMPARE_OP_ALWAYS;
		case DepthComparisonMode::EQUAL:					return VK_COMPARE_OP_EQUAL;
		case DepthComparisonMode::GREATER:				return VK_COMPARE_OP_GREATER;
		case DepthComparisonMode::GREATER_EQUAL:			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case DepthComparisonMode::LESS:					return VK_COMPARE_OP_LESS;
		case DepthComparisonMode::LESS_EQUAL:				return VK_COMPARE_OP_LESS_OR_EQUAL;
		case DepthComparisonMode::NEVER:					return VK_COMPARE_OP_NEVER;
		case DepthComparisonMode::NOT_EQUAL:				return VK_COMPARE_OP_NOT_EQUAL;
		case DepthComparisonMode::DISABLED:				return VK_COMPARE_OP_NEVER;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkBlendOp BlendEquationToVulkanEnum(BlendEquation type) {
		switch (type) {
		case BlendEquation::ADD:				return VK_BLEND_OP_ADD;
		case BlendEquation::MAX:				return VK_BLEND_OP_MAX;
		case BlendEquation::MIN:				return VK_BLEND_OP_MIN;
		case BlendEquation::REVERSE_SUBTRACT:	return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendEquation::SUBTRACT:			return VK_BLEND_OP_SUBTRACT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkCullModeFlagBits CullModeTOVulkanFlags(CullMode type) {
		switch (type) {
		case CullMode::BACK:				return VK_CULL_MODE_BACK_BIT;
		case CullMode::FRONT:				return VK_CULL_MODE_FRONT_BIT;
		case CullMode::NONE:				return VK_CULL_MODE_NONE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static int TextureFormatToTexelSize(TextureFormat type) {
		switch (type) {
		case TextureFormat::RGBA_UNSIGNED_CHAR:				return 4*sizeof(unsigned char);
		case TextureFormat::RGB_UNSIGNED_CHAR:				return 3*sizeof(unsigned char);
		case TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR:	return 4*sizeof(unsigned char);
		case TextureFormat::RGB_32FLOAT:					return 3*sizeof(float);
		case TextureFormat::RGBA_32FLOAT:					return 4*sizeof(float);
		case TextureFormat::R_UNSIGNED_INT:					return sizeof(uint32_t);
		case TextureFormat::R_UNSIGNED_CHAR:                return sizeof(char);
		case TextureFormat::R_UNSIGNED_CHAR_NORM:           return sizeof(char);
		case TextureFormat::R_8FLOAT:						return sizeof(char);
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkSamplerAddressMode TextureAddressModeToVulkanAddressMode(TextureAddressMode mode) {
		switch (mode) {
		case TextureAddressMode::BORDER:				return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case TextureAddressMode::CLAMP:					return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case TextureAddressMode::MIRROR:				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case TextureAddressMode::WRAP:					return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkFilter TextureFilterToMinMagFilter(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::LINEAR_MIN_MAG:					return VK_FILTER_LINEAR;
		case TextureFilter::POINT_MIN_MAG:					return VK_FILTER_NEAREST;
		case TextureFilter::LINEAR_MIN_MAG_MIP:				return VK_FILTER_LINEAR;
		case TextureFilter::LINEAR_MIN_MAG_POINT_MIP:		return VK_FILTER_LINEAR;
		case TextureFilter::POINT_MIN_MAG_LINEAR_MIP:		return VK_FILTER_NEAREST;
		case TextureFilter::POINT_MIN_MAG_MIP:				return VK_FILTER_NEAREST ;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkSamplerMipmapMode  TextureFilterToMipFilter(TextureFilter filter) {
		switch (filter) {
		case TextureFilter::LINEAR_MIN_MAG:					return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case TextureFilter::POINT_MIN_MAG:					return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case TextureFilter::LINEAR_MIN_MAG_MIP:				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case TextureFilter::LINEAR_MIN_MAG_POINT_MIP:		return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case TextureFilter::POINT_MIN_MAG_LINEAR_MIP:		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		case TextureFilter::POINT_MIN_MAG_MIP:				return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkPolygonMode  PrimitivePolygonRenderModetoVulkanEnum(PrimitivePolygonRenderMode mode) {
		switch (mode) {
		case PrimitivePolygonRenderMode::DEFAULT:				return VK_POLYGON_MODE_FILL;
		case PrimitivePolygonRenderMode::WIREFRAME:				return VK_POLYGON_MODE_LINE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkBlendFactor BlendFunctiontoVkBlendFactor(BlendFunction mode) {
		switch (mode) {
		case BlendFunction::ONE:					return VK_BLEND_FACTOR_ONE;
		case BlendFunction::ZERO:					return VK_BLEND_FACTOR_ZERO;
		case BlendFunction::DST_ALPHA:				return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFunction::SRC_ALPHA:				return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFunction::ONE_MINUS_DST_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFunction::ONE_MINUS_SRC_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFunction::SRC_COLOR:				return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFunction::DST_COLOR:				return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFunction::ONE_MINUS_SRC_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFunction::ONE_MINUS_DST_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkBufferUsageFlags BufferUsageToVkFlags(RenderBufferUsage mode) {
		switch (mode) {
		case RenderBufferUsage::CONSTANT_BUFFER:			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case RenderBufferUsage::VERTEX_BUFFER:			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case RenderBufferUsage::INDEX_BUFFER:			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case RenderBufferUsage::STAGING:			return VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkImageUsageFlags TextureUsageToVkTextureUsage(TextureUsage usage) {
		switch (usage) {
		case TextureUsage::COLOR_ATTACHMENT:			return VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::COLOR_ATTACHMENT_READABLE:	return VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::COLOR_ATTACHMENT_WRITABLE:	return VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::DEPTH_ATTACHMENT:			return VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::DEPTH_ATTACHMENT_READABLE:	return VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::SAMPLE:						return VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
		case TextureUsage::SAMPLE_WRITABLE:				return VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::TRANSFER:					return VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::STORAGE:						return VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
		case TextureUsage::STORAGE_READABLE:			return VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
		case TextureUsage::STORAGE_WRITABLE:			return VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::STORAGE_READABLE_WRITABLE:	return VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VmaMemoryUsage BufferTypeToVmaUsage(RenderBufferType mode) {
		switch (mode) {
		case RenderBufferType::UPLOAD:				return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		case RenderBufferType::DEFAULT:				return VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VmaAllocationCreateFlags BufferTypeToVmaFlags(RenderBufferType mode) {
		switch (mode) {
		case RenderBufferType::UPLOAD:				return VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		case RenderBufferType::DEFAULT:				return NULL;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static shaderc_shader_kind ShaderStageToShadercShaderKind(VulkanShaderStages mode) {
		switch (mode) {
		case VulkanShaderStages::FRAGMENT:		return shaderc_shader_kind::shaderc_fragment_shader;
		case VulkanShaderStages::VERTEX:		return shaderc_shader_kind::shaderc_vertex_shader;
		case VulkanShaderStages::GEOMETRY:		return shaderc_shader_kind::shaderc_geometry_shader;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkShaderStageFlagBits ShaderStageToVkShaderStage(VulkanShaderStages mode) {
		switch (mode) {
		case VulkanShaderStages::FRAGMENT:		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		case VulkanShaderStages::VERTEX:		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		case VulkanShaderStages::GEOMETRY:		return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static VkDescriptorType DescriptorTypeToVkDescriptorType(RootDescriptorType type) {
		switch (type) {
		case RootDescriptorType::CONSTANT_BUFFER:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case RootDescriptorType::TEXTURE_2D:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case RootDescriptorType::TEXTURE_2D_ARRAY:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		case RootDescriptorType::TEXTURE_2D_CUBEMAP:		return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

	static RootParameterType DescriptorTypeToVkRootParameterType(RootDescriptorType type) {
		switch (type) {
		case RootDescriptorType::CONSTANT_BUFFER:		return RootParameterType::CONSTANT_BUFFER;
		case RootDescriptorType::TEXTURE_2D:		return RootParameterType::TEXTURE_2D;
		case RootDescriptorType::TEXTURE_2D_ARRAY:		return RootParameterType::TEXTURE_2D_ARRAY;
		case RootDescriptorType::TEXTURE_2D_CUBEMAP:		return RootParameterType::TEXTURE_2D_CUBEMAP;
		default:
			throw std::runtime_error("Conversion failed");
		}
	}

};

