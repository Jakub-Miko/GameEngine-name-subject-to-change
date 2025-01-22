#include "VulkanUnitConverter.h"

VulkanUnitConverter* VulkanUnitConverter::instance = nullptr;

VulkanUnitConverter::VulkanUnitConverter()
{
	DEFINE_VK_INSTANCE(context);
	VkPhysicalDevice device = context->GetVkbDevice().physical_device.physical_device;
	VkFormat depth_stencil_formats[] = {VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT};
	VkFormat depth_formats[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM };

	VkFormatProperties2 props = {};
	props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;

	for (VkFormat format : depth_stencil_formats) {
		vkGetPhysicalDeviceFormatProperties2(device, format, &props);
		if ((props.formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
			default_depth_stencil_format = format;
			break;
		}
	}

	for (VkFormat format : depth_formats) {
		vkGetPhysicalDeviceFormatProperties2(device, format, &props);
		if ((props.formatProperties.optimalTilingFeatures & VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
			default_depth_format = format;
			break;
		}
	}

}

void VulkanUnitConverter::Init() {
	if (!instance) {
		instance = new VulkanUnitConverter;
	}
}

void VulkanUnitConverter::Shutdown() {
	if (instance) {
		delete instance;
	}
}

VulkanUnitConverter* VulkanUnitConverter::Get() {
	return instance;
}