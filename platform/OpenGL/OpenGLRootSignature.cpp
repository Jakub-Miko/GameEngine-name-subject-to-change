#include "OpenGLRootSignature.h"
#include <stdexcept>

OpenGLRootSignature::OpenGLRootSignature(const RootSignatureDescriptor& descriptor) : parameters()
{
	int binding_id = 0;
	int texture_slot = 0;
	parameters.reserve(descriptor.parameters.size());
	for (auto desc : descriptor.parameters) {
		ExtraElementInfo info;
		info.type = desc.type;
		if (desc.type == RootParameterType::CONSTANT_BUFFER) {
			info.constant_binding_id = binding_id;
			binding_id++;
		}

		if (desc.type == RootParameterType::TEXTURE_2D) {
			info.texture_slot = texture_slot;
			texture_slot++;
		}

		parameters.insert(std::make_pair(desc.name,info));
	}
}

int OpenGLRootSignature::GetUniformBlockBindingId(const std::string& name)
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		if (fnd->second.type == RootParameterType::CONSTANT_BUFFER) {
			return fnd->second.constant_binding_id;
		}
		else {
			throw std::runtime_error("Parameter isn't a constant buffer");
		}
	}
	else {
		throw std::runtime_error("Uniformblock with name " + name + " doesn't exist");
	}
}

int OpenGLRootSignature::GetTextureSlot(const std::string& name)
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		if (fnd->second.type == RootParameterType::TEXTURE_2D) {
			return fnd->second.texture_slot;
		}
		else {
			throw std::runtime_error("Parameter isn't a Texture 2D");
		}
	}
	else {
		throw std::runtime_error("Texture 2D with name " + name + " doesn't exist");
	}
}
