#include "VulkanContext.h"

VulkanContext* VulkanContext::instance = nullptr;

void VulkanContext::Create()
{
	if (!instance) {
		instance = new VulkanContext;
	}
}

void VulkanContext::Initialize()
{
}

VulkanContext* VulkanContext::Get()
{
	return instance;
}

void VulkanContext::Shutdown()
{
	if (instance) {
		delete instance;
		instance = nullptr;
	}
}

void VulkanContext::RequestExtension(const std::string& extension)
{
	auto fnd = std::find(requested_extensions.begin(), requested_extensions.end(), extension);
	if (fnd == requested_extensions.end()) {
		requested_extensions.push_back(extension);
	}
}

void VulkanContext::RequestExtensions(const char** extensions, int count)
{
	for (int i = 0; i < count; i++) {
		RequestExtension(std::string(extensions[i]));
	}
}

std::shared_ptr<const char*> VulkanContext::GetExtensions()
{
	const char** extensions = (const char**)malloc(sizeof(char*) * requested_extensions.size());

	int i = 0;
	for (auto& string : requested_extensions) {
		extensions[i++] = string.c_str();
	}

	return std::shared_ptr<const char*>(extensions, free);
}

VulkanContext::VulkanContext() : requested_extensions()
{
	requested_extensions.reserve(10);
}

VulkanContext::~VulkanContext()
{
}
