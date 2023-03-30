#include "TextRenderer.h"
#include <freetype/freetype.h>
#include <AsyncTaskDispatcher.h>
#include <filesystem>
#include <FileManager.h>
#include <Renderer/Renderer.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/TextureManager.h>
#include <Renderer/RenderResourceManager.h>
#include <World/World.h>
#include <World/Components/UITextComponent.h>
#include <Application.h>
#include <Window.h>
#include <Renderer/RootSignature.h>
#include <ft2build.h>
#include FT_FREETYPE_H


TextRenderer* TextRenderer::instance = nullptr; 


struct TextPreset {};

template<>
struct VertexLayoutFactory<TextPreset> {

    static VertexLayout* GetLayout() {
        static std::unique_ptr<VertexLayout> layout = nullptr;
        if (!layout) {
            VertexLayout* layout_new = new VertexLayout({
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "position"),
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2, "uv0")
                });


            layout = std::unique_ptr<VertexLayout>(layout_new);
        }
        return layout.get();
    }

};

struct TextRenderer::Internal_data {
    FT_Library ft_lib;
    std::shared_ptr<TextureSampler> sampler;

    std::shared_ptr<RenderBufferResource> const_buf;
    std::shared_ptr<Pipeline> pipeline;

};


TextRenderer::TextRenderer() : m_Internal_data(new Internal_data), m_font_object_map(), m_font_object_map_mutex()
{
    m_Internal_data->ft_lib = FT_Library();
    auto error = FT_Init_FreeType(&m_Internal_data->ft_lib);
    if (error) {
        throw std::runtime_error("FreeType could not be initialized");
    }
    TextureSamplerDescritor sampler_desc;
    m_Internal_data->sampler = TextureSampler::CreateSampler(sampler_desc);
    PipelineDescriptor pipeline_desc;
    pipeline_desc.blend_equation = BlendEquation::ADD;
    pipeline_desc.blend_functions = PipelineBlendFunctions{BlendFunction::SRC_ALPHA, BlendFunction::ONE_MINUS_SRC_ALPHA, BlendFunction::ONE , BlendFunction::ZERO};
    pipeline_desc.cull_mode = CullMode::NONE;
    pipeline_desc.depth_function = DepthFunction::ALWAYS;
    pipeline_desc.enable_depth_clip = false;
    pipeline_desc.flags = PipelineFlags::ENABLE_BLEND;
    pipeline_desc.layout = VertexLayoutFactory<TextPreset>::GetLayout();
    pipeline_desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
    pipeline_desc.scissor_rect = RenderScissorRect();
    pipeline_desc.viewport = RenderViewport();
    pipeline_desc.shader = ShaderManager::Get()->GetShader("shaders/TextShader.glsl");

    m_Internal_data->pipeline = PipelineManager::Get()->CreatePipeline(pipeline_desc);

    RenderBufferDescriptor const_buf_desc(sizeof(glm::mat4) + sizeof(float), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
    m_Internal_data->const_buf = RenderResourceManager::Get()->CreateBuffer(const_buf_desc);



}

struct TextVertex {
    glm::vec2 position;
    glm::vec2 uvs;
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

void TextRenderer::TextRenderSystem()
{
    auto ui_view = Application::GetWorld().GetRegistry().view<UITextComponent>();
    auto list = Renderer::Get()->GetRenderCommandList();
    auto queue = Renderer::Get()->GetCommandQueue();
    list->SetPipeline(m_Internal_data->pipeline);
    list->SetConstantBuffer("conf", m_Internal_data->const_buf);
    list->SetDefaultRenderTarget();

    for (auto entity : ui_view) {
        Entity ent = Entity((uint32_t)entity);
        auto& component = Application::GetWorld().GetComponent<UITextComponent>(ent);
        auto& transform = Application::GetWorld().GetComponent<TransformComponent>(ent);
        if (!component.font || component.font->GetStatus() != FontObject::Font_status::LOADED) continue;
        int max_size = component.font->max_size;
        if ((!component.text_quads) || component.dirty) {
            int width = std::ceil(std::sqrt(LOAD_FONT_SYMBOLS_COUNT));
            int height = std::ceil(LOAD_FONT_SYMBOLS_COUNT / width) + 1;
            float x_pos = 0;
            float scale = 1.0f / max_size / 96;
            float y_pos = 0;
            int buffer_offset = 0;
            TextVertex* data_buffer = new TextVertex[component.text.size() * 6];
            for (char character : component.text) {
                const FontObject::GlyphData& ch = component.font->GetGlyphData(character);
                float xpos = x_pos + ch.offset_x * scale;
                float ypos = y_pos - (ch.height * scale - ch.offset_y * scale);

                float uv_x_min = (float)ch.x / (float)width;
                float uv_x_max = (float)uv_x_min + (float)ch.width / (float)max_size / (float)width;
                float uv_y_min = (float)ch.y / (float)height;
                float uv_y_max = (float)uv_y_min + (float)ch.height / (float)max_size / (float)height;

                float w = ch.width * scale;
                float h = ch.height * scale;
                TextVertex tile_vertex[6] = {
                    { {xpos, ypos + h},     {uv_x_min, uv_y_min}},
                    { {xpos, ypos},         {uv_x_min, uv_y_max}},
                    { {xpos + w, ypos},     {uv_x_max, uv_y_max}},
                    { {xpos, ypos + h},     {uv_x_min, uv_y_min}},
                    { {xpos + w, ypos},     {uv_x_max, uv_y_max}},
                    { {xpos + w, ypos + h}, {uv_x_max, uv_y_min}}
                };
                memcpy(data_buffer + buffer_offset, tile_vertex, sizeof(tile_vertex));
                buffer_offset += 6;
                x_pos += (float)ch.advance * scale;
            }
            RenderBufferDescriptor text_buf(sizeof(TextVertex) * component.text.size() * 6, RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);
            component.text_quads = RenderResourceManager::Get()->CreateBuffer(text_buf);
            RenderResourceManager::Get()->UploadDataToBuffer(list, component.text_quads, data_buffer, sizeof(TextVertex) * component.text.size() * 6, 0);


            delete[] data_buffer;
            component.dirty = false;
        }
        list->SetTexture2D("font_atlas", component.font->font_atlas);
        int res_x = Application::Get()->GetWindow()->GetProperties().resolution_x;
        int res_y = Application::Get()->GetWindow()->GetProperties().resolution_y;


        glm::mat4 text_matrix = transform.TransformMatrix;
        glm::mat4 projection = glm::ortho(0.0f, (float)res_x / (float)res_y, 0.0f, 1.0f);
        text_matrix = projection * text_matrix;
        float font_size = component.GetFontSize();
        RenderResourceManager::Get()->UploadDataToBuffer(list, m_Internal_data->const_buf, glm::value_ptr(text_matrix), sizeof(glm::mat4), 0);
        RenderResourceManager::Get()->UploadDataToBuffer(list, m_Internal_data->const_buf, &font_size, sizeof(float), sizeof(glm::mat4));
        list->SetVertexBuffer(component.text_quads);
        list->DrawArray(6 * component.text.size());


    }

    queue->ExecuteRenderCommandList(list);
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
                loaded_font.font->max_size = loaded_obj.object.max_size;
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
    FT_Face face;
    auto error = FT_New_Face(m_Internal_data->ft_lib, file_path.c_str(), 0, &face);
    int num_of_glyphs = std::min((int)face->num_glyphs, (int)LOAD_FONT_SYMBOLS_COUNT);
    int max_size = LOAD_FONT_SYMBOLS_HEIGHT;
    FT_Set_Pixel_Sizes(face, LOAD_FONT_SYMBOLS_HEIGHT, LOAD_FONT_SYMBOLS_HEIGHT);
    for (int i = 0; i < num_of_glyphs; i++) {
        error = FT_Load_Glyph(face, FT_Get_Char_Index(face, i), FT_LOAD_BITMAP_METRICS_ONLY);
        if (face->glyph->bitmap.rows > max_size) {
            max_size = face->glyph->bitmap.rows;
        }
        else if(face->glyph->bitmap.width > max_size) {
            max_size = face->glyph->bitmap.width;
        }

    }


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
    texture_desc.height = height * max_size;
    texture_desc.width = width * max_size;
    payload.object.font_atlas = RenderResourceManager::Get()->CreateTexture(texture_desc);
    if (error) {
        throw std::runtime_error("Could not opne font file: " + file_path);
    }

    int offset_x = 0;
    int offset_y = 0;
    auto list = Renderer::Get()->GetRenderCommandList();
    auto queue = Renderer::Get()->GetCommandQueue();
    for (int i = 0; i < num_of_glyphs; i++) {
        error = FT_Load_Glyph(face, FT_Get_Char_Index(face, i), FT_LOAD_RENDER);
        
        if (error) {
            throw std::runtime_error("Character could not be loaded");
        }
        if (offset_x + max_size > texture_desc.width) {
            offset_x = 0;
            offset_y += max_size;
        }
        if (face->glyph->bitmap.buffer) {
            RenderResourceManager::Get()->UploadDataToTexture2D(list, payload.object.font_atlas, (void*)face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, offset_x, offset_y, 0);
        }

        payload.object.glypth_data.push_back(FontObject::GlyphData{ (char)i,offset_x / max_size,offset_y / max_size,face->glyph->advance.x >> 6,face->glyph->bitmap_left,
            face->glyph->bitmap_top, (int)face->glyph->bitmap.width ,(int)face->glyph->bitmap.rows });

        offset_x += max_size;
    }
    payload.object.max_size = max_size;
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

TextRenderer::~TextRenderer()
{
    FT_Done_FreeType(m_Internal_data->ft_lib);
	delete m_Internal_data;
}
