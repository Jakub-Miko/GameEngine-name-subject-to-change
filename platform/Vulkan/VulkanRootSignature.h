#pragma once
#include <Renderer/RootSignature.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

struct ExtraElementInfo {
	RootParameterType type;
	uint32_t table_binding; ///< which table
	uint32_t table_index; ///< which index in the table
};


/**
 * @brief This is basically the same as OpenGLRootSignature, but may change in the future so i duplicated it
 */
class VulkanRootSignature : public RootSignature {
public:
	friend RootSignature;
	int GetUniformBlockBindingId(const std::string& name) const ;
	int GetTextureSlot(const std::string& name) const;

	uint32_t GetTableBinding(const std::string& name) const;

private:
	virtual ~VulkanRootSignature() {}
	VkDescriptorSetLayout CreateDescriptorTableParams(const RootDescriptorTable& table, uint32_t table_id, const std::string& name);
	VulkanRootSignature(const RootSignatureDescriptor& descriptor);

private:
	std::unordered_map<std::string, ExtraElementInfo> parameters;
};
