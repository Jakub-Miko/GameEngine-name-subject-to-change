#include "TextRenderer.h"
#include <FreeType/freetype.h>
#include <AsyncTaskDispatcher.h>
#include <filesystem>
#include <FileManager.h>
#include <Renderer/Renderer.h>
#include <Renderer/TextureManager.h>
#include <Renderer/RenderResourceManager.h>
#include <Application.h>
#include <ft2build.h>
#include FT_FREETYPE_H


TextRenderer* TextRenderer::instance = nullptr; 

struct TextRenderer::Internal_data {
    FT_Library ft_lib;
    std::shared_ptr<TextureSampler> sampler;
};


void TextRenderer::Init()
{
	if (!instance) {
		instance = new TextRenderer;
	}
}

TextRenderer* TextRenderer::Get()
{
	return instance;
}


std::shared_ptr<FontObject> TextRenderer::GetFontObject(const std::string& in_path)
{
    std::string absolute_path = std::filesystem::absolute(std::filesystem::path(in_path)).generic_string();
    std::string relative_path = FileManager::Get()->GetRelativeFilePath(absolute_path);


    std::unique_lock<std::mutex> lock(m_font_object_map_mutex);
    auto fnd = m_font_object_map.find(relative_path); //TODO: Make sure the file path is in an appropriate format
    if (fnd != m_font_object_map.end()) {
        return fnd->second;
    }

    FontObject font;

    font.status = FontObject::Font_status::LOADING;
    font.glypth_data = std::vector<FontObject::GlyphData>();
    font.font_atlas = TextureManager::Get()->GetDefaultTexture();
    font.path = relative_path;

    auto font_final = RegisterFontObject(std::make_unique<FontObject>(font), relative_path);


    auto async_queue = Application::GetAsyncDispather();

    auto task = async_queue->CreateTask<font_load_future_payload>([absolute_path, this]() -> font_load_future_payload {
        return LoadFontFromFileImpl(absolute_path);
        });

    async_queue->Submit(task);

    font_load_future future;
    future.fence.reset();
    future.font = font_final;
    future.future_object = task->GetFuture();

    std::lock_guard<std::mutex> lock2(font_Load_queue_mutex);
    font_Load_queue.push_back(future);

    return font_final; // Handle return types differently 

}

void TextRenderer::UpdateLoadedFonts()
{
    std::lock_guard<std::mutex> lock(font_Load_queue_mutex);
    for (auto& loaded_font : font_Load_queue) {
        if (!loaded_font.future_object.IsAvailable() || loaded_font.destroyed) continue;
        
        try {
            if (!loaded_font.fence) {
                auto loaded_obj = loaded_font.future_object.GetValue();
                loaded_font.font->font_atlas = loaded_obj.object.font_atlas;
                loaded_font.font->glypth_data = std::move(loaded_obj.object.glypth_data);
                loaded_font.fence = loaded_obj.texture_atlas_fence;
            }
            if (loaded_font.fence->GetValue() == 1) {
                loaded_font.font->status = FontObject::Font_status::LOADED;
                loaded_font.destroyed = true;
            }
            

        }
        catch (...) {
            loaded_font.font->status = FontObject::Font_status::ERROR;
            std::lock_guard<std::mutex> lock(font_Load_queue_mutex);
            m_font_object_map.erase(loaded_font.font->path);
            loaded_font.destroyed = true;
        }
    }


    while (!font_Load_queue.empty() && font_Load_queue.front().destroyed) {
        font_Load_queue.pop_front();
    }

}

TextRenderer::font_load_future_payload TextRenderer::LoadFontFromFileImpl(const std::string& file_path)
{
    font_load_future_payload payload;
    payload.object = FontObject();
    payload.texture_atlas_fence = std::shared_ptr<RenderFence>(Renderer::Get()->GetFence());
    payload.object.glypth_data = std::vector<FontObject::GlyphData>();
    payload.object.glypth_data.reserve(128);
    RenderTexture2DDescriptor texture_desc;
    texture_desc.format = TextureFormat::R_UNSIGNED_CHAR;
    texture_desc.sampler = m_Internal_data->sampler;
    int width = std::ceil(std::sqrt(LOAD_FONT_SYMBOLS_COUNT));
    int height = std::ceil(LOAD_FONT_SYMBOLS_COUNT / width) +1;
    texture_desc.height = height * LOAD_FONT_SYMBOLS_HEIGHT;
    texture_desc.width = width * LOAD_FONT_SYMBOLS_HEIGHT;
    payload.object.font_atlas = RenderResourceManager::Get()->CreateTexture(texture_desc);
    FT_Face face;
    auto error = FT_New_Face(m_Internal_data->ft_lib, file_path.c_str(), 0, &face);
    if (error) {
        throw std::runtime_error("Could not opne font file: " + file_path);
    }
    int num_of_glyphs = std::min((int)face->num_glyphs, (int)LOAD_FONT_SYMBOLS_COUNT);

    int offset_x = 0;
    int offset_y = 0;
    auto list = Renderer::Get()->GetRenderCommandList();
    auto queue = Renderer::Get()->GetCommandQueue();
    for (int i = 0; i < num_of_glyphs; i++) {
        FT_Set_Pixel_Sizes(face, 0, LOAD_FONT_SYMBOLS_HEIGHT);
        error = FT_Load_Glyph(face, FT_Get_Char_Index(face, i), FT_LOAD_RENDER);
        
        if (error) {
            throw std::runtime_error("Character could not be loaded");
        }
        if (offset_x + LOAD_FONT_SYMBOLS_HEIGHT > texture_desc.width) {
            offset_x = 0;
            offset_y += LOAD_FONT_SYMBOLS_HEIGHT;
        }
        if (face->glyph->bitmap.buffer) {
            RenderResourceManager::Get()->UploadDataToTexture2D(list, payload.object.font_atlas, (void*)face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, offset_x, offset_y, 0);
        }

        payload.object.glypth_data.push_back(FontObject::GlyphData{ (char)i,offset_x,offset_y,face->glyph->advance.x >> 6,face->glyph->bitmap_left,
            face->glyph->bitmap_top, (int)face->glyph->bitmap.width ,(int)face->glyph->bitmap.rows });

        offset_x += LOAD_FONT_SYMBOLS_HEIGHT;
    }
    queue->ExecuteRenderCommandList(list);
    queue->Signal(payload.texture_atlas_fence, 1);

    return payload;
}

void TextRenderer::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

std::shared_ptr<FontObject> TextRenderer::RegisterFontObject(std::shared_ptr<FontObject> object, const std::string& name)
{
    auto font_final = m_font_object_map.insert(std::make_pair(name, object));
    return font_final.first->second;
}

TextRenderer::TextRenderer() : m_Internal_data(new Internal_data), m_font_object_map(), m_font_object_map_mutex()
{
    m_Internal_data->ft_lib = FT_Library();
    auto error = FT_Init_FreeType(&m_Internal_data->ft_lib);
    if (error) {
        throw std::runtime_error("FreeType could not be initialized");
    }
    TextureSamplerDescritor sampler_desc;
    m_Internal_data->sampler = TextureSampler::CreateSampler(sampler_desc);
        
}

TextRenderer::~TextRenderer()
{
    FT_Done_FreeType(m_Internal_data->ft_lib);
	delete m_Internal_data;
}
