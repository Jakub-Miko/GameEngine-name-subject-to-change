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

	std::shared_ptr<RenderTexture2DCubemapResource> GetSpecularMap() const {
		return specular_map;
	}

	ReflectionMapStatus GetStatus() const {
		return status;
	}

private:
	friend class TextureManager;
	ReflectionMapStatus status = ReflectionMapStatus::UNINITIALIZED;
	std::shared_ptr<RenderTexture2DCubemapResource> diffuse_map;
	std::shared_ptr<RenderTexture2DCubemapResource> specular_map;
};