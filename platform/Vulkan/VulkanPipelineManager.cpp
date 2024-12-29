#include "VulkanPipelineManager.h"
#include <vulkan/vulkan.h>
#include "VulkanUnitConverter.h"
#include "VulkanRootSignature.h"

RootBinding VulkanPipeline::GetBindingId(const std::string& name)
{
	return RootBinding();
}

VulkanPipeline::~VulkanPipeline()
{
}

void VulkanPipeline::SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer)
{
}

void VulkanPipeline::SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer)
{
}

void VulkanPipeline::SetTexture2D(RootBinding binding_id, std::shared_ptr<RenderTexture2DResource> buffer)
{
}

void VulkanPipeline::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> buffer)
{
}

void VulkanPipeline::SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> buffer)
{
}

void VulkanPipeline::SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> buffer)
{
}

void VulkanPipeline::BeginVertexContext(std::shared_ptr<RenderBufferResource> vertex_buffer)
{
}

void VulkanPipeline::EndVertexContext()
{
}

void VulkanPipeline::SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table)
{
}

VulkanPipeline::VulkanPipeline(const PipelineDescriptor& desc) : Pipeline(desc)
{
}

VulkanPipeline::VulkanPipeline(PipelineDescriptor&& desc) : Pipeline(std::move(desc))
{
}

VkDescriptorSetLayout GetVkDescriptorSetLayoutFromRootDescriptorTable(const RootDescriptorTable& table) {
	DEFINE_VK_INSTANCE(context);
	VkDescriptorSetLayout layout;

	VkDescriptorSetLayoutCreateInfo set_layout = {};
	set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.reserve(table.size());
	int binding_id = 0;
	for (auto& binding : table) {
		VkDescriptorSetLayoutBinding vk_binding = {};
		vk_binding.descriptorCount = binding.size;
		vk_binding.descriptorType = VulkanUnitConverter::DescriptorTypeToVkDescriptorType(binding.type);
		vk_binding.binding = binding_id++;
		bindings.push_back(vk_binding);
	}

	set_layout.bindingCount = table.size();
	set_layout.pBindings = bindings.data();
	vkCreateDescriptorSetLayout(context->GetVkDevice(), &set_layout, NULL, &layout);

	return layout;
}

VkPipelineLayout GetPipelineLayoutInfo(const PipelineDescriptor& desc)
{
	DEFINE_VK_INSTANCE(context);
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	const VulkanRootSignature* signature = static_cast<const VulkanRootSignature*>(&desc.GetSignature());

	RootDescriptorTable global_table = {};
	int global_textures = 0;
	int global_textures_array = 0;
	int global_textures_cubemaps = 0;
	int global_buffers = 0;

	std::vector<VkDescriptorSetLayout> table_layouts;
	for (auto& element : signature->GetDescriptor().parameters) {
		switch (element.type)
		{
		case RootParameterType::CONSTANT_BUFFER:
			global_buffers++;
			break;
		case RootParameterType::TEXTURE_2D:
			global_textures++;
			break;
		case RootParameterType::TEXTURE_2D_ARRAY:
			global_textures_array++;
			break;
		case RootParameterType::TEXTURE_2D_CUBEMAP:
			global_textures_cubemaps++;
			break;
		case RootParameterType::DESCRIPTOR_TABLE:
			table_layouts.push_back(GetVkDescriptorSetLayoutFromRootDescriptorTable(element.table));
			break;
		default:
			break;
		}
	}
	if(global_buffers > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::CONSTANT_BUFFER, global_buffers, "global"));

	if (global_textures > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D, global_textures, "global"));
	
	if (global_textures_array > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D_ARRAY, global_textures_array, "global"));

	if (global_textures_cubemaps > 0)
		global_table.push_back(RootDescriptorTableRange(RootDescriptorType::TEXTURE_2D_CUBEMAP, global_textures_cubemaps, "global"));

	if (global_table.size() > 0) {
		table_layouts.push_back(GetVkDescriptorSetLayoutFromRootDescriptorTable(global_table));
	}

	info.setLayoutCount = table_layouts.size();
	info.pSetLayouts = table_layouts.data();

	VkPipelineLayout layout;
	vkCreatePipelineLayout(context->GetVkDevice(), &info, NULL, &layout);

	return layout;
}

std::shared_ptr<Pipeline> VulkanPipelineManager::CreatePipeline(const PipelineDescriptor& desc)
{
	DEFINE_VK_INSTANCE(context);
	VkStencilOpState stencil_ops = {};

	VkPipelineLayout layout = GetPipelineLayoutInfo(desc);

	VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = true;
	depth_stencil.depthWriteEnable = true;
	depth_stencil.depthCompareOp = VulkanUnitConverter::DepthFunctionToVulkanCompareFunc(desc.depth_function);
	depth_stencil.back = stencil_ops;
	depth_stencil.front = stencil_ops;
	depth_stencil.stencilTestEnable = false;
	depth_stencil.flags = NULL;

	VkDynamicState dyn_states[] = {VkDynamicState::VK_DYNAMIC_STATE_SCISSOR, VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT};

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO; 
	dynamic_state.pDynamicStates = dyn_states;
	dynamic_state.dynamicStateCount = 2;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.primitiveRestartEnable = false;
	input_assembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	//info.pColorBlendState = ; // generate from framebuffer
	//info.pVertexInputState = ; // Generate from Vertex layout
	info.layout = layout; // generate from root signature
	info.pDepthStencilState = &depth_stencil; 
	info.pDynamicState = &dynamic_state;
	info.pInputAssemblyState = &input_assembly;
	//info.pMultisampleState = ;
	//info.pRasterizationState = ;
	//info.pStages = ;
	//info.stageCount = ;

	return std::shared_ptr<Pipeline>();
}

VulkanPipelineManager::VulkanPipelineManager()
{
}
