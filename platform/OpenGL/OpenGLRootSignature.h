#pragma once
#include <Renderer/RootSignature.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

struct ExtraElementInfo {
	RootParameterType type;
	union {
		int constant_binding_id;
		int texture_slot;
		int table_descriptor_id;
	};
};

class OpenGLRootSignature : public RootSignature {
public:
	struct RootDescriptorTableBinding {
		RootDescriptorTable table;
		unsigned int starting_binding_id;
		unsigned int starting_texture_id;
	};
	friend RootSignature;
	int GetUniformBlockBindingId(const std::string& name) const ;
	int GetTextureSlot(const std::string& name) const;

	const RootDescriptorTableBinding& GetTableBinding(const std::string& name) const;

private:
	virtual ~OpenGLRootSignature() {}
	OpenGLRootSignature(const RootSignatureDescriptor& descriptor);

	void CreateDescriptorTableParams(const RootSignatureDescriptorElement& element, int& binding_id, int& texture_id);

private:
	std::unordered_map<std::string, ExtraElementInfo> parameters;
	std::vector<std::unique_ptr<RootDescriptorTableBinding>> descriptor_tables;
};
