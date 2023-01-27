#include "SkeletalMeshPropertiesPanelEntry.h"
#include <Application.h>
#include <World/World.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/PhysicsComponent.h>
#include <World/Components/LightComponent.h>
#include <imgui.h>

SkeletalMeshPropertiesPanelEntry::SkeletalMeshPropertiesPanelEntry() : PropertiesPanelEntry("Skeletal Mesh Component"), mesh_path_buffer(new char[200]), material_path_buffer(new char[200]), default_animation_path(new char[200])
{
	mesh_path_buffer[0] = '\0';
	material_path_buffer[0] = '\0';
	default_animation_path[0] = '\0';
}

SkeletalMeshPropertiesPanelEntry::SkeletalMeshPropertiesPanelEntry(const SkeletalMeshPropertiesPanelEntry& other) : PropertiesPanelEntry("Skeletal Mesh Component"), mesh_path_buffer(new char[200]), material_path_buffer(new char[200]),
 default_animation_path(new char[200])
{
	mesh_path_buffer[0] = '\0';
	material_path_buffer[0] = '\0';
	default_animation_path[0] = '\0';
}

void SkeletalMeshPropertiesPanelEntry::RenderPanel(Entity ent)
{
	World& world = Application::GetWorld();
	SkeletalMeshComponent& mesh = world.GetComponent<SkeletalMeshComponent>(ent);

	bool visible = mesh.GetVisibility();
	ImGui::Checkbox("Visible", &visible);
	mesh.SetVisibility(visible);

	if (strlen(mesh_path_buffer) == 0) {
		mesh_path_buffer[0] = '\0';
		memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
	}
	bool enter = ImGui::InputText("Mesh path", mesh_path_buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload") || enter) {
		try {
			Application::GetWorld().SetEntitySkeletalMesh(ent, FileManager::Get()->GetPath(mesh_path_buffer));
		}
		catch (std::runtime_error* e) {
			Editor::Get()->EditorError("Invalid or corrupted Skeletal Mesh file");
			mesh_path_buffer[0] = '\0';
			memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
		}
	}
	ImGui::SameLine();
	if (mesh.GetMesh()->GetMeshStatus() == Mesh_status::ERROR) {
		mesh.ResetMesh();
		Editor::Get()->EditorError("Invalid or corrupted Skeletal Mesh file");
		mesh_path_buffer[0] = '\0';
		memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
	}
	if (ImGui::Button("Set Selected")) {
		Application::GetWorld().SetEntitySkeletalMesh(ent, Editor::Get()->GetSelectedFilePath());
		memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
	}
	ImGui::Separator();
	if (strlen(material_path_buffer) == 0) {
		material_path_buffer[0] = '\0';
		memcpy(material_path_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
	}
	bool enter_material = ImGui::InputText("Material path", material_path_buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload##material") || enter_material) {
		try {
			mesh.ChangeMaterial(material_path_buffer);
		}
		catch (std::runtime_error* e) {
			Editor::Get()->EditorError("Invalid or corrupted Material file");
			material_path_buffer[0] = '\0';
			memcpy(material_path_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Set Selected##material")) {
		mesh.ChangeMaterial(FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()));
		memcpy(material_path_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
	}
	if (mesh.GetMaterialStatus() == Material::Material_status::ERROR) {
		Editor::Get()->EditorError("Invalid or corrupted Material file");
		mesh.ChangeMaterial("");
		material_path_buffer[0] = '\0';
		memcpy(material_path_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
	}

	ImGui::Separator();
	if (strlen(default_animation_path) == 0) {
		default_animation_path[0] = '\0';
		memcpy(default_animation_path, mesh.GetDefaultAnimationPath().c_str(), mesh.GetDefaultAnimationPath().size() + 1);
	}
	bool enter_anim = ImGui::InputText("Default Animation path", default_animation_path, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload##anim") || enter_anim) {
		try {
			mesh.SetDefaultAnimationPath(default_animation_path, true);
		}
		catch (std::runtime_error* e) {
			Editor::Get()->EditorError("Invalid or corrupted Animation file");
			default_animation_path[0] = '\0';
			memcpy(default_animation_path, mesh.GetDefaultAnimationPath().c_str(), mesh.GetDefaultAnimationPath().size() + 1);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Set Selected##anim")) {
		mesh.SetDefaultAnimationPath(FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()),true);
		memcpy(default_animation_path, mesh.GetDefaultAnimationPath().c_str(), mesh.GetDefaultAnimationPath().size() + 1);
	}
	if (mesh.GetAnimation().GetAnimationStatus() == Animation::animation_status::ERROR) {
		Editor::Get()->EditorError("Invalid or corrupted Animation file");
		mesh.SetDefaultAnimationPath("", true);
		default_animation_path[0] = '\0';
		memcpy(default_animation_path, mesh.GetDefaultAnimationPath().c_str(), mesh.GetDefaultAnimationPath().size() + 1);
	}


}

bool SkeletalMeshPropertiesPanelEntry::IsAvailable(Entity ent)
{
	return !Application::GetWorld().HasComponent<LightComponent>(ent) && !Application::GetWorld().HasComponent<MeshComponent>(ent);
}

bool SkeletalMeshPropertiesPanelEntry::IsAssigned(Entity ent)
{
	return Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent);
}

PropertiesPanelEntry* SkeletalMeshPropertiesPanelEntry::clone()
{
	return (PropertiesPanelEntry*)new SkeletalMeshPropertiesPanelEntry(*this);
}

void SkeletalMeshPropertiesPanelEntry::OnAssign(Entity ent)
{
	Application::GetWorld().SetComponent<SkeletalMeshComponent>(ent);
}

void SkeletalMeshPropertiesPanelEntry::OnRemove(Entity ent)
{
	auto& world = Application::GetWorld();
	world.RemoveComponent<SkeletalMeshComponent>(ent);
	if (Application::GetWorld().HasComponent<PhysicsComponent>(ent) && !Application::GetWorld().HasComponent<MeshComponent>(ent)) {
		world.RemoveComponent<PhysicsComponent>(ent);
	}
	memcpy(mesh_path_buffer, "Unknown", strlen("Unknown") + 1);
}

bool SkeletalMeshPropertiesPanelEntry::IsAssignable()
{
	return true;
}

SkeletalMeshPropertiesPanelEntry::~SkeletalMeshPropertiesPanelEntry()
{
	if (mesh_path_buffer) {
		delete mesh_path_buffer;
	}
	if (material_path_buffer) {
		delete material_path_buffer;
	}
	if (default_animation_path) {
		delete default_animation_path;
	}
}
