#include "VulkanRenderResourceStore.h"

#include "VulkanRenderContext.h"
#include "VulkanRenderResourceManager.h"
#include "VulkanUnitConverter.h"

VulkanRenderResourceStoreLayout::VulkanRenderResourceStoreLayout(const RootDescriptorType resource_descriptor_type) : RenderResourceStoreLayout(resource_descriptor_type) {
    DEFINE_VK_INSTANCE(context);

    auto& limits = context->GetBindlessLimits();

    uint64_t bindless_limit;

    switch(resource_descriptor_type) {
    case RootDescriptorType::CONSTANT_BUFFER:
        bindless_limit = limits.max_bindless_uniform_buffers;
        break;
    case RootDescriptorType::TEXTURE_2D:
    case RootDescriptorType::TEXTURE_2D_ARRAY:
    case RootDescriptorType::TEXTURE_2D_CUBEMAP:
        bindless_limit = limits.max_bindless_textures;
        break;
    case RootDescriptorType::STORAGE_BUFFER:
        bindless_limit = limits.max_bindless_storage_buffers;
        break;
    default:
        bindless_limit = 1024;
    }

    bindless_limit /= MAX_BINDLESS_DESCRIPTOR_SET_IN_USE;
    bindless_limit -= 30;

	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = 0;
	layout_binding.descriptorType = VulkanUnitConverter::DescriptorTypeToVkDescriptorType(resource_descriptor_type);
	layout_binding.descriptorCount = bindless_limit;
	layout_binding.stageFlags = VK_SHADER_STAGE_ALL;

	VkDescriptorBindingFlags binding_flags = VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
		| VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT
		| VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT
        | VkDescriptorBindingFlagBits::VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo layout_binding_flags = {};
	layout_binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	layout_binding_flags.bindingCount = 1;
	layout_binding_flags.pBindingFlags = &binding_flags;

	VkDescriptorSetLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &layout_binding;
	layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	layout_info.pNext = &layout_binding_flags;

	vkCreateDescriptorSetLayout(context->GetVkDevice(), &layout_info, nullptr, &descriptor_set_layout);
}

VulkanRenderResourceStoreLayout::~VulkanRenderResourceStoreLayout() {
    DEFINE_VK_INSTANCE(context);
    vkDestroyDescriptorSetLayout(context->GetVkDevice(), descriptor_set_layout, NULL);
}

bool VulkanRenderResourceStore::Destroy() {
    DEFINE_VK_INSTANCE(context)
    vkDestroyDescriptorPool(context->GetVkDevice(), descriptor_pool, NULL);
    descriptor_pool = VK_NULL_HANDLE;
    descriptor_set = VK_NULL_HANDLE;
    return true;
}

uint32_t VulkanRenderResourceStore::AttachResource(std::shared_ptr<RenderResource> resource) {
    DEFINE_VK_INSTANCE(context);
    auto vk_res =  static_cast<VulkanRenderResource*>(resource->GetExtensionData());
    auto& descriptor = GetDescriptor();
    if(resource->GetResourceStore()) {
        auto existing_binding = std::static_pointer_cast<VulkanRenderResourceStore>(resource->GetResourceStore());
        if(existing_binding.get() == this) {
            throw std::runtime_error("Resource is already attached to a different store.");
        } else {
            return resource->GetResourceStoreIndex();
        }
    }

    uint32_t binding_index;

    if(resource->GetRenderState() == RenderState::UNINITIALIZED) {
        if(IsReadOnly()) {
            throw std::runtime_error("Resources attached to a read-only store must be initialized.");
        } else {
            resource->SetRenderState(vk_res->GetDefaultState()); // we never know when a resource in a store will be written to, so we consider it initialized.
        }
    }

    if(!free_indices.empty()) {
        binding_index = free_indices.back();
        free_indices.pop_back();
    } else {
        if(last_allocated_binding >= descriptor.max_resource_count) {
            throw std::runtime_error("No more resource bindings available in this store.");
        }
        binding_index = last_allocated_binding++;
    }

    resource_bindings[binding_index] = resource;

    VulkanRenderResourceStoreAttachment new_attachment;
    new_attachment.store = shared_from_this();
    new_attachment.binding_index = binding_index;
    vk_res->SetStoreAttachment(new_attachment);

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptor_set;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.dstArrayElement = binding_index;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VulkanUnitConverter::DescriptorTypeToVkDescriptorType(descriptor.resource_descriptor_type);

    VkDescriptorBufferInfo buffer_info = {};
    VkDescriptorImageInfo image_info = {};

    switch(resource->GetResourceType()) {
    case RenderResourceType::RenderBufferResource:
    {
        auto buffer = std::static_pointer_cast<VulkanRenderBufferResource>(resource);
        buffer_info.buffer = buffer->GetBuffer();
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        writeDescriptorSet.pBufferInfo = &buffer_info;
        break;
    }
    case RenderResourceType::RenderTexture2DResource:
    case RenderResourceType::RenderTexture2DArrayResource:
    case RenderResourceType::RenderTexture2DCubemapResource:
    {
        auto texture = dynamic_cast<VulkanRenderTextureResource*>(vk_res);
        if(vk_res->GetDefaultState() != descriptor.default_image_resource_state) {
            throw std::runtime_error("Default resource state does not match default store descriptor state.");
        }
        image_info.imageView = texture->GetImageView();
        image_info.sampler = std::static_pointer_cast<VulkanTextureSampler>(texture->GetSampler())->GetSampler();
        image_info.imageLayout = VulkanUnitConverter::RenderStateToTextureLayout(descriptor.default_image_resource_state);
        writeDescriptorSet.pImageInfo = &image_info;
        break;
    }
    default:
        throw std::runtime_error("Unsupported resource type.");
    }

    vkUpdateDescriptorSets(context->GetVkDevice(), 1, &writeDescriptorSet, 0, NULL);

    return binding_index;
}

bool VulkanRenderResourceStore::DeattachResource(std::shared_ptr<RenderResource> resource) {
    auto vk_res =  static_cast<VulkanRenderResource*>(resource->GetExtensionData());
    if(resource->GetResourceStore()) {
        throw std::runtime_error("Resource is not attached a resource store.");
    }

    auto store_binding = std::static_pointer_cast<VulkanRenderResourceStore>(resource->GetResourceStore());

    if(store_binding.get() != this) {
        throw std::runtime_error("Resource is not attached to this store.");
    }

    free_indices.push_back(resource->GetResourceStoreIndex());
    resource_bindings.erase(resource->GetResourceStoreIndex());
    vk_res->ResetStoreAttachment();
    return true;
}

bool VulkanRenderResourceStore::IsReadOnly() {
    auto& descriptor = GetDescriptor();
    switch(descriptor.resource_descriptor_type) {
    case RootDescriptorType::CONSTANT_BUFFER:
    case RootDescriptorType::STORAGE_BUFFER:
        return descriptor.buffer_is_read_only;
        break;
    case RootDescriptorType::TEXTURE_2D:
    case RootDescriptorType::TEXTURE_2D_ARRAY:
    case RootDescriptorType::TEXTURE_2D_CUBEMAP:
        return true;
        break;
    default:
        throw std::runtime_error("Unsupported resource descriptor type.");
    }
}
