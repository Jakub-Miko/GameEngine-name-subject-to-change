#include "RootSignature.h"

#ifdef OpenGL_API
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <platform/OpenGL/OpenGLUnitConverter.h>
#elif defined Vulkan_API
#include <platform/Vulkan/VulkanRootSignature.h>
#include <platform/Vulkan/VulkanUnitConverter.h>
#endif

RootSignature* RootSignature::CreateSignature(const RootSignatureDescriptor& descriptor)
{
#ifdef OpenGL_API
	RootSignature* signature = new OpenGLRootSignature(descriptor);
#elif defined Vulkan_API
	RootSignature* signature = new VulkanRootSignature(descriptor);
#endif
	return signature;
}

RootSignature* RootSignature::CreateSignature(const RootSignatureDescriptor& descriptor, RootMappingTable&& mapping_table)
{
#ifdef OpenGL_API
	RootSignature* signature = new OpenGLRootSignature(descriptor);
#elif defined Vulkan_API
	RootSignature* signature = new VulkanRootSignature(descriptor);
#endif
	signature->RootMappings = std::move(mapping_table);
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

RootMappingEntry RootSignature::GetRootParameterId(const std::string& semantic_name) const
{
	auto fnd = RootMappings.find(semantic_name);
	if (fnd != RootMappings.end()) {
		return RootMappingEntry( fnd->second.parameter_id );
	}
	else {
		throw std::runtime_error("Root parameter " + semantic_name + "doesn't exist.\n");
	}
}

const RootSignatureDescriptorElement& RootSignature::GetRootParameter(const std::string& semantic_name) const
{
	auto fnd = RootMappings.find(semantic_name);
	if (fnd != RootMappings.end()) {
		return descriptor.parameters[fnd->second.parameter_id];
	}
	else {
		throw std::runtime_error("Root parameter " + semantic_name + "doesn't exist.\n");
	}
}