#include "VulkanPipelineManager.h"
#include <vulkan/vulkan.h>
#include "VulkanUnitConverter.h"
#include "VulkanRootSignature.h"
#include "VulkanShaderManager.h"

RootBinding VulkanPipeline::GetBindingId(const std::string& name)
{
	return RootBinding();
}

VulkanPipeline::~VulkanPipeline()
{
}

VulkanPipeline::VulkanPipeline(const PipelineDescriptor& desc, VkPipeline pipeline) : Pipeline(desc), pipeline(pipeline)
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
		attrib.location = binding_num++;
		attributes.push_back(attrib);
	}

	return attributes;
}

std::vector<VkPipelineColorBlendAttachmentState> GetBlendState(const PipelineDescriptor& desc) {	
	std::vector<VkPipelineColorBlendAttachmentState> blend_attachments;
	blend_attachments.reserve(desc.framebuffer_format.color_attachemt_formats.size());

	VkPipelineColorBlendAttachmentState global_attachment;
	global_attachment.colorBlendOp = VulkanUnitConverter::BlendEquationToVulkanEnum(desc.blend_equation);
	global_attachment.alphaBlendOp = VulkanUnitConverter::BlendEquationToVulkanEnum(desc.blend_equation);
	global_attachment.blendEnable = (bool)(desc.flags & PipelineFlags::ENABLE_BLEND);
	global_attachment.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
		VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
	global_attachment.dstAlphaBlendFactor = VulkanUnitConverter::BlendFunctiontoVkBlendFactor(desc.blend_functions.dstAlpha);
	global_attachment.srcAlphaBlendFactor = VulkanUnitConverter::BlendFunctiontoVkBlendFactor(desc.blend_functions.srcAlpha);
	global_attachment.dstColorBlendFactor = VulkanUnitConverter::BlendFunctiontoVkBlendFactor(desc.blend_functions.dstRGB);
	global_attachment.srcColorBlendFactor = VulkanUnitConverter::BlendFunctiontoVkBlendFactor(desc.blend_functions.srcRGB);

	for (auto& attachment : desc.framebuffer_format.color_attachemt_formats) {
		VkPipelineColorBlendAttachmentState col_attachment = global_attachment;
		blend_attachments.push_back(col_attachment);
	}

	return blend_attachments;

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

	VkDynamicState dyn_states[] = { VkDynamicState::VK_DYNAMIC_STATE_SCISSOR, VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT };

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pDynamicStates = dyn_states;
	dynamic_state.dynamicStateCount = 2;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.primitiveRestartEnable = false;
	input_assembly.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineMultisampleStateCreateInfo multi_sample_info = {};
	multi_sample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multi_sample_info.alphaToCoverageEnable = false;
	multi_sample_info.alphaToOneEnable = false;
	multi_sample_info.sampleShadingEnable = false;
	multi_sample_info.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

	VkPipelineRasterizationStateCreateInfo raster_info = {};
	raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_info.cullMode = VulkanUnitConverter::CullModeTOVulkanFlags(desc.cull_mode);
	raster_info.depthClampEnable = false;
	raster_info.depthBiasEnable = false;
	raster_info.lineWidth = 1;
	raster_info.frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
	raster_info.rasterizerDiscardEnable = false;
	raster_info.polygonMode = VulkanUnitConverter::PrimitivePolygonRenderModetoVulkanEnum(desc.polygon_render_mode);

	VulkanShader* shader = static_cast<VulkanShader*>(desc.shader.get());

	std::vector<VkPipelineShaderStageCreateInfo> stages;
	int i = 0;
	for (auto& stage : shader->GetStages()) {
		if (stage.defined) {
			VkPipelineShaderStageCreateInfo stage_info = {};
			stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage_info.pName = "main";
			stage_info.stage = VulkanUnitConverter::ShaderStageToVkShaderStage((VulkanShaderStages)i);
			stage_info.module = stage.stage;
			stages.push_back(stage_info);
		}
		i++;
	}

	std::vector<VkPipelineColorBlendAttachmentState> blend_state_attachments = GetBlendState(desc);

	VkPipelineColorBlendStateCreateInfo blend_state;
	blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend_state.attachmentCount = blend_state_attachments.size();
	blend_state.pAttachments = blend_state_attachments.data();
	blend_state.logicOpEnable = false;


	VkPipelineRenderingCreateInfo dynamic_render_info = {};
	dynamic_render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	dynamic_render_info.colorAttachmentCount = desc.framebuffer_format.color_attachemt_formats.size();

	std::vector<VkFormat> framebuffer_formats;
	framebuffer_formats.reserve(dynamic_render_info.colorAttachmentCount);
	for (auto& format : desc.framebuffer_format.color_attachemt_formats) {
		framebuffer_formats.push_back(VulkanUnitConverter::TextureFormatToVulkanInternalformat(format.format));
	}

	dynamic_render_info.pColorAttachmentFormats = framebuffer_formats.data();
	dynamic_render_info.depthAttachmentFormat = VulkanUnitConverter::TextureFormatToVulkanInternalformat(desc.framebuffer_format.depth_attachemt_format.format);

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.pVertexInputState = &vertex_input_state;
	info.layout = layout; 
	info.pDepthStencilState = &depth_stencil; 
	info.pViewportState = &viewport_state;
	info.pDynamicState = &dynamic_state;
	info.pInputAssemblyState = &input_assembly;
	info.pMultisampleState = &multi_sample_info;
	info.pRasterizationState = &raster_info;
	info.stageCount = stages.size();
	info.pStages = stages.data();
	info.pColorBlendState = &blend_state;
	info.pNext = &dynamic_render_info;

	VkPipeline pipeline;

	vkCreateGraphicsPipelines(context->GetVkDevice(), VK_NULL_HANDLE, 1, &info, NULL, &pipeline);

	VulkanPipeline* new_pipeline = new VulkanPipeline(desc, pipeline);

	return std::shared_ptr<Pipeline>(new_pipeline);
}

VulkanPipelineManager::VulkanPipelineManager()
{
}
