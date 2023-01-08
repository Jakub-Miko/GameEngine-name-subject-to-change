#pragma once
#include <Renderer/RendererDefines.h>
#include <Promise.h>
#include <Renderer/RenderFence.h>
#include <Renderer/RenderResource.h>
#include <deque>
#include <unordered_map>
#include <vector>
#include <mutex>

#define LOAD_FONT_SYMBOLS_COUNT 128
#define LOAD_FONT_SYMBOLS_HEIGHT 96

class FontObject {
public:
	enum class Font_status : char {
		UNINITIALIZED = 0, LOADING = 1, LOADED = 2, ERROR = 3
	};

	struct GlyphData {
		char character;
		int x, y;
		int advance;
		int offset_x, offset_y;
		int width, height;
	};
	
	std::shared_ptr<RenderTexture2DResource> GetTextureAtlas() const {
		return font_atlas;
	}

	Font_status GetStatus() const {
		return status;
	}

	const GlyphData& GetGlyphData(char character) const {
		int index = (int)character;
		if (index >= glypth_data.size()) {
			return glypth_data[0];
		}
		return glypth_data[index];
	}

private:
	friend class TextRenderer;
	std::string path;
	Font_status status = Font_status::UNINITIALIZED;
	std::shared_ptr<RenderTexture2DResource> font_atlas;
	std::vector<GlyphData> glypth_data;
};

class TextRenderer {
public:

	TextRenderer(const TextRenderer& ref) = delete;
	TextRenderer(TextRenderer&& ref) = delete;
	TextRenderer& operator=(const TextRenderer& ref) = delete;
	TextRenderer& operator=(TextRenderer&& ref) = delete;

	std::shared_ptr<FontObject> GetFontObject(const std::string& path);

	static void Init();

	static TextRenderer* Get();

	void TextRenderSystem();

	void UpdateLoadedFonts();

	static void Shutdown();
	
private:
	struct Internal_data;
	
	struct font_load_future_payload {
		FontObject object;
		std::shared_ptr<RenderFence> texture_atlas_fence;
	};

	struct font_load_future {
		std::shared_ptr<FontObject> font;
		Future<font_load_future_payload> future_object;
		std::shared_ptr<RenderFence> fence = nullptr;
		bool destroyed = false;
	};

	std::shared_ptr<FontObject> RegisterFontObject(std::shared_ptr<FontObject> object, const std::string& name);

	font_load_future_payload LoadFontFromFileImpl(const std::string& file_path);

	TextRenderer();
	~TextRenderer();
	static TextRenderer* instance;

	Internal_data* m_Internal_data;
	std::unordered_map<std::string, std::shared_ptr<FontObject>> m_font_object_map;
	std::mutex m_font_object_map_mutex;

	std::mutex font_Load_queue_mutex;
	std::deque<font_load_future> font_Load_queue;

};