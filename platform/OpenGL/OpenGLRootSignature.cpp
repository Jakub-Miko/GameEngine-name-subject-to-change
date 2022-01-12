#include "OpenGLRootSignature.h"
#include <stdexcept>

OpenGLRootSignature::OpenGLRootSignature(const RootSignatureDescriptor& descriptor) : parameters()
{
	int binding_id = 0;
	parameters.reserve(descriptor.parameters.size());
	for (auto desc : descriptor.parameters) {
		ExtraElementInfo info;
		info.type = desc.type;
		if (desc.type == RootParameterType::CONSTANT_BUFFER) {
			info.constant_binding_id = binding_id;
			binding_id++;
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
