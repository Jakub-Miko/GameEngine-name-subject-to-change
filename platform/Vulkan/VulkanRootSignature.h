#pragma once
#include <Renderer/RootSignature.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>

struct VulkanDescriptorBinding {
	RootParameterType type;
	uint32_t table_binding; ///< which table
	uint32_t table_index = -1; ///< which index in the table(-1 if type is table itself)
	uint32_t array_index = -1; ///< which index in descriptor array(-1 if not array)
};


/**
 * @brief This is basically the same as OpenGLRootSignature, but may change in the future so i duplicated it
 */
class VulkanRootSignature : public RootSignature {
public:
	friend RootSignature;
	VulkanDescriptorBinding GetDescriptorBinding(const std::string& name) const;
	VkPipelineLayout GetPipelineLayout() const { return layout; }
private:
	virtual ~VulkanRootSignature();
	VkDescriptorSetLayout CreateDescriptorTableParams(const RootDescriptorTable& table, uint32_t table_id, const std::string& name);
	VulkanRootSignature(const RootSignatureDescriptor& descriptor);


private:
	std::unordered_map<std::string, VulkanDescriptorBinding> parameters;
	std::vector<VkDescriptorSetLayout> table_layouts;
	VkPipelineLayout layout;
};
