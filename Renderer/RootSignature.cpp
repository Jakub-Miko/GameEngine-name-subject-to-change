#include "RootSignature.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <platform/OpenGL/OpenGLUnitConverter.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRootSignature.h>
#include <platform/Vulkan/VulkanUnitConverter.h>
#endif

RootMappingEntry RootSignature::GetRootMapping(const std::string& semantic_name) const 
{
	auto fnd = RootMappings.find(semantic_name);
	if (fnd != RootMappings.end()) {
		return fnd->second;
	}
	else {
		return RootMappingEntry();
	}
}

const ConstantBufferLayout& RootSignature::GetConstantBufferLayout(const std::string& semantic_name) const
{
	auto fnd = ConstantBufferLayouts.find(semantic_name);
	if (fnd != ConstantBufferLayouts.end()) {
		return fnd->second;
	}
	else {
		throw std::runtime_error("Constant Buffer Layout " + semantic_name + " not found");
	}
}

RootSignature* RootSignature::CreateSignature(const RootSignatureDescriptor& descriptor)
{
#ifdef OpenGL_API
	RootSignature* signature = new OpenGLRootSignature(descriptor);
#elif defined Vulkan_API
	RootSignature* signature = new VulkanRootSignature(descriptor);
#endif
	signature->descriptor = descriptor;
	return signature;
}

RootSignature* RootSignature::CreateSignature(const RootSignatureDescriptor& descriptor, RootMappingTable&& mapping_table, const ConstantBufferLayoutTable& const_buf_layouts)
{
#ifdef OpenGL_API
	RootSignature* signature = new OpenGLRootSignature(descriptor);
#elif defined Vulkan_API
	RootSignature* signature = new VulkanRootSignature(descriptor);
#endif
	signature->descriptor = descriptor;
	signature->RootMappings = std::move(mapping_table);
	signature->ConstantBufferLayouts = const_buf_layouts;
	return signature;
}

void VertexLayout::CalculateStride()
{
	for (auto& element : layout) {
		if (element.name.find("uv" + std::to_string(num_of_uv_channels)) != element.name.npos) {
			num_of_uv_channels += 1;
		}

		element.offset = stride;
#ifdef OpenGL_API
		stride += element.size * OpenGLUnitConverter::PrimitiveSize(element.type);
#elif defined Vulkan_API
		stride += element.size * VulkanUnitConverter::PrimitiveSize(element.type);
#endif
	}
}