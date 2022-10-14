#include "PropertiesPanel.h"
#include <imgui.h>
#include <World/World.h>
#include <Editor/Editor.h>
#include <Application.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/PhysicsComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/PrefabComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/MeshComponent.h>
#include <FileManager.h>

PropertiesPanel::PropertiesPanel()
{
	text_buffer = new char[buffer_size];
	text_buffer[0] = '\0';
	prefab_path_buffer = new char[buffer_size];
	prefab_path_buffer[0] = '\0';
	material_file_buffer = new char[buffer_size];
	material_file_buffer[0] = '\0';
}

PropertiesPanel::~PropertiesPanel()
{
	delete[] text_buffer;
	delete[] prefab_path_buffer;
	delete[] material_file_buffer;
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
	data.material_file_buffer = material_file_buffer;

	if (selected != last_entity) {
		text_buffer[0] = '\0';
		prefab_path_buffer[0] = '\0';
		material_file_buffer[0] = '\0';
		last_entity = selected;
	}

	RenderProperties(selected, data);

	ImGui::End();

}

void PropertiesPanel::Refresh()
{
	text_buffer[0] = '\0';
	prefab_path_buffer[0] = '\0';
	material_file_buffer[0] = '\0';
}

void PropertiesPanel::RenderProperties(Entity entity, const PropertiesPanel_persistent_data& data)
{
	Entity selected = entity;
	World& world = Application::GetWorld();
	
	char* text_buffer = data.mesh_file_buffer;
	char* material_file_buffer = data.material_file_buffer;
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

	if (ImGui::BeginPopupModal("Error##material", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Invalid or corrupted Material file");

		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	int material_error_id = ImGui::GetID("Error##material");

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

	if (world.HasComponent<LightComponent>(selected) && ImGui::TreeNode("Light ")) {
		bool has_shadow = world.HasComponent<ShadowCasterComponent>(selected);
		glm::vec4 color = world.GetComponent<LightComponent>(selected).GetLightColor();
		ImGui::ColorPicker3("Light Color", glm::value_ptr(color));
		ImGui::DragFloat("Intensity", &glm::value_ptr(color)[3]);
		if (color != world.GetComponent<LightComponent>(selected).GetLightColor()) {
			LightComponent::SetLightColor(color, selected);
		}
		const char* light_types[] = { "DIRECTIONAL", "POINT"};
		int original = (int)world.GetComponent<LightComponent>(selected).type;
		int type = original;
		ImGui::Combo("Light Type", &type, light_types, 2);
		if (original != type) {
			LightComponent::ChangeType((LightType)type, selected);
		}
		if (original == (int)LightType::POINT) {
			glm::vec3 atten = world.GetComponent<LightComponent>(selected).GetAttenuation();
			ImGui::DragFloat("Attenuation Constant", &atten.r);
			ImGui::DragFloat("Attenuation Linear", &atten.g);
			ImGui::DragFloat("Attenuation Quadratic", &atten.b);
			if (atten != world.GetComponent<LightComponent>(selected).GetAttenuation()) {
				LightComponent::SetAttenuation(atten, selected);
			}
		}

		if (type == (int)LightType::DIRECTIONAL || type == (int)LightType::POINT) {
			bool shadows_enabled = has_shadow;
			ImGui::Checkbox("Enable Shadows", &shadows_enabled);
			if (has_shadow != shadows_enabled) {
				if (shadows_enabled) {
					//Enable Shadows
					world.SetComponent<ShadowCasterComponent>(selected, ShadowCasterComponent());
				}
				else {
					//Disable Shadows
					world.RemoveComponent<ShadowCasterComponent>(selected);
				}
			}
		}

		ImGui::TreePop();
	}

	if(world.HasComponent<ShadowCasterComponent>(selected) && ImGui::TreeNode("Shadow Casting")) {
		auto& shadow = world.GetComponent<ShadowCasterComponent>(selected);
		int res[2] = { shadow.res_x, shadow.res_y };
		if(ImGui::DragInt2("ShadowMapResolution", res) && (res[0] != shadow.res_x || res[1] != shadow.res_y)) {
			shadow.res_x = res[0];
			shadow.res_y = res[1];
			shadow.shadow_map.reset();
		}

		ImGui::DragFloat("Shadow Map Near Plane", &shadow.near_plane);
		ImGui::DragFloat("Shadow Map Far Plane", &shadow.far_plane);
		
		ImGui::TreePop();
	}

	if (world.HasComponent<PhysicsComponent>(selected) && ImGui::TreeNode("Physics")) {
		auto& phys_comp = world.GetComponent<PhysicsComponent>(selected);

		const char* shapes[2] = { "BOUNDING BOX","CONVEX HULL" };
		int current_shape = (int)phys_comp.shape_type;
		if (ImGui::Combo("Collision Shape", &current_shape, shapes, 2) && current_shape != (int)phys_comp.shape_type) {
			phys_comp.shape_type = (PhysicsShapeType)current_shape;
			Application::GetWorld().GetPhysicsEngine().RefreshObject(selected);
		}

		bool kinematic_checked = phys_comp.is_kinematic;
		ImGui::Checkbox("Kinematic", &kinematic_checked);
		if (kinematic_checked != phys_comp.is_kinematic) {
			phys_comp.is_kinematic = kinematic_checked;
			if (phys_comp.is_kinematic) {
				phys_comp.mass = 0.0f;
			}
			Application::GetWorld().GetPhysicsEngine().RefreshObject(selected);
		}
		float mass_input = phys_comp.mass;
		if (ImGui::DragFloat("Mass", &mass_input)) {
			if (mass_input != 0.0f) {
				phys_comp.is_kinematic = false;
			}
			phys_comp.mass = mass_input;
			Application::GetWorld().GetPhysicsEngine().RefreshObject(selected);
		};

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
				memcpy(text_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
			}
			bool enter = ImGui::InputText("Mesh path", text_buffer, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Reload") || enter) {
				try {
					mesh.ChangeMesh(FileManager::Get()->GetPath(text_buffer));
				}
				catch (std::runtime_error* e) {
					ImGui::OpenPopup(mesh_error_id);
					text_buffer[0] = '\0';
					memcpy(text_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
				}
			}
			ImGui::SameLine();
			if (mesh.GetMesh()->GetMeshStatus() == Mesh_status::ERROR) {
				mesh.ResetMesh();
				ImGui::OpenPopup(mesh_error_id);
				text_buffer[0] = '\0';
				memcpy(text_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
			}
			if (ImGui::Button("Set Selected")) {
				mesh.ChangeMesh(Editor::Get()->GetSelectedFilePath());
				memcpy(text_buffer, mesh.GetMeshPath().c_str(), mesh.GetMeshPath().size() + 1);
			}
			ImGui::Separator();
			if (strlen(material_file_buffer) == 0) {
				material_file_buffer[0] = '\0';
				memcpy(material_file_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
			}
			bool enter_material = ImGui::InputText("Material path", material_file_buffer, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Reload##material") || enter_material) {
				try {
					mesh.ChangeMaterial(material_file_buffer);
				}
				catch (std::runtime_error* e) {
					ImGui::OpenPopup(material_error_id);
					material_file_buffer[0] = '\0';
					memcpy(material_file_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Set Selected##material")) {
				mesh.ChangeMaterial(FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()));
				memcpy(material_file_buffer, mesh.GetMaterialPath().c_str(), mesh.GetMaterialPath().size() + 1);
			}
			if (mesh.GetMaterialStatus() == Material::Material_status::ERROR) {
				ImGui::OpenPopup(material_error_id);
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
				memcpy(data.prefab_path, prefab.GetFilePath().c_str(), prefab.GetFilePath().size() + 1);
				prefab.status = PrefabStatus::UNINITIALIZED;
			}
			
			if (strlen(data.prefab_path) == 0) {
				data.prefab_path[0] = '\0';
				memcpy(data.prefab_path, prefab.GetFilePath().c_str(), prefab.GetFilePath().size() + 1);
			}
			bool enter = ImGui::InputText("Prefab path", data.prefab_path, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Reload") || enter) {
				try {
					prefab.SetFilePath(data.prefab_path);
					Application::GetWorld().RemoveEntity(selected, RemoveEntityAction::RELOAD_PREFAB);
				}
				catch (std::runtime_error* e) {
					ImGui::OpenPopup(prefab_error_id);
					data.prefab_path[0] = '\0';
					memcpy(data.prefab_path, prefab.GetFilePath().c_str(), prefab.GetFilePath().size() + 1);
				}
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Set Selected")) {
				prefab.SetFilePath(FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()));
				Application::GetWorld().RemoveEntity(selected, RemoveEntityAction::RELOAD_PREFAB);
				memcpy(data.prefab_path, prefab.GetFilePath().c_str(), prefab.GetFilePath().size() + 1);
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
		bool has_light = world.HasComponent<LightComponent>(entity);
		bool has_bounds = world.HasComponent<BoundingVolumeComponent>(entity);
		bool has_shadow = world.HasComponent<ShadowCasterComponent>(entity);
		bool has_physics = world.HasComponent<PhysicsComponent>(entity);
		if (!has_light) {
		
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
				if (has_physics) {
					world.RemoveComponent<PhysicsComponent>(entity);
				}
				memcpy(data.mesh_file_buffer, "Unknown", strlen("Unknown") + 1);
			}
			if (!has_mesh) {
				ImGui::EndDisabled();
			}
		}

		if (!has_mesh) {
			if (has_light) {
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Add Light Component")) {
				if (has_bounds) {
					world.RemoveComponent<BoundingVolumeComponent>(entity);
				}
				world.SetComponent<LightComponent>(entity);
			}
			if (has_light) {
				ImGui::EndDisabled();
			}
			ImGui::SameLine();
			if (!has_light) {
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Remove Light Component")) {
				world.RemoveComponent<LightComponent>(entity);
			}
			if (!has_light) {
				ImGui::EndDisabled();
			}
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
				world.GetComponent<PrefabComponent>(entity).SetFilePath("Unknown");
				world.RemoveEntity(entity, RemoveEntityAction::REMOVE_PREFABS);
				data.prefab_path[0] = '\0';
			}
			if (!has_prefab) {
				ImGui::EndDisabled();
			}
		}

		if (has_mesh) {
			if (has_physics) {
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Add Physics Component")) {
				PhysicsComponent comp;
				comp.mass = 0.0f;
				comp.is_kinematic = true;
				comp.object_type = PhysicsObjectType::RIGID_BODY;
				comp.shape_type = PhysicsShapeType::BOUNDING_BOX;
				world.SetComponent<PhysicsComponent>(entity, comp);
			}
			if (has_physics) {
				ImGui::EndDisabled();
			}
			ImGui::SameLine();
			if (!has_physics) {
				ImGui::BeginDisabled();
			}
			if (ImGui::Button("Remove Physics Component")) {
				world.RemoveComponent<PhysicsComponent>(entity);
			}
			if (!has_physics) {
				ImGui::EndDisabled();
			}
		}

		ImGui::TreePop();
	}
}
