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

std::vector<VkVertexInputAttributeDescription> GetVertexInputStateFromVertexLayout(const VertexLayout& layout) {
	std::vector<VkVertexInputAttributeDescription> attributes;
	attributes.reserve(layout.layout.size());
	
	int binding_num = 0;
	for (auto& element : layout.layout) {

		VkVertexInputAttributeDescription attrib;
		attrib.binding = 0;
		attrib.format = VulkanUnitConverter::PrimitiveAndSizeToVulkan(element.type, element.size);
		attrib.offset = element.offset;
		attrib.location = attrib.binding;
		attributes.push_back(attrib);
	}

	return std::move(attributes);
}

std::shared_ptr<Pipeline> VulkanPipelineManager::CreatePipeline(const PipelineDescriptor& desc)
{
	DEFINE_VK_INSTANCE(context);
	VkStencilOpState stencil_ops = {};
	VkPipelineLayout layout = static_cast<const VulkanRootSignature*>(&desc.GetSignature())->GetPipelineLayout();
	
	std::vector<VkVertexInputAttributeDescription> attributes = GetVertexInputStateFromVertexLayout(*desc.layout);

	VkVertexInputBindingDescription binding;
	binding.binding = 0;
	binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	binding.stride = desc.layout->stride;

	VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
	vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state.vertexBindingDescriptionCount = 1;
	vertex_input_state.pVertexBindingDescriptions = &binding;
	vertex_input_state.vertexAttributeDescriptionCount = attributes.size();
	vertex_input_state.pVertexAttributeDescriptions = attributes.data();

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
	info.pVertexInputState = &vertex_input_state; // Generate from Vertex layout
	info.layout = layout; 
	info.pDepthStencilState = &depth_stencil; 
	info.pDynamicState = &dynamic_state;
	info.pInputAssemblyState = &input_assembly;
	//info.pColorBlendState = ; // generate from framebuffer
	//info.pMultisampleState = ;
	//info.pRasterizationState = ;
	//info.pStages = ;
	//info.stageCount = ;

	return std::shared_ptr<Pipeline>();
}

VulkanPipelineManager::VulkanPipelineManager()
{
}
