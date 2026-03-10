#include "EditorDynamicParameterUIAdapters.h"
#include <imgui.h>


void UIModuleAdapter<std::string>::RenderUI() {
    if(ImGui::InputText(value->GetName().c_str(), text_buffer.get(), MAX_INPUT_TEXT_SIZE, ImGuiInputTextFlags_EnterReturnsTrue)) {
        value->SetValue(std::string(text_buffer.get()));
        std::memcpy(text_buffer.get(), value->GetValueTyped().c_str(), std::min((size_t)MAX_INPUT_TEXT_SIZE, value->GetValueTyped().size()));
    }
}

void UIModuleAdapter<MultiChoice>::RenderUI() {
    int current = value->GetValueTyped().GetIndex();
    std::vector<const char*> choices;
    for (auto& choice : value->GetValueTyped().GetChoices()) {
        choices.push_back(choice.c_str());
    }
    if(ImGui::Combo(value->GetName().c_str(), &current, choices.data(), (int)value->GetValueTyped().GetChoices().size())) {
        value->GetValueTyped().SetValue(current);
        std::memcpy(text_buffer.get(), value->GetValueTyped().GetValue().c_str(), std::min((size_t)MAX_INPUT_TEXT_SIZE, value->GetValueTyped().GetValue().size()));
    };
}

void UIModuleAdapter<bool>::RenderUI() {
    bool val = value->GetValueTyped();
    if(ImGui::Checkbox(value->GetName().c_str(), &val)) {
        value->SetValue(val);
    }
}

void UIModuleAdapter<float>::RenderUI() {
    float val = value->GetValueTyped();
    if(ImGui::DragFloat(value->GetName().c_str(), &val)) {
        value->SetValue(val);
    }
}
