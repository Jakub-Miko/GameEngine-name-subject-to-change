#include "PropertiesPanel.h"
#include <imgui.h>
#include <World/World.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/MeshComponent.h>
#include <FileManager.h>

PropertiesPanel::PropertiesPanel()
{
	text_buffer = new char[buffer_size];
	text_buffer[0] = '\0';
}

PropertiesPanel::~PropertiesPanel()
{
	delete[] text_buffer;
}

void PropertiesPanel::Render()
{
	Entity selected = Editor::Get()->GetSelectedEntity();
	World& world = Application::GetWorld();
	int mesh_error_id = ImGui::GetID("Error##mesh");

	if (ImGui::BeginPopupModal("Error##mesh", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Invalid or corrupted Mesh file");

		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	
	
	ImGui::Begin("Properties");


	if (selected == Entity()) {
		ImGui::End();
		return;
	}
	bool is_prefab = world.HasComponent<LoadedComponent>(selected);
	if (!is_prefab) {
		AddComponent();
	}
	bool is_camera = world.HasComponent<CameraComponent>(selected);

	if (ImGui::TreeNode("Label")) {
		if (world.HasComponent<LabelComponent>(selected)) {
			LabelComponent& label = world.GetComponent<LabelComponent>(selected);
			char* buf = new char[50];
			memcpy(buf, label.label.c_str(), label.label.size() + 1);
			ImGui::InputText("Label", buf, 50);
			label.label = buf;
			delete[] buf;

		} else if (ImGui::Button("Add label")) {
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
			if (last_entity != selected) {
				text_buffer[0] = '\0';
				memcpy(text_buffer, mesh.file_path.c_str(), mesh.file_path.size() + 1);
				last_entity = selected;
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

	if(world.HasComponent<LoadedComponent>(selected)) {
		if (ImGui::TreeNode("Prefab Setting")) {
			if (ImGui::Button("OpenPrefabEditor")) {
				Editor::Get()->OpenPrefabEditorWindow(selected);
			}

			ImGui::TreePop();
		}
	}
	
	ImGui::End();

}

void PropertiesPanel::AddComponent()
{
	auto& world = Application::GetWorld();
	if (ImGui::TreeNode("Add Component")) {
		bool has_mesh = world.HasComponent<MeshComponent>(Editor::Get()->GetSelectedEntity());
		if (has_mesh) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Add Mesh Component")) {
			world.SetComponent<MeshComponent>(Editor::Get()->GetSelectedEntity());	
		}
		if (has_mesh) {
			ImGui::EndDisabled();
		} 
		ImGui::SameLine();
		if (!has_mesh) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Remove Mesh Component")) {
			world.RemoveComponent<MeshComponent>(Editor::Get()->GetSelectedEntity());
			memcpy(text_buffer,"Unknown",strlen("Unknown")+1);
		}
		if (!has_mesh) {
			ImGui::EndDisabled();
		}


		bool has_camera = world.HasComponent<CameraComponent>(Editor::Get()->GetSelectedEntity());
		if (has_camera) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Add Camera Component")) {
			world.SetComponent<CameraComponent>(Editor::Get()->GetSelectedEntity());
		}
		if (has_camera) {
			ImGui::EndDisabled();
		}
		ImGui::SameLine();
		if (!has_camera) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Remove Camera Component")) {
			world.RemoveComponent<CameraComponent>(Editor::Get()->GetSelectedEntity());
		}
		if (!has_camera) {
			ImGui::EndDisabled();
		}
		ImGui::TreePop();
	}
}
