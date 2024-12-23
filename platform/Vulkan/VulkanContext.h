#pragma once
#include <vector>
#include <string>
#include <memory>

class VulkanContext {
public:

	static void Create();
	static void Initialize();
	static VulkanContext* Get();
	static void Shutdown();

	void RequestExtension(const std::string& extension);
	void RequestExtensions(const char** extensions, int count);
	std::shared_ptr<const char*> GetExtensions();

private:
	VulkanContext();
	std::vector<std::string> requested_extensions;
	virtual ~VulkanContext();
	static VulkanContext* instance;

};