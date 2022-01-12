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
	for (auto element : layout) {
		stride += element.size * OpenGLUnitConverter::PrimitiveSize(element.type);
	}
}