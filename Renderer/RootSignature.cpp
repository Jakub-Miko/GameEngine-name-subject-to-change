#include "RootSignature.h"
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <platform/OpenGL/OpenGLUnitConverter.h>

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

RootSignature* RootSignature::CreateSignature(const RootSignatureDescriptor& descriptor)
{
	RootSignature* signature = new OpenGLRootSignature(descriptor);
	signature->descriptor = descriptor;
	return signature;
}

RootSignature* RootSignature::CreateSignature(const RootSignatureDescriptor& descriptor, RootMappingTable&& mapping_table)
{
	RootSignature* signature = new OpenGLRootSignature(descriptor);
	signature->descriptor = descriptor;
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
		stride += element.size * OpenGLUnitConverter::PrimitiveSize(element.type);
	}
}