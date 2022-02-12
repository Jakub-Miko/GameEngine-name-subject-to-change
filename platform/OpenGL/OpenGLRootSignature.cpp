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
			parameters.insert(std::make_pair(desc.name, info));
		}

		if (desc.type == RootParameterType::TEXTURE_2D) {
			info.texture_slot = texture_slot;
			texture_slot++;
			parameters.insert(std::make_pair(desc.name, info));
		}

		if (desc.type == RootParameterType::DESCRIPTOR_TABLE) {
			CreateDescriptorTableParams(desc,binding_id, texture_slot);
		}
	}
}

void OpenGLRootSignature::CreateDescriptorTableParams(const RootSignatureDescriptorElement& element, int& binding_id, int& texture_id)
{
	RootDescriptorTableBinding* binding = new RootDescriptorTableBinding();
	binding->table = element.table;
	binding->starting_binding_id = binding_id;
	binding->starting_texture_id = texture_id;
	ExtraElementInfo info;
	descriptor_tables.emplace_back(binding);
	info.type = RootParameterType::DESCRIPTOR_TABLE;
	info.table_descriptor_id = descriptor_tables.size() - 1;
	parameters.insert(std::make_pair(element.name, info));

	for (auto range : element.table) {
		if (range.type == RootDescriptorType::CONSTANT_BUFFER) {
			binding_id += range.size;
		}
		else if (range.type == RootDescriptorType::TEXTURE_2D) {
			texture_id += range.size;
		}
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

const OpenGLRootSignature::RootDescriptorTableBinding& OpenGLRootSignature::GetTableBinding(const std::string& name)
{
	auto fnd = parameters.find(name);
	if (fnd != parameters.end()) {
		if (fnd->second.type == RootParameterType::DESCRIPTOR_TABLE) {
			return *descriptor_tables[fnd->second.table_descriptor_id];
		}
		else {
			throw std::runtime_error("Parameter isn't a Descriptor Table");
		}
	}
	else {
		throw std::runtime_error("Descriptor Table with name " + name + " doesn't exist");
	}
}
