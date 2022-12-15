#include "DynamicPropertiesPanelEntry.h"
#include <Application.h>
#include <World/World.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/PhysicsComponent.h>
#include <World/Components/LightComponent.h>
#include <imgui.h>
#include <variant>
#include <type_traits>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

DynamicPropertiesPanelEntry::DynamicPropertiesPanelEntry() : PropertiesPanelEntry("Dynamic Properties")
{

}

DynamicPropertiesPanelEntry::DynamicPropertiesPanelEntry(const DynamicPropertiesPanelEntry& other) : PropertiesPanelEntry("Dynamic Properties") 
{

}

void DynamicPropertiesPanelEntry::RenderPanel(Entity ent)
{
	World& world = Application::GetWorld();
	DynamicPropertiesComponent& props = world.GetComponent<DynamicPropertiesComponent>(ent);
	for (auto& prop : props.m_Properties) {
			std::visit([&prop,&props](auto&& value) {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, int>) {
					
					if (ImGui::TreeNode(prop.first.c_str())) {

						int v = value;
						ImGui::DragInt(" Value", &v);
						if (v != value) {
							props.m_Properties[prop.first] = v;
						}
						ImGui::TreePop();
					}
				}
				else if constexpr (std::is_same_v<T, float>) {
					if (ImGui::TreeNode(prop.first.c_str())) {
						float v = value;
						ImGui::DragFloat(" Value", &v);
						if (v != value) {
							props.m_Properties[prop.first] = v;
						}
						ImGui::TreePop();
					}
				}
				else if constexpr (std::is_same_v<T, double>) {
					if (ImGui::TreeNode(prop.first.c_str())) {
						float v = value;
						ImGui::DragFloat(" Value", &v);
						if (v != value) {
							props.m_Properties[prop.first] = v;
						}
						ImGui::TreePop();
					}
				}
				else if constexpr (std::is_same_v<T, glm::vec2>) {
					if (ImGui::TreeNode(prop.first.c_str())) {
						glm::vec2 v = value;
						ImGui::DragFloat2(" Value", glm::value_ptr(v));
						if (v != value) {
							props.m_Properties[prop.first] = v;
						}
						ImGui::TreePop();
					}
				}
				else if constexpr (std::is_same_v<T, glm::vec3>) {
					if (ImGui::TreeNode(prop.first.c_str())) {
						glm::vec3 v = value;
						ImGui::DragFloat3(" Value", glm::value_ptr(v));
						if (v != value) {
							props.m_Properties[prop.first] = v;
						}
						ImGui::TreePop();
					}
				}
				else if constexpr (std::is_same_v<T, glm::vec4>) {
					if (ImGui::TreeNode(prop.first.c_str())) {
						glm::vec4 v = value;
						ImGui::DragFloat4(" Value", glm::value_ptr(v));
						if (v != value) {
							props.m_Properties[prop.first] = v;
						}
						ImGui::TreePop();
					}
				}
				else if constexpr (std::is_same_v<T, std::string>) {
					if (ImGui::TreeNode(prop.first.c_str())) {
						char text[200];
						strcpy_s(text, value.data());
						bool changed = ImGui::InputText(" Value", text, 200);
						if (changed && text != value) {
							props.m_Properties[prop.first] = text;
						}
						ImGui::TreePop();
					}
				}


			}, prop.second);
	}

}

bool DynamicPropertiesPanelEntry::IsAvailable(Entity ent)
{
	return Application::GetWorld().HasComponent<ScriptComponent>(ent);
}

bool DynamicPropertiesPanelEntry::IsAssigned(Entity ent)
{
	return Application::GetWorld().HasComponent<DynamicPropertiesComponent>(ent);
}

PropertiesPanelEntry* DynamicPropertiesPanelEntry::clone()
{
	return (PropertiesPanelEntry*)new DynamicPropertiesPanelEntry(*this);
}

void DynamicPropertiesPanelEntry::OnAssign(Entity ent)
{
	throw std::runtime_error("Dynamic Properties Component cannot be addigned by the Editor");
}

void DynamicPropertiesPanelEntry::OnRemove(Entity ent)
{
	throw std::runtime_error("Dynamic Properties Component cannot be removed by the Editor");
}

bool DynamicPropertiesPanelEntry::IsAssignable()
{
	return false;
}

DynamicPropertiesPanelEntry::~DynamicPropertiesPanelEntry()
{

}
