#include "MeshPropertiesPanelEntry.h"
#include <Application.h>
#include <World/World.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
#include <World/Components/PhysicsComponent.h>
#include <World/Components/LightComponent.h>
#include <imgui.h>

MeshPropertiesPanelEntry::MeshPropertiesPanelEntry() : PropertiesPanelEntry("Mesh Component"), mesh_path_buffer(new char[200]), material_path_buffer(new char[200])
{
	mesh_path_buffer[0] = '\0';
	material_path_buffer[0] = '\0';
}

MeshPropertiesPanelEntry::MeshPropertiesPanelEntry(const MeshPropertiesPanelEntry& other) : PropertiesPanelEntry("Mesh Component"), mesh_path_buffer(new char[200]), material_path_buffer(new char[200])
{
	mesh_path_buffer[0] = '\0';
	material_path_buffer[0] = '\0';
}

void MeshPropertiesPanelEntry::RenderPanel(Entity ent)
{
	World& world = Application::GetWorld();
	MeshComponent& mesh = world.GetComponent<MeshComponent>(ent);
	if (strlen(mesh_path_buffer) == 0) {
		mesh_path_buffer[0] = '\0';
		memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
	}
	bool visible = mesh.GetVisibility();
	ImGui::Checkbox("Visible", &visible);
	mesh.SetVisibility(visible);

	bool enter = ImGui::InputText("Mesh path", mesh_path_buffer, 200, ImGuiInputTextFlags_EnterReturnsTrue);
	if (ImGui::Button("Reload") || enter) {
		try {
			Application::GetWorld().SetEntityMesh(ent, FileManager::Get()->GetPath(mesh_path_buffer));
		}
		catch (std::runtime_error* e) {
			Editor::Get()->EditorError("Invalid or corrupted Mesh file");
			mesh_path_buffer[0] = '\0';
			memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
		}
	}
	ImGui::SameLine();
	if (mesh.GetMesh()->GetMeshStatus() == Mesh_status::ERROR) {
		mesh.ResetMesh();
		Editor::Get()->EditorError("Invalid or corrupted Mesh file");
		mesh_path_buffer[0] = '\0';
		memcpy(mesh_path_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
	}

	if (ImGui::Button("Set Selected")) {
		Application::GetWorld().SetEntityMesh(ent, Editor::Get()->GetSelectedFilePath());
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




}

bool MeshPropertiesPanelEntry::IsAvailable(Entity ent)
{
	return !Application::GetWorld().HasComponent<LightComponent>(ent) && !Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent);
}

bool MeshPropertiesPanelEntry::IsAssigned(Entity ent)
{
	return Application::GetWorld().HasComponent<MeshComponent>(ent);
}

PropertiesPanelEntry* MeshPropertiesPanelEntry::clone()
{
	return (PropertiesPanelEntry*)new MeshPropertiesPanelEntry(*this);
}

void MeshPropertiesPanelEntry::OnAssign(Entity ent)
{
	Application::GetWorld().SetComponent<MeshComponent>(ent);
}

void MeshPropertiesPanelEntry::OnRemove(Entity ent)
{
	auto& world = Application::GetWorld();
	world.RemoveComponent<MeshComponent>(ent);
	if (Application::GetWorld().HasComponent<PhysicsComponent>(ent) && !Application::GetWorld().HasComponent<SkeletalMeshComponent>(ent)) {
		world.RemoveComponent<PhysicsComponent>(ent);
	}
	memcpy(mesh_path_buffer, "Unknown", strlen("Unknown") + 1);
}

bool MeshPropertiesPanelEntry::IsAssignable()
{
	return true;
}

MeshPropertiesPanelEntry::~MeshPropertiesPanelEntry()
{
	if (mesh_path_buffer) {
		delete mesh_path_buffer;
	}
	if (material_path_buffer) {
		delete material_path_buffer;
	}
}
