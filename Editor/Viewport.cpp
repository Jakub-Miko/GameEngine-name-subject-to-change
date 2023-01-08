#include "Viewport.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include <World/Components/CameraComponent.h>
#include <World/Components/UITextComponent.h>
#include <Renderer/Renderer.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/RenderResourceManager.h>
#include <Application.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <ImGuizmo.h>
#include <Input/Input.h>
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


    if (entity_pick_request.IsValid() && entity_pick_request.IsAvailable()) {
        read_pixel_data data = entity_pick_request.GetValue();
        Entity select = Entity(std::get<unsigned int>(data));
        if (Application::GetWorld().EntityIsValid(select)) {
            SceneNode* node = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(select);
            SceneNode* parent_node = node;
            while (!(int)(parent_node->state & SceneNodeState::PREFAB) && parent_node->parent != nullptr)
            {
                parent_node = parent_node->parent;
            }
            if ((int)(node->state & SceneNodeState::PREFAB_CHILD) && select_mode != ViewportSelectMode::PREFAB_CHILDREN) {
                if (!parent_node) throw std::runtime_error("Could not Find the Prefab Parent node of a Prefab Child");
                Editor::Get()->SetSelectedEntity(parent_node->entity);
            }
            else {
                Editor::Get()->SetSelectedEntity(select);
            }
            if ((int)(node->state & SceneNodeState::PREFAB_CHILD)) {
                PrefabEditorWindow* window = Editor::Get()->GetOpenPrefabWindow(parent_node->entity);
                if (window) {
                    window->selected_entity = select;
                }

            }
            entity_pick_request = Future<read_pixel_data>();
        }
        else {
            entity_pick_request = Future<read_pixel_data>();
        }
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowSize(ImVec2{ viewport_size.x,viewport_size.y + ImGui::GetCurrentWindow()->TitleBarHeight() });
    if (ImGui::IsWindowFocused() && Input::Get()->IsKeyPressed_Editor(KeyCode::KEY_DELETE) && Application::GetWorld().EntityExists(Editor::Get()->GetSelectedEntity())) {
        Application::GetWorld().RemoveEntity(Editor::Get()->GetSelectedEntity());
        Editor::Get()->SetSelectedEntity(Entity());
    }

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

    ViewportGizmoSpace space = gizmo_space;
    if (gizmo_space ==ViewportGizmoSpace::LOCAL) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Local")) {
        space = ViewportGizmoSpace::LOCAL;
    }
    if (gizmo_space == ViewportGizmoSpace::LOCAL) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (gizmo_space == ViewportGizmoSpace::WORLD) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("World")) {
        space = ViewportGizmoSpace::WORLD;
    }
    if (gizmo_space == ViewportGizmoSpace::WORLD) {
        ImGui::EndDisabled();
    }
    gizmo_space = space;


    ViewportSelectMode sel_md = select_mode;
    if (select_mode == ViewportSelectMode::PREFABS) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Prefabs")) {
        sel_md = ViewportSelectMode::PREFABS;
    }
    if (select_mode == ViewportSelectMode::PREFABS) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (select_mode == ViewportSelectMode::PREFAB_CHILDREN) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Prefab Children")) {
        sel_md = ViewportSelectMode::PREFAB_CHILDREN;
    }
    if (select_mode == ViewportSelectMode::PREFAB_CHILDREN) {
        ImGui::EndDisabled();
    }
    select_mode = sel_md;



    ImGui::SetCursorPos((ImGui::GetWindowSize() + ImVec2{ 0,ImGui::GetCurrentWindow()->TitleBarHeight() } - ImVec2{ viewport_size.x,viewport_size.y }) * 0.5f); 

    ImVec2 mid = ImGui::GetWindowSize() / 2;
    ImGui::Image(static_cast<ImTextureID>(viewport_frame_buffer->GetBufferDescriptor().color_attachments[0].get()), { viewport_size.x,viewport_size.y}, { 0,1 }, { 1,0 });
    if (ImGui::IsItemClicked() && !(ImGuizmo::IsOver() || ImGuizmo::IsUsing())) {
        ImVec2 mouse_pos = ImGui::GetMousePos() - ImGui::GetWindowPos() - ImVec2{ 0,ImGui::GetCurrentWindow()->TitleBarHeight() / 2 } - mid + ImVec2{ viewport_size.x,viewport_size.y } / 2;
        mouse_pos /= ImVec2{ viewport_size.x,viewport_size.y };
        glm::vec2 clamped_pos = glm::clamp(glm::vec2(mouse_pos.x, mouse_pos.y), glm::vec2(0.0f), glm::vec2(1.0f));
        SelectEntityOnViewportPos(clamped_pos.x, clamped_pos.y);
    }

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
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

        ImGuizmo::MODE md = ImGuizmo::MODE::LOCAL;
        switch (gizmo_space)
        {
        case ViewportGizmoSpace::LOCAL:
            md = ImGuizmo::LOCAL;
            break;
        case ViewportGizmoSpace::WORLD:
            md = ImGuizmo::WORLD;
            break;
        }

        if (Editor::Get()->IsEditorEnabled()) {

            if (Application::GetWorld().HasComponent<UITextComponent>(selected)) {
                glm::mat4 camera = glm::mat4(1.0f);
                int res_x = Application::Get()->GetWindow()->GetProperties().resolution_x;
                int res_y = Application::Get()->GetWindow()->GetProperties().resolution_y;
                ImGuizmo::SetOrthographic(true);
                glm::mat4 projection_ui = glm::ortho(0.0f, (float)res_x / (float)res_y, 0.0f, 1.0f);
                transform[3][2] = 0.0f;
                ImGuizmo::Manipulate(glm::value_ptr(camera), glm::value_ptr(projection_ui), op, md, glm::value_ptr(transform), glm::value_ptr(delta));
            }
            else {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::Manipulate(glm::value_ptr(camera_transform), glm::value_ptr(projection), op, md, glm::value_ptr(transform), glm::value_ptr(delta));
            }

            if (ImGuizmo::IsUsing()) {
                glm::mat3 rot = delta;
                glm::vec3 scale = glm::vec3(glm::length(rot[0]), glm::length(rot[1]), glm::length(rot[2]));
                glm::vec3 trans_scale = glm::vec3(glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2])) / transform_comp.size;
                rot[0] = rot[0] / scale.x;
                rot[1] = rot[1] / scale.y;
                rot[2] = rot[2] / scale.z;
                glm::quat rot_quat = glm::toQuat(rot);

                switch (gizmo_mode)
                {
                case ViewportGizmoMode::TRANSLATION:
                    Application::GetWorld().SetEntityTranslation(selected, transform_comp.translation + glm::vec3(delta[3])/ trans_scale);
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

void Viewport::SelectEntityOnViewportPos(float x, float y)
{
    auto buffer = Renderer3D::Get()->GetPersistentResource<std::shared_ptr<RenderFrameBufferResource>>("G_Buffer");
    auto index = Renderer3D::Get()->GetPersistentResource<int>("ID");
    entity_pick_request = RenderResourceManager::Get()->GetPixelValue(buffer, index, x, y);
}
