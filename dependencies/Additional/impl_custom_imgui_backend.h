#pragma once
#include <Renderer/RendererDefines.h>
#include <Renderer/RenderResourceManager.h>
#include <dependencies/imgui/imgui.h>
#include <Core/FrameMultiBufferResource.h>

class impl_custom_imgui_backend {
	struct backend_data;
public:
	static void Init();
	static void Shutdown();
	static void NewFrame();
	static backend_data* GetBackendData() {
		return current_backend_data;
	}

	static void PreShutdown();

	static void DrawData(ImDrawData* draw_data);
	
	static std::shared_ptr<RenderBufferResource> UploadDataDynamicSize(std::shared_ptr<RenderCommandList>  command_list, std::shared_ptr<RenderBufferResource> existing_buffer, void* data, 
		size_t size, size_t offset, bool allow_invalidation = false);
	
	static void CreateObjects();
private:

	static bool objects_init;
	struct backend_data {
		std::shared_ptr<Shader> shader = nullptr;
		std::shared_ptr<Pipeline> pipeline = nullptr;
		FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> vertex_buffer;
		FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> index_buffer;
		FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>> constant_buffer;
		std::shared_ptr<RenderTexture2DResource> atlas_texture = nullptr;
	};

	static backend_data* current_backend_data;

};
