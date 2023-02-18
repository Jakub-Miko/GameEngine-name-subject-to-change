#include "SkyLightPropertiesPanelEntry.h"
#include <FileManager.h>
#include <Application.h>
#include <World/World.h>
#include <imgui.h>
#include <variant>
#include <type_traits>
#include <World/Components/SkylightComponent.h>

SkyLightPropertiesPanelEntry::SkyLightPropertiesPanelEntry() : PropertiesPanelEntry("Sky Light Component"), buffer(new char[200])
{
	buffer[0] = '\0';
}

SkyLightPropertiesPanelEntry::SkyLightPropertiesPanelEntry(const SkyLightPropertiesPanelEntry& other) : PropertiesPanelEntry("Sky Light Component"), buffer(new char[200])
{
	buffer[0] = '\0';
}

void SkyLightPropertiesPanelEntry::RenderPanel(Entity ent)
{
	World& world = Application::GetWorld();
	SkylightComponent& comp = world.GetComponent<SkylightComponent>(ent);

	glm::vec3 color = (glm::vec3)comp.GetLightColor();
	if (ImGui::ColorPicker3("Tint Color", glm::value_ptr(color))) {
		comp.SetLightColor(glm::vec4(color, comp.GetLightColor().w));
	}
	
	float intensity = comp.GetLightColor().w;
	if (ImGui::DragFloat("Instensity", &intensity)) {
		comp.SetLightColor(glm::vec4((glm::vec3)comp.GetLightColor(), intensity));
	}
	
	if (strlen(buffer) == 0) {
		buffer[0] = '\0';
		memcpy_s(buffer,200 ,comp.GetReflectionMapPath().c_str(), comp.GetReflectionMapPath().size() + 1);
	}
	bool HDR_path = ImGui::InputText("HDR path", buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload##hdr") || HDR_path) {
		if (buffer[0] != '\0') {
			comp.SetReflectionMap(buffer);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Set Selected##hdr")) {
		comp.SetReflectionMap(FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()));
		memcpy_s(buffer,200 , comp.GetReflectionMapPath().c_str(), comp.GetReflectionMapPath().size() + 1);
	}

	bool show_bg = comp.IsBackgroundVisible();
	if (ImGui::Checkbox("Show Background", &show_bg)) {
		comp.SetShowBackground(show_bg);
	}


}

bool SkyLightPropertiesPanelEntry::IsAvailable(Entity ent)
{
	return true;
}

bool SkyLightPropertiesPanelEntry::IsAssigned(Entity ent)
{
	return Application::GetWorld().HasComponent<SkylightComponent>(ent);
}

PropertiesPanelEntry* SkyLightPropertiesPanelEntry::clone()
{
	return (PropertiesPanelEntry*)new SkyLightPropertiesPanelEntry(*this);
}

void SkyLightPropertiesPanelEntry::OnAssign(Entity ent)
{
	Application::GetWorld().SetComponent<SkylightComponent>(ent);
}

void SkyLightPropertiesPanelEntry::OnRemove(Entity ent)
{
	Application::GetWorld().RemoveComponent<SkylightComponent>(ent);
}

bool SkyLightPropertiesPanelEntry::IsAssignable()
{
	return true;
}

SkyLightPropertiesPanelEntry::~SkyLightPropertiesPanelEntry()
{
	if (buffer) {
		delete[] buffer;
	}
}
