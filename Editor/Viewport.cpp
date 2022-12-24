#include "Viewport.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <World/Components/CameraComponent.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderResourceManager.h>
#include <Application.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <ImGuizmo.h>
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
    ImGui::SameLine();
    ImGui::Separator();

    ViewportGizmoMode mode = gizmo_mode;
    if (gizmo_mode == ViewportGizmoMode::TRANSLATION) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Translate")) {
        mode = ViewportGizmoMode::TRANSLATION;
    }
    if (gizmo_mode == ViewportGizmoMode::TRANSLATION) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (gizmo_mode == ViewportGizmoMode::ROTATION) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Rotate")) {
        mode = ViewportGizmoMode::ROTATION;
    }
    if (gizmo_mode == ViewportGizmoMode::ROTATION) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (gizmo_mode == ViewportGizmoMode::SCALE) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Scale")) {
        mode = ViewportGizmoMode::SCALE;
    }
    if (gizmo_mode == ViewportGizmoMode::SCALE) {
        ImGui::EndDisabled();
    }
    gizmo_mode = mode;

    ImGui::SetCursorPos((ImGui::GetWindowSize() + ImVec2{ 0,ImGui::GetCurrentWindow()->TitleBarHeight() } - ImVec2{ viewport_size.x,viewport_size.y }) * 0.5f); 

    ImGui::Image(static_cast<ImTextureID>(viewport_frame_buffer->GetBufferDescriptor().color_attachments[0].get()), { viewport_size.x,viewport_size.y}, { 0,1 }, { 1,0 });

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImVec2 mid = ImGui::GetWindowSize() / 2;
    ImVec2 pos = ImGui::GetWindowPos() + ImVec2{ 0,ImGui::GetCurrentWindow()->TitleBarHeight()/2 };
    ImGuizmo::SetRect(pos.x + mid.x - viewport_size.x/2, pos.y + mid.y - viewport_size.y/2, viewport_size.x, viewport_size.y);

    if (Editor::Get()->GetSelectedEntity() != Entity()) {
        Entity camera = Application::GetWorld().GetPrimaryEntity();
        auto& camera_comp = Application::GetWorld().GetComponent<CameraComponent>(camera);
        camera_comp.UpdateProjectionMatrix();
        Entity selected = Editor::Get()->GetSelectedEntity();
        auto camera_transform = glm::inverse(Application::GetWorld().GetComponent<TransformComponent>(camera).TransformMatrix);
        auto& projection = camera_comp.GetProjectionMatrix();
        auto& transform_comp = Application::GetWorld().GetComponent<TransformComponent>(selected);
        auto transform = transform_comp.TransformMatrix;
        glm::mat4 delta = glm::mat4(1.0f);

        ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
        switch (gizmo_mode)
        {
        case ViewportGizmoMode::TRANSLATION:
            op = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case ViewportGizmoMode::SCALE:
            op = ImGuizmo::OPERATION::SCALE;
            break;
        case ViewportGizmoMode::ROTATION:
            op = ImGuizmo::OPERATION::ROTATE;
            break;
        }


        ImGuizmo::Manipulate(glm::value_ptr(camera_transform), glm::value_ptr(projection), op, ImGuizmo::MODE::LOCAL, glm::value_ptr(transform),glm::value_ptr(delta));
        
        if (ImGuizmo::IsUsing()) {
            glm::mat3 rot = delta;
            glm::vec3 scale = glm::vec3(glm::length(rot[0]), glm::length(rot[1]), glm::length(rot[2]));
            rot[0] = rot[0] / scale.x;
            rot[1] = rot[1] / scale.y;
            rot[2] = rot[2] / scale.z;
            glm::quat rot_quat = glm::toQuat(rot);

            switch (gizmo_mode)
            {
            case ViewportGizmoMode::TRANSLATION:
                Application::GetWorld().SetEntityTranslation(selected, transform_comp.translation + glm::vec3(delta[3]));
                break;
            case ViewportGizmoMode::SCALE:
                Application::GetWorld().SetEntityScale(selected, transform_comp.size * scale);
                break;
            case ViewportGizmoMode::ROTATION:
                Application::GetWorld().SetEntityRotation(selected, rot_quat * transform_comp.rotation);
                break;
            }

        }

    }

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
