#include "Viewport.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <Renderer/Renderer.h>
#include <Renderer/RenderResourceManager.h>
#include <Application.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <World/World.h>
#include <Window.h>

Viewport::Viewport()
{
    TextureSamplerDescritor smp_desc;

    auto window_props = Application::Get()->GetWindow()->GetProperties();

    viewport_resolution_x = window_props.resolution_x;
    viewport_resolution_y = window_props.resolution_y;

    viewport_size = glm::vec2(viewport_resolution_x,viewport_resolution_y) / 2.0f;

    auto fb_sampler = TextureSampler::CreateSampler(smp_desc);

    RenderTexture2DDescriptor color_desc;
    color_desc.format = TextureFormat::RGB_UNSIGNED_CHAR;
    color_desc.height = viewport_resolution_y;
    color_desc.width = viewport_resolution_x;
    color_desc.sampler = fb_sampler;

    auto color_at = RenderResourceManager::Get()->CreateTexture(color_desc);

    RenderTexture2DDescriptor depth_desc;
    depth_desc.format = TextureFormat::DEPTH24_STENCIL8_UNSIGNED_CHAR;
    depth_desc.height = viewport_resolution_y;
    depth_desc.width = viewport_resolution_x;
    depth_desc.sampler = fb_sampler;

    auto depth_at = RenderResourceManager::Get()->CreateTexture(depth_desc);

    RenderFrameBufferDescriptor desc;
    desc.color_attachments = { color_at };
    desc.depth_stencil_attachment = depth_at;

    viewport_frame_buffer = RenderResourceManager::Get()->CreateFrameBuffer(desc);
    Renderer::Get()->SetDefaultFrameBuffer(viewport_frame_buffer);


}

Viewport::~Viewport()
{
}

void Viewport::Render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2{ viewport_size.x,viewport_size.y + ImGui::GetCurrentWindow()->TitleBarHeight() });

    Editor::Get()->is_viewport_focused = ImGui::IsWindowFocused();

    bool phys_active = Application::GetWorld().GetPhysicsEngine().IsPhysicsActive();
    if (phys_active) ImGui::BeginDisabled();
    if (ImGui::Button("Enable Simulation")) {
        Application::GetWorld().GetPhysicsEngine().ActiveMode();
    }
    if (phys_active) ImGui::EndDisabled();
    ImGui::SameLine();
    if (!phys_active) ImGui::BeginDisabled();
    if (ImGui::Button("Disable Simulation")) {
        Application::GetWorld().GetPhysicsEngine().PassiveMode();
    }
    if (!phys_active) ImGui::EndDisabled();

    ImGui::SetCursorPos((ImGui::GetWindowSize() + ImVec2{ 0,ImGui::GetCurrentWindow()->TitleBarHeight() } - ImVec2{ viewport_size.x,viewport_size.y }) * 0.5f); +ImGui::GetCurrentWindow()->TitleBarHeight();

    ImGui::Image(static_cast<ImTextureID>(viewport_frame_buffer->GetBufferDescriptor().color_attachments[0].get()), { viewport_size.x,viewport_size.y}, { 0,1 }, { 1,0 });

    ImGui::End();

    ImGui::PopStyleVar();

}

void Viewport::BeginViewportFrameBuffer()
{
    Renderer::Get()->SetDefaultFrameBuffer(viewport_frame_buffer);
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();
    list->SetDefaultRenderTarget();
    list->Clear();
    queue->ExecuteRenderCommandList(list);
}

void Viewport::EndViewportFrameBuffer()
{
    Renderer::Get()->SetDefaultFrameBuffer();
    auto queue = Renderer::Get()->GetCommandQueue();
    auto list = Renderer::Get()->GetRenderCommandList();
    list->SetDefaultRenderTarget();
    queue->ExecuteRenderCommandList(list);
}
