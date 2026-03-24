#include "AnimationPropertiesPanelEntry.h"

#include "Application.h"
#include "imgui.h"
#include "Editor/Editor.h"
#include "World/Components/AnimationComponent.h"

AnimationPropertiesPanelEntry::AnimationPropertiesPanelEntry() : PropertiesPanelEntry("Animation Component") {
    path_buffer = new char[256];
    path_buffer[0] = '\0';
}

AnimationPropertiesPanelEntry::AnimationPropertiesPanelEntry(const AnimationPropertiesPanelEntry& other) : PropertiesPanelEntry("Animation Component"){
    path_buffer = new char[256];
    path_buffer[0] = '\0';
}

void AnimationPropertiesPanelEntry::RenderPanel(Entity ent) {
    World& world = Application::GetWorld();
    AnimationComponent& animation_component = world.GetComponent<AnimationComponent>(ent);
    if (strlen(path_buffer) == 0) {
        path_buffer[0] = '\0';
        memcpy(path_buffer, animation_component.animation_group_path.c_str(), animation_component.animation_group_path.size() + 1);
    }

    bool enter = ImGui::InputText("Animation group path", path_buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
    if(ImGui::Button("Reload") || enter) {
        animation_component.SetAnimation(path_buffer, 0.0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Set Selected")) {
        animation_component.SetAnimation(Editor::Get()->GetSelectedFilePath(), 0.0);
        memcpy(path_buffer, animation_component.animation_group_path.c_str(), animation_component.animation_group_path.size() + 1);
    }
    float speed = animation_component.playback_speed;
    if(ImGui::SliderFloat("Playback speed", &speed, 0.0f, 5.0f)) {
        animation_component.playback_speed = speed;
    }

    bool playing = animation_component.play_mode == AnimationComponent::PlayMode::PLAYING;
    if(playing && ImGui::Button("Pause")) {
        animation_component.Stop();
    } else if (!playing && ImGui::Button("Play")) {
        animation_component.Play();
    }

    bool looping = animation_component.looping;
    if(ImGui::Checkbox("Looping", &looping)) {
        animation_component.SetLooping(looping);
    }
}

bool AnimationPropertiesPanelEntry::IsAssignable() {
    return true;
}

bool AnimationPropertiesPanelEntry::IsAvailable(Entity ent) {
    return true;
}

void AnimationPropertiesPanelEntry::OnAssign(Entity ent) {
    Application::GetWorld().SetComponent<AnimationComponent>(ent);
}

void AnimationPropertiesPanelEntry::OnRemove(Entity ent) {
    Application::GetWorld().RemoveComponent<AnimationComponent>(ent);
}

bool AnimationPropertiesPanelEntry::IsAssigned(Entity ent) {
    return Application::GetWorld().HasComponent<AnimationComponent>(ent);
}

PropertiesPanelEntry* AnimationPropertiesPanelEntry::clone() {
    return (PropertiesPanelEntry*)new AnimationPropertiesPanelEntry();
}

AnimationPropertiesPanelEntry::~AnimationPropertiesPanelEntry() {
    if(path_buffer) {
        delete[] path_buffer;
    }
}
