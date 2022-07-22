#include "PropertiesPanel.h"
#include <imgui.h>
#include <World/World.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/PrefabComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/MeshComponent.h>
#include <FileManager.h>

PropertiesPanel::PropertiesPanel()
{
	text_buffer = new char[buffer_size];
	text_buffer[0] = '\0';
	prefab_path_buffer = new char[buffer_size];
	prefab_path_buffer[0] = '\0';
}

PropertiesPanel::~PropertiesPanel()
{
	delete[] text_buffer;
	delete[] prefab_path_buffer;
}

void PropertiesPanel::Render()
{
	Entity selected = Editor::Get()->GetSelectedEntity();
	World& world = Application::GetWorld();

	ImGui::Begin("Properties");

	PropertiesPanel_persistent_data data;
	data.buffer_size = buffer_size;
	data.mesh_file_buffer = text_buffer;
	data.show_flags = (ShowPropertyFlags)0;
	data.prefab_path = prefab_path_buffer;

	if (selected != last_entity) {
		text_buffer[0] = '\0';
		prefab_path_buffer[0] = '\0';
		last_entity = selected;
	}

	RenderProperties(selected, data);

	ImGui::End();

}

void PropertiesPanel::Refresh()
{
	text_buffer[0] = '\0';
	prefab_path_buffer[0] = '\0';
}

