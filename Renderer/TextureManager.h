#pragma once
#include <string>
#include <unordered_map>
#include <Renderer/RendererDefines.h>
#include <Renderer/RenderResource.h>
#include <AsyncTaskDispatcher.h>
#include <mutex>
#include <memory>

enum class Texture_status : char {
	UNINITIALIZED = 0, LOADING = 1, READY = 2
};

struct texture_data {
	unsigned char* tex_data = nullptr;
	int res_x = 0, res_y = 0, channels = 4;
	TextureSamplerDescritor sampler_desc;
	~texture_data() {
		delete[] tex_data;
	}

};

class TextureManager {
public:

	TextureManager(const TextureManager& ref) = delete;
	TextureManager(TextureManager&& ref) = delete;
	TextureManager& operator=(const TextureManager& ref) = delete;
	TextureManager& operator=(TextureManager&& ref) = delete;

	void MakeTextureFromImage(const std::string& input_image_path, const std::string& output_image_path, const TextureSamplerDescritor& sampler);
	Future<void> MakeTextureFromImageAsync(const std::string& input_image_path, const std::string& output_image_path, const TextureSamplerDescritor& sampler);


	std::shared_ptr<RenderTexture2DResource> LoadTextureFromFile(const std::string& file_path, bool generate_mips = false);

	Future<std::shared_ptr<RenderTexture2DResource>> LoadTextureFromFileAsync(const std::string& file_path, bool generate_mips);

	std::shared_ptr<RenderTexture2DResource> GetDefaultTexture() const {
		return default_texture;
	}

	std::shared_ptr<RenderTexture2DArrayResource> GetDefaultTextureArray() const {
		return default_texture_array;
	}

	std::shared_ptr<RenderTexture2DResource> GetDefaultNormalTexture() const {
		return default_normal_texture;
	}

	bool IsTextureAvailable(const std::string& file_path);

	void ReleaseTexture(const std::string& file_path);

	static void Init();

	static TextureManager* Get();

	static void Shutdown();
private:
	TextureManager();
	friend class World;
	void ClearTextureCache();


	std::shared_ptr<RenderTexture2DResource> default_texture;
	std::shared_ptr<RenderTexture2DArrayResource> default_texture_array;
	std::shared_ptr<RenderTexture2DResource> default_normal_texture;
	std::mutex texture_Map_mutex;
	std::unordered_map<std::string, std::shared_ptr<RenderTexture2DResource>> texture_Map;
	std::mutex sampler_cache_mutex;
	std::unordered_map<TextureSamplerDescritor, std::shared_ptr<TextureSampler>> sampler_cache;


	static TextureManager* instance;
};
