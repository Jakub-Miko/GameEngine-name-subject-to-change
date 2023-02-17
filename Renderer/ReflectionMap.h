#pragma once
#include <Renderer/RenderResource.h>
#include <vector>
#include <Renderer/RenderDescriptorHeap.h>

enum class ReflectionMapStatus : char {
	UNINITIALIZED = 0, LOADING = 1, LOADED = 2, ERROR = 3
};

class ReflectionMap {
public:
	std::shared_ptr<RenderTexture2DCubemapResource> GetDiffuseMap() const {
		return diffuse_map;
	}

	RenderDescriptorTable GetDescriptorTable() const {
		return descriptor_table;
	}

	const std::vector<std::pair<float, std::shared_ptr<RenderTexture2DCubemapResource>>>& GetSpecularMaps() const {
		return specular_maps;
	}

	ReflectionMapStatus GetStatus() const {
		return status;
	}

private:
	friend class TextureManager;
	ReflectionMapStatus status = ReflectionMapStatus::UNINITIALIZED;
	std::shared_ptr<RenderTexture2DCubemapResource> diffuse_map;
	std::vector<std::pair<float, std::shared_ptr<RenderTexture2DCubemapResource>>> specular_maps;
	RenderDescriptorTable descriptor_table;
};