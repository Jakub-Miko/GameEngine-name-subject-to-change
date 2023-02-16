#pragma once
#include <Renderer/RenderResource.h>
#include <vector>
#include <Renderer/RenderDescriptorHeap.h>


class ReflectionMap {
	
	std::shared_ptr<RenderTexture2DCubemapResource> GetDiffuseMap() const {
		return diffuse_map;
	}

	RenderDescriptorTable GetDescriptorTable() const {
		return descriptor_table;
	}

	const std::vector<std::pair<float, std::shared_ptr<RenderTexture2DCubemapResource>>>& GetSpecularMaps() const {
		return specular_maps;
	}


private:
	friend class TextureManager;
	std::shared_ptr<RenderTexture2DCubemapResource> diffuse_map;
	std::vector<std::pair<float, std::shared_ptr<RenderTexture2DCubemapResource>>> specular_maps;
	RenderDescriptorTable descriptor_table;
};