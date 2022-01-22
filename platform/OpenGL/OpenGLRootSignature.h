#pragma once
#include <Renderer/RootSignature.h>
#include <unordered_map>
#include <string>

struct ExtraElementInfo {
	RootParameterType type;
	union {
		int constant_binding_id;
		int texture_slot;
	};
};

class OpenGLRootSignature : public RootSignature {
public:
	friend RootSignature;
	int GetUniformBlockBindingId(const std::string& name);
	int GetTextureSlot(const std::string& name);

private:
	virtual ~OpenGLRootSignature() {}
	OpenGLRootSignature(const RootSignatureDescriptor& descriptor);


private:
	std::unordered_map<std::string, ExtraElementInfo> parameters;

};
