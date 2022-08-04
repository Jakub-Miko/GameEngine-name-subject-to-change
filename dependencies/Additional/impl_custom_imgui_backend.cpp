#include "impl_custom_imgui_backend.h"
#include <stdexcept>

#ifdef  OpenGL
#include <platform/OpenGL/OpenGLRenderCommandList.h>
#endif

bool impl_custom_imgui_backend::objects_init = false;

static void ImGui_custom_RenderWindow(ImGuiViewport* viewport, void*);
static void ImGui_custom_InitPlatformInterface();
static void ImGui_custom_ShutdownPlatformInterface();

impl_custom_imgui_backend::backend_data* impl_custom_imgui_backend::current_backend_data = nullptr;

struct ImGUI_Shader {  };

template<>
struct RootSignatureFactory<ImGUI_Shader> {

    static RootSignature* GetRootSignature() {
        static RootSignature* signature = nullptr;
        if (!signature) {
            RootSignature* sig = RootSignature::CreateSignature(RootSignatureDescriptor(
                {
                    RootSignatureDescriptorElement("conf",RootParameterType::CONSTANT_BUFFER),
                    RootSignatureDescriptorElement("Texture",RootParameterType::TEXTURE_2D)
                }
            ));

            signature = sig;
        }

        return signature;
    }

};

template<>
struct VertexLayoutFactory<ImGUI_Shader> {

    static VertexLayout* GetLayout() {
        static std::unique_ptr<VertexLayout> layout = nullptr;
        if (!layout) {
            VertexLayout* layout_new = new VertexLayout({
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2,"Position"),
                VertexLayoutElement(RenderPrimitiveType::FLOAT,2,"UV"),
                VertexLayoutElement(RenderPrimitiveType::UNSIGNED_CHAR,4,"Color",true)
                });


            layout = std::unique_ptr<VertexLayout>(layout_new);
        }
        return layout.get();
    }

};

