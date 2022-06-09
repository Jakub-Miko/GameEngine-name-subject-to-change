#include "RootSignature.h"
#include <platform/OpenGL/OpenGLRootSignature.h>
#include <platform/OpenGL/OpenGLUnitConverter.h>

RootMappingEntry RootSignature::GetRootMapping(const std::string& semantic_name)
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
	return new OpenGLRootSignature(descriptor);
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