#include "UITextPropertiesPanelEntry.h"
#include <FileManager.h>
#include <Application.h>
#include <World/World.h>
#include <imgui.h>
#include <variant>
#include <type_traits>
#include <World/Components/UITextComponent.h>

UITextPropertiesPanelEntry::UITextPropertiesPanelEntry() : PropertiesPanelEntry("UI Text Component"), buffer(new char[200]), text_buffer(new char[200])
{
	buffer[0] = '\0';
	text_buffer[0] = '\0';
}

UITextPropertiesPanelEntry::UITextPropertiesPanelEntry(const UITextPropertiesPanelEntry& other) : PropertiesPanelEntry("UI Text Component"), buffer(new char[200]), text_buffer(new char[200])
{
	buffer[0] = '\0';
	text_buffer[0] = '\0';
}

void UITextPropertiesPanelEntry::RenderPanel(Entity ent)
{
	World& world = Application::GetWorld();
	UITextComponent& comp = world.GetComponent<UITextComponent>(ent);

	if (strlen(buffer) == 0) {
		buffer[0] = '\0';
		if (comp.GetFontObject() && comp.GetFontObject()->GetStatus() == FontObject::Font_status::LOADED) {
			memcpy(buffer, comp.GetFontObject()->GetFilePath().c_str(), comp.GetFontObject()->GetFilePath().size() + 1);
		}
	}
	if (strlen(text_buffer) == 0) {
		text_buffer[0] = '\0';
		memcpy(text_buffer, comp.GetText().c_str(), comp.GetText().size() + 1);
	}
	bool enter_buffer = ImGui::InputText("font path", buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload##font") || enter_buffer) {
		if (buffer[0] != '\0') {
			comp.SetFontObject(TextRenderer::Get()->GetFontObject(FileManager::Get()->GetPath(buffer)));
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Set Selected##font")) {
		comp.SetFontObject(TextRenderer::Get()->GetFontObject(Editor::Get()->GetSelectedFilePath()));
		memcpy(buffer, comp.GetFontObject()->GetFilePath().c_str(), comp.GetFontObject()->GetFilePath().size() + 1);
	}

	if (ImGui::InputText("Text", text_buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue)) {
		comp.SetText(text_buffer);
	}

	int font_size = comp.GetFontSize();
	if (ImGui::DragInt("Font Size", &font_size) && font_size != comp.GetFontSize()) {
		comp.SetFontSize(font_size);
	}

}

bool UITextPropertiesPanelEntry::IsAvailable(Entity ent)
{
	return true;
}

bool UITextPropertiesPanelEntry::IsAssigned(Entity ent)
{
	return Application::GetWorld().HasComponent<UITextComponent>(ent);
}

PropertiesPanelEntry* UITextPropertiesPanelEntry::clone()
{
	return (PropertiesPanelEntry*)new UITextPropertiesPanelEntry(*this);
}

void UITextPropertiesPanelEntry::OnAssign(Entity ent)
{
	Application::GetWorld().SetComponent<UITextComponent>(ent);
}

void UITextPropertiesPanelEntry::OnRemove(Entity ent)
{
	Application::GetWorld().RemoveComponent<UITextComponent>(ent);
}

bool UITextPropertiesPanelEntry::IsAssignable()
{
	return true;
}

UITextPropertiesPanelEntry::~UITextPropertiesPanelEntry()
{
	if (buffer) {
		delete[] buffer;
	}
	if (text_buffer) {
		delete[] text_buffer;
	}
}