static void ResetState(ImDrawData* data, int width, int height) {
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();

    float L = data->DisplayPos.x;
    float R = data->DisplayPos.x + data->DisplaySize.x;
    float T = data->DisplayPos.y;
    float B = data->DisplayPos.y + data->DisplaySize.y;
    const float ortho_projection[4][4] =
    {
        { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
        { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
        { 0.0f,         0.0f,        -1.0f,   0.0f },
        { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
    };

    list->SetPipeline(impl_custom_imgui_backend::GetBackendData()->pipeline);
    list->SetViewport(RenderViewport({ 0,0 }, { width,height }));
    RenderResourceManager::Get()->UploadDataToBuffer(list, impl_custom_imgui_backend::GetBackendData()->constant_buffer.GetResource(), (void*)&ortho_projection, sizeof(float) * 16, 0);
    list->SetConstantBuffer("conf", impl_custom_imgui_backend::GetBackendData()->constant_buffer.GetResource());
    list->SetVertexBuffer(impl_custom_imgui_backend::GetBackendData()->vertex_buffer.GetResource());
    list->SetIndexBuffer(impl_custom_imgui_backend::GetBackendData()->index_buffer.GetResource());

    #ifdef  OpenGL
    //Used to deal with opengl multi contexts not sharing vaos.
    static_cast<OpenGLRenderCommandList*>(list)->RefreshVertexContext();
#endif //  OpenGL


    queue->ExecuteRenderCommandList(list);
}

static void CreateShaders() {
    const char* shader_glsl_410_core =
        R"(
        #RootSignature
        {
        	"RootSignature": [
        		{
        			"name" : "conf",
        			"type" : "constant_buffer"
        		},
                {
        			"name" : "Texture",
        			"type" : "texture_2D"
        		}
        
        	]
        }
        #end
        )"        
        "#Vertex //--------------------------------------------------\n"
        "#version 410\n"
        "\n"
        "uniform conf "
        "{\n"
        "   mat4 ProjMtx;\n"
        "};\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n"
        "#end\n"
        "#Fragment //------------------------------------------------\n"
        "#version 410\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "uniform sampler2D Texture[1];"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture[0], Frag_UV.st);\n"
        "}\n"
        "#end\n";

    auto shader = ShaderManager::Get()->CreateShaderFromString(shader_glsl_410_core);

    impl_custom_imgui_backend::GetBackendData()->shader = shader;
}

static void CreatePipeline() {
    PipelineDescriptor desc;

    desc.flags = PipelineFlags::ENABLE_SCISSOR_TEST | PipelineFlags::ENABLE_BLEND | PipelineFlags::IS_MULTI_WINDOW;
    desc.layout = VertexLayoutFactory<ImGUI_Shader>::GetLayout();
    desc.polygon_render_mode = PrimitivePolygonRenderMode::DEFAULT;
    desc.scissor_rect = RenderScissorRect();
    desc.shader = impl_custom_imgui_backend::GetBackendData()->shader;
    desc.viewport = RenderViewport();
    desc.blend_functions = PipelineBlendFunctions{ BlendFunction::SRC_ALPHA, BlendFunction::ONE_MINUS_SRC_ALPHA ,BlendFunction::ONE, BlendFunction::ONE_MINUS_SRC_ALPHA };

    auto pipeline = PipelineManager::Get()->CreatePipeline(desc);
    impl_custom_imgui_backend::GetBackendData()->pipeline = pipeline;

}

static void CreateBuffers() {
    RenderBufferDescriptor index_desc(0, RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);

    impl_custom_imgui_backend::GetBackendData()->index_buffer = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([&]() -> std::shared_ptr<RenderBufferResource> {
        std::shared_ptr<RenderBufferResource> index_buffer_instance = RenderResourceManager::Get()->CreateBuffer(index_desc);
        return index_buffer_instance;
        });

    RenderBufferDescriptor vertex_desc(0, RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);

    impl_custom_imgui_backend::GetBackendData()->vertex_buffer = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([&]() -> std::shared_ptr<RenderBufferResource> { 
        std::shared_ptr<RenderBufferResource> vertex_buffer_instance = RenderResourceManager::Get()->CreateBuffer(vertex_desc);
        return vertex_buffer_instance;
        });

    RenderBufferDescriptor const_buf_desc((size_t)(sizeof(float) * 16), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);

    impl_custom_imgui_backend::GetBackendData()->constant_buffer = FrameMultiBufferResource<std::shared_ptr<RenderBufferResource>>([&]() -> std::shared_ptr<RenderBufferResource> {

        std::shared_ptr<RenderBufferResource> constant_buffer_instance = RenderResourceManager::Get()->CreateBuffer(const_buf_desc);
        return constant_buffer_instance;
        });

}

static void CreateTextures() {
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();
    ImGuiIO& io = ImGui::GetIO();
    // Build texture atlas
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    RenderTexture2DDescriptor desc;
    desc.format = TextureFormat::RGBA_UNSIGNED_CHAR;
    desc.height = height;
    desc.width = width;

    TextureSamplerDescritor sampler_desc;
    sampler_desc.AddressMode_U = TextureAddressMode::CLAMP;
    sampler_desc.AddressMode_V = TextureAddressMode::CLAMP;
    sampler_desc.AddressMode_W = TextureAddressMode::CLAMP;
    sampler_desc.border_color = glm::vec4(1.0f);
    sampler_desc.filter = TextureFilter::LINEAR_MIN_MAG_MIP;
    sampler_desc.LOD_bias = 0;
    sampler_desc.max_LOD = 5;
    sampler_desc.min_LOD = 0;

    auto sampler = TextureSampler::CreateSampler(sampler_desc);

    desc.sampler = sampler;

    auto texture = RenderResourceManager::Get()->CreateTexture(desc);
    impl_custom_imgui_backend::GetBackendData()->atlas_texture = texture;

    RenderResourceManager::Get()->UploadDataToTexture2D(list, texture, pixels, width, height, 0, 0, 0);


    std::shared_ptr<RenderFence> fence = std::shared_ptr<RenderFence>(Renderer::Get()->GetFence());
    queue->ExecuteRenderCommandList(list);
    queue->Signal(fence, 1);
    fence->WaitForValue(1);

    io.Fonts->SetTexID((ImTextureID)texture.get());
}

void impl_custom_imgui_backend::CreateObjects() {
    CreateShaders();
    CreatePipeline();
    CreateBuffers();
    CreateTextures();
}

void impl_custom_imgui_backend::Init()
{
    ImGuiIO& io = ImGui::GetIO();

    io.BackendRendererUserData = GetBackendData();
    io.BackendRendererName = "imgui_impl_custom";
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    if (current_backend_data) throw std::runtime_error("ImGUI Backend is already inintialized");
	current_backend_data = new backend_data;


    ImGui_custom_InitPlatformInterface();

}

void impl_custom_imgui_backend::Shutdown()
{

}

void impl_custom_imgui_backend::NewFrame()
{
    if (objects_init == false) {
        CreateObjects();
        objects_init = true;
    }
}

void impl_custom_imgui_backend::PreShutdown()
{
    ImGui_custom_ShutdownPlatformInterface();
    delete current_backend_data;
}

void impl_custom_imgui_backend::DrawData(ImDrawData* draw_data)
{
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();

    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho_projection[4][4] =
    {
        { 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
        { 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
        { 0.0f,         0.0f,        -1.0f,   0.0f },
        { (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
    };

    list->SetPipeline(current_backend_data->pipeline);
    list->SetViewport(RenderViewport({ 0,0 }, { fb_width,fb_height },0.0f,1000.0f));
    RenderResourceManager::Get()->UploadDataToBuffer(list, current_backend_data->constant_buffer.GetResource(), (void*)&ortho_projection, sizeof(float) * 16, 0);
    list->SetConstantBuffer("conf", current_backend_data->constant_buffer.GetResource());
    list->SetVertexBuffer(current_backend_data->vertex_buffer.GetResource());
    list->SetIndexBuffer(current_backend_data->index_buffer.GetResource());
    list->SetDefaultRenderTarget();


    ImVec2 clip_off = draw_data->DisplayPos;      
    ImVec2 clip_scale = draw_data->FramebufferScale;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        const size_t vtx_buffer_size = (size_t)cmd_list->VtxBuffer.Size * (int)sizeof(ImDrawVert);
        const size_t idx_buffer_size = (size_t)cmd_list->IdxBuffer.Size * (int)sizeof(ImDrawIdx);
        RenderResourceManager::Get()->ReallocateAndUploadBuffer(list, current_backend_data->vertex_buffer.GetResource(), (void*)cmd_list->VtxBuffer.Data, vtx_buffer_size);
        RenderResourceManager::Get()->ReallocateAndUploadBuffer(list, current_backend_data->index_buffer.GetResource(), (void*)cmd_list->IdxBuffer.Data, idx_buffer_size);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)

                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                    ResetState(draw_data, fb_width, fb_height);
                }
                else {
                    pcmd->UserCallback(cmd_list, pcmd);
                }

            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
                if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                    continue;

                // Apply scissor/clipping rectangle (Y is inverted in OpenGL)
                list->SetScissorRect(RenderScissorRect({ (int)clip_min.x, (int)((float)fb_height - clip_max.y) }, { (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y) }));

                //WARNING: thextures passed this way are not reference counted properly and if not managed properly will cause dangling pointers.
                list->SetTexture2D("Texture", std::shared_ptr< RenderTexture2DResource>((RenderTexture2DResource*)pcmd->TextureId, [](RenderTexture2DResource* ptr)
                    {}));
                list->Draw(pcmd->ElemCount, true, (int)(pcmd->IdxOffset * sizeof(ImDrawIdx)));

            }



        }

    }
    

    queue->ExecuteRenderCommandList(list);
    ResetState(draw_data, fb_width, fb_height);

}

static void ImGui_custom_RenderWindow(ImGuiViewport* viewport, void*)
{
    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
    {
        auto queue = Renderer::Get()->GetCommandQueue();
        auto list = Renderer::Get()->GetRenderCommandList();
        list->Clear();
        queue->ExecuteRenderCommandList(list);
    }
    impl_custom_imgui_backend::DrawData(viewport->DrawData);

}

static void ImGui_custom_InitPlatformInterface()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_RenderWindow = ImGui_custom_RenderWindow;
}

static void ImGui_custom_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}