void PropertiesPanel::RenderProperties(Entity entity, const PropertiesPanel_persistent_data& data)
{
	Entity selected = entity;
	World& world = Application::GetWorld();
	
	char* text_buffer = data.mesh_file_buffer;
	int buffer_size = data.buffer_size;


	if (ImGui::BeginPopupModal("Error##mesh", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Invalid or corrupted Mesh file");

		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	int mesh_error_id = ImGui::GetID("Error##mesh");

	if (ImGui::BeginPopupModal("Error##prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Invalid or corrupted Prefab file");

		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	int prefab_error_id = ImGui::GetID("Error##prefab");

	if (selected == Entity()) {
		return;
	}
	bool is_prefab = world.HasComponent<PrefabComponent>(selected);
	AddComponent(selected, data);
	bool is_camera = world.HasComponent<CameraComponent>(selected);

	if (ImGui::TreeNode("Label")) {
		if (world.HasComponent<LabelComponent>(selected)) {
			LabelComponent& label = world.GetComponent<LabelComponent>(selected);
			char* buf = new char[50];
			memcpy(buf, label.label.c_str(), label.label.size() + 1);
			ImGui::InputText("Label", buf, 50);
			ImGui::SameLine();
			ImGui::TextUnformatted(("(id: " + std::to_string(selected.id) + ")").c_str());
			label.label = buf;
			delete[] buf;

		}
		else if (ImGui::Button("Add label")) {
			world.SetComponent<LabelComponent>(selected, "Unknown");
		}
		ImGui::TreePop();
	}


	if (world.HasComponent<TransformComponent>(selected) && ImGui::TreeNode("Transform")) {
		TransformComponent& transform = world.GetComponent<TransformComponent>(selected);
		glm::vec3& translation = transform.translation;
		glm::vec3& size = transform.size;
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 rotation_back = rotation;

		if (ImGui::Button("Reset##0")) {
			transform.translation = glm::vec3(0.0f);
		}
		ImGui::SameLine();
		ImGui::DragFloat3("Translation", glm::value_ptr(translation));
		if (ImGui::Button("Reset##1")) {
			transform.size = glm::vec3(1.0f);
		}
		ImGui::SameLine();
		ImGui::DragFloat3("Scale", glm::value_ptr(size));
		if (ImGui::Button("Reset##2")) {
			transform.rotation = glm::quat(glm::vec3(0.0f));
		}
		ImGui::SameLine();
		ImGui::DragFloat3("Rotation", glm::value_ptr(rotation));

		if (rotation != rotation_back) {
			transform.rotation = transform.rotation * glm::quat(rotation / 180.0f * glm::pi<float>());
		}

		world.GetSceneGraph()->MarkEntityDirty(world.GetSceneGraph()->GetSceneGraphNode(selected));

		ImGui::TreePop();
	}

	if (is_camera && !is_prefab) {
		if (ImGui::TreeNode("Camera")) {
			auto& camera = world.GetComponent<CameraComponent>(selected);

			ImGui::SliderFloat("Field of View", &camera.fov, 0.0f, 180.0f);
			ImGui::DragFloat("ZFar", &camera.zFar);
			ImGui::DragFloat("ZNear", &camera.zNear);
			bool is_primary = Application::GetWorld().GetPrimaryEntity() == selected;
			bool primary_back = is_primary;
			ImGui::Checkbox("Primary", &is_primary);
			if (is_primary != primary_back && is_primary) {
				Application::GetWorld().SetPrimaryEntity(selected);
			}

			camera.UpdateProjectionMatrix();

			ImGui::TreePop();
		}
	}

	if (world.HasComponent<MeshComponent>(selected) && !is_prefab) {
		if (ImGui::TreeNode("Mesh")) {
			MeshComponent& mesh = world.GetComponent<MeshComponent>(selected);
			if (strlen(text_buffer) == 0) {
				text_buffer[0] = '\0';
				memcpy(text_buffer, mesh.file_path.c_str(), mesh.file_path.size() + 1);
			}
			bool enter = ImGui::InputText("Mesh path", text_buffer, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Reload") || enter) {
				try {
					mesh.ChangeMesh(FileManager::Get()->GetPath(text_buffer));
				}
				catch (std::runtime_error* e) {
					ImGui::OpenPopup(mesh_error_id);
					text_buffer[0] = '\0';
					memcpy(text_buffer, mesh.file_path.c_str(), mesh.file_path.size() + 1);
				}
			}
			ImGui::SameLine();
			if (mesh.mesh->GetMeshStatus() == Mesh_status::ERROR) {
				mesh.mesh = MeshManager::Get()->GetDefaultMesh();
				mesh.file_path = "Unknown";
				ImGui::OpenPopup(mesh_error_id);
				text_buffer[0] = '\0';
				memcpy(text_buffer, mesh.file_path.c_str(), mesh.file_path.size() + 1);
			}
			if (ImGui::Button("Set Selected")) {
				mesh.ChangeMesh(Editor::Get()->GetSelectedFilePath());
				memcpy(text_buffer, mesh.file_path.c_str(), mesh.file_path.size() + 1);
			}


			ImGui::TreePop();
		}
	}

	if (world.HasComponent<PrefabComponent>(selected) && data.prefab_path && !((char)data.show_flags & (char)ShowPropertyFlags::HIDE_PREFABS)) {
		if (ImGui::TreeNode("Prefab Setting")) {
			PrefabComponent& prefab = world.GetComponent<PrefabComponent>(selected);
			if (prefab.status == PrefabStatus::ERROR) {
				ImGui::OpenPopup(prefab_error_id);
				data.prefab_path[0] = '\0';
				memcpy(data.prefab_path, prefab.file_path.c_str(), prefab.file_path.size() + 1);
				prefab.status = PrefabStatus::UNINITIALIZED;
			}
			
			if (strlen(data.prefab_path) == 0) {
				data.prefab_path[0] = '\0';
				memcpy(data.prefab_path, prefab.file_path.c_str(), prefab.file_path.size() + 1);
			}
			bool enter = ImGui::InputText("Prefab path", data.prefab_path, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Reload") || enter) {
				try {
					prefab.file_path = data.prefab_path;
					Application::GetWorld().RemoveEntity(selected, RemoveEntityAction::RELOAD_PREFAB);
				}
				catch (std::runtime_error* e) {
					ImGui::OpenPopup(prefab_error_id);
					data.prefab_path[0] = '\0';
					memcpy(data.prefab_path, prefab.file_path.c_str(), prefab.file_path.size() + 1);
				}
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Set Selected")) {
				prefab.file_path = FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath());
				Application::GetWorld().RemoveEntity(selected, RemoveEntityAction::RELOAD_PREFAB);
				memcpy(data.prefab_path, prefab.file_path.c_str(), prefab.file_path.size() + 1);
			}
			
			if (ImGui::Button("OpenPrefabEditor")) {
				Editor::Get()->OpenPrefabEditorWindow(selected);
			}

			ImGui::TreePop();
		}
	}

}

void PropertiesPanel::AddComponent(Entity entity,const PropertiesPanel_persistent_data& data)
{
	auto& world = Application::GetWorld();
	if (ImGui::TreeNode("Add Component")) {
		bool has_mesh = world.HasComponent<MeshComponent>(entity);
		if (has_mesh) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Add Mesh Component")) {
			world.SetComponent<MeshComponent>(entity);
		}
		if (has_mesh) {
			ImGui::EndDisabled();
		} 
		ImGui::SameLine();
		if (!has_mesh) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Remove Mesh Component")) {
			world.RemoveComponent<MeshComponent>(entity);
			memcpy(data.mesh_file_buffer,"Unknown",strlen("Unknown")+1);
		}
		if (!has_mesh) {
			ImGui::EndDisabled();
		}


		bool has_camera = world.HasComponent<CameraComponent>(entity);
		if (has_camera) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Add Camera Component")) {
			world.SetComponent<CameraComponent>(entity);
		}
		if (has_camera) {
			ImGui::EndDisabled();
		}
		ImGui::SameLine();
		if (!has_camera) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Remove Camera Component")) {
			world.RemoveComponent<CameraComponent>(entity);
		}
		if (!has_camera) {
			ImGui::EndDisabled();
		}

		if (!((char)data.show_flags & (char)ShowPropertyFlags::HIDE_PREFABS)) {
			bool has_prefab = world.HasComponent<PrefabComponent>(entity);
			if (has_prefab) {
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Add Prefab Component")) {
				PrefabComponent comp;
				comp.status = PrefabStatus::UNINITIALIZED;
				world.SetComponent<PrefabComponent>(entity, comp);
				world.GetSceneGraph()->GetSceneGraphNode(entity)->state = world.GetSceneGraph()->GetSceneGraphNode(entity)->state | SceneNodeState::PREFAB;
			}
			if (has_prefab) {
				ImGui::EndDisabled();
			}
			ImGui::SameLine();
			if (!has_prefab) {
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Remove Prefab Component")) {
				world.GetSceneGraph()->GetSceneGraphNode(entity)->state = world.GetSceneGraph()->GetSceneGraphNode(entity)->state & ~SceneNodeState::PREFAB;
				world.GetComponent<PrefabComponent>(entity).file_path = "Unknown";
				world.RemoveEntity(entity, RemoveEntityAction::REMOVE_PREFABS);
				data.prefab_path[0] = '\0';
			}
			if (!has_prefab) {
				ImGui::EndDisabled();
			}
		}

		ImGui::TreePop();
	}
}
