#include "PrefabEditor.h"
#include <Editor/Editor.h>
#include <sstream>
#include <World/EntityManager.h>
#include <World/Components/SerializableComponent.h>
#include <FileManager.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <Application.h>
#include <World/Components/PrefabComponent.h>
#include <World/World.h>

PrefabEditor::PrefabEditor() : open_windows()
{
	save_prefab_buffer = new char[buffer_size];
	save_prefab_buffer[0] = '\0';
}

PrefabEditor::~PrefabEditor()
{
	if (save_prefab_buffer) {
		delete[] save_prefab_buffer;
	}
}

void PrefabEditor::Render()
{
	for (auto& window : open_windows) {
		RenderWindow(window);
	}
}

void PrefabEditor::OpenPrefabEditorWindow(Entity entity)
{
	for (auto& window : open_windows) {
		if (window.entity == entity) {
			return;
		}
	}
	PrefabEditorWindow window;
	window.entity = entity;
	window.buffer_size = 200;
	window.mesh_path = new char[window.buffer_size];
	window.material_path = new char[window.buffer_size];
	window.last_entity = Entity();
	window.selected_entity = Entity();
	window.mesh_path[0] = '\0';
	window.material_path[0] = '\0';

	open_windows.push_back(std::move(window));
}

void PrefabEditor::ClosePrefabEditorWindow(Entity entity)
{
	for (auto it = open_windows.begin(); it < open_windows.end(); ++it) {
		if (it->entity == entity) {
			open_windows.erase(it);
			break;
		}
	}
}



void PrefabEditor::PrefabSceneGraph(PrefabEditorWindow& window)
{
	ImGui::Begin(("SceneGraph##" + std::to_string(window.entity.id)).c_str(), nullptr, ImGuiWindowFlags_None);
	if (ImGui::Button("Create Empty Entity")) {
		Entity ent;
		if (window.selected_entity != Entity()) {
			ent = Application::GetWorld().CreateEntity<PrefabChildEntityType>(window.selected_entity, true);
		}
		else {
			ent = Application::GetWorld().CreateEntity<PrefabChildEntityType>(window.entity, true);
		}
		window.selected_entity = ent;
	}

	if (ImGui::Button("Delete Entity") && window.selected_entity != Entity()) {
		Application::GetWorld().RemoveEntity(window.selected_entity);
		window.selected_entity = Entity();
	}


	ImGui::Separator();
	
	
	std::string name;
	Entity entity = window.entity;
	SceneNode* first_child = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(Application::GetWorld().GetComponent<PrefabComponent>(entity).first_child);

	ImGui::PushID(std::to_string(entity.id).c_str());

	if (Application::GetWorld().HasComponent<LabelComponent>(entity)) {
		name = Application::GetWorld().GetComponent<LabelComponent>(entity).label;
	} 
	else {
		name = std::to_string(entity.id);
	}

	while (first_child) {
		PrefabSceneGraphNode(first_child, window);
		first_child = first_child->next;
	}
	ImGui::PopID();

	auto pos = ImGui::GetCursorPos();
	ImGui::Dummy(ImVec2{ ImGui::GetContentRegionAvail().x,std::max(ImGui::GetContentRegionAvail().y,200.0f) });
	if (ImGui::IsItemClicked()) {
		window.selected_entity = Entity();
	}
	ImGui::SetCursorPos(pos);
	
	ImGui::End();
}

void PrefabEditor::PrefabSceneGraphNode(SceneNode* entity, PrefabEditorWindow& window)
{
	
	std::string name;
	ImGui::PushID(std::to_string(entity->entity.id).c_str());

	if (Application::GetWorld().HasComponent<LabelComponent>(entity->entity)) {
		name = Application::GetWorld().GetComponent<LabelComponent>(entity->entity).label;
	}
	else {
		name = std::to_string(entity->entity.id);
	}

	auto node_flags = entity->first_child == nullptr ? ImGuiTreeNodeFlags_Leaf : 0;
	node_flags |= ImGuiTreeNodeFlags_SpanFullWidth;
	node_flags |= window.selected_entity == entity->entity ? ImGuiTreeNodeFlags_Selected : 0;
	node_flags |= ImGuiTreeNodeFlags_OpenOnArrow;

	auto tree_node = ImGui::TreeNodeEx(name.c_str(), node_flags);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
		window.selected_entity = entity->entity;
	}
	if (tree_node) {
		SceneNode* current = entity->first_child;

		while (current) {
			PrefabSceneGraphNode(current, window);
			current = current->next;
		}
		ImGui::TreePop();
	}
	ImGui::PopID();
}

void PrefabEditor::PrefabPropertiesPanel(PrefabEditorWindow& window)
{
	ImGui::Begin(("Properties Panel##" + std::to_string(window.entity.id)).c_str(), nullptr, ImGuiWindowFlags_None);
	
	PropertiesPanel_persistent_data data = Editor::Get()->GetPropertiesPanel().CreateDefaultProperties();
	data.mesh_file_buffer = window.mesh_path;
	data.material_file_buffer = window.material_path;
	data.buffer_size = window.buffer_size;
	data.show_flags = ShowPropertyFlags::HIDE_PREFABS;
	if (window.selected_entity != window.last_entity) {
		window.mesh_path[0] = '\0';
		window.material_path[0] = '\0';
		window.last_entity = window.selected_entity;
	}

	PropertiesPanel::RenderProperties(window.selected_entity, data);

	ImGui::End();
}

void PrefabEditor::RenderWindow(PrefabEditorWindow& window)
{
	bool opened = true;
	auto& prefab_comp = Application::GetWorld().GetComponent<PrefabComponent>(window.entity);
	EntityTemplate ent_template = EntityManager::Get()->GetEntitySignature(prefab_comp.GetFilePath());
	auto file_name = std::filesystem::path(FileManager::Get()->GetPath(prefab_comp.GetFilePath())).filename().generic_string();
	auto temp_file_name = FileManager::Get()->GetTempFilePath(file_name + ".lua");
	bool temp_file_exists = std::filesystem::exists(std::filesystem::path(temp_file_name));
	auto temp_file_inline_name = FileManager::Get()->GetTempFilePath("inline_" + file_name + ".lua");
	bool temp_file_inline_exists = std::filesystem::exists(std::filesystem::path(temp_file_inline_name));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });
	auto dock_id = ImGui::GetID(("Prefab Editor##" + std::to_string(window.entity.id)).c_str());
	ImGui::SetNextWindowSize({ 800,600 }, ImGuiCond_Once);
	ImGui::Begin(("Prefab Editor##" + std::to_string(window.entity.id)).c_str(),&opened, ImGuiWindowFlags_MenuBar);
	auto save_prefab_popup_id = ImGui::GetID("Save Prefab");
	ImGui::PopStyleVar();
	if (ImGui::BeginPopupModal("Save Prefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		bool enter_pressed = ImGui::InputText("File path", save_prefab_buffer, buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (ImGui::Button("Set Selected")) {
			memcpy(save_prefab_buffer, FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath()).c_str(), Editor::Get()->GetSelectedFilePath().size() + 1);
		}
		std::string actual_path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(save_prefab_buffer));
		ImGui::Separator();
		if (ImGui::Button("Save and Reload") || enter_pressed) {
			Application::GetWorld().SerializePrefab(window.entity, FileManager::Get()->GetPath(actual_path));
			auto& prefab = Application::GetWorld().GetComponent<PrefabComponent>(window.entity);
			if (prefab.status == PrefabStatus::UNINITIALIZED) {
				prefab.SetFilePath(actual_path);
				prefab.status = PrefabStatus::OK;
				Editor::Get()->Refresh();
			}
			Application::GetWorld().RemoveEntity(window.entity, RemoveEntityAction::RELOAD_ALL_PREFABS_OF_THIS_TYPE);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save prefab")) {
				save_prefab_buffer[0] = '\0';
				ImGui::OpenPopup(save_prefab_popup_id);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Script")) {
			if (ImGui::MenuItem(ent_template.construction_script.empty() ? "Add Construction Script" : "Edit Construction Script")) {
				std::ofstream temp_file(temp_file_name);
				if (!temp_file.is_open()) throw std::runtime_error("File " + temp_file_name + " could not be created!");
				temp_file << ent_template.construction_script;
				temp_file.close();
				Application::Get()->GetOsApi()->OpenFileInDefaultApp(temp_file_name);
			}
			if (ImGui::MenuItem(ent_template.inline_script.empty() ? "Add Inline Script" : "Edit Inline Script")) {
				std::ofstream temp_file(temp_file_inline_name);
				if (!temp_file.is_open()) throw std::runtime_error("File " + temp_file_inline_name + " could not be created!");
				temp_file << ent_template.inline_script;
				temp_file.close();
				Application::Get()->GetOsApi()->OpenFileInDefaultApp(temp_file_inline_name);
			}
			if (temp_file_exists) {
				if (ImGui::MenuItem("Apply Construction Script")) {
					std::ifstream temp_file(temp_file_name);
					if (!temp_file.is_open()) throw std::runtime_error("File " + temp_file_name + " could not be opened!");
					std::stringstream sstream;
					sstream << temp_file.rdbuf();
					EntityManager::Get()->AddConstructionScriptToPrefab(prefab_comp.GetFilePath(), sstream.str());

					temp_file.close();
				}
			}
			if (temp_file_inline_exists) {
				if (ImGui::MenuItem("Apply Inline Script")) {
					std::ifstream temp_file(temp_file_inline_name);
					if (!temp_file.is_open()) throw std::runtime_error("File " + temp_file_inline_name + " could not be opened!");
					std::stringstream sstream;
					sstream << temp_file.rdbuf();
					EntityManager::Get()->AddInlineScriptToPrefab(prefab_comp.GetFilePath(), sstream.str());

					temp_file.close();
				}
			}
			if (!ent_template.construction_script.empty()) {
				if (ImGui::MenuItem("Remove Construction Script")) {
					EntityManager::Get()->RemoveConstructionScriptToPrefab(prefab_comp.GetFilePath());
					if (temp_file_exists) {
						std::filesystem::remove(temp_file_name);
					}
				}
			}
			if (!ent_template.inline_script.empty()) {
				if (ImGui::MenuItem("Remove Inline Script")) {
					EntityManager::Get()->RemoveInlineScriptToPrefab(prefab_comp.GetFilePath());
					if (temp_file_inline_exists) {
						std::filesystem::remove(temp_file_inline_name);
					}
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	auto id = dock_id;
	if (ImGui::DockBuilderGetNode(dock_id) == nullptr) {

		ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);

		ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetContentRegionAvail());

		auto left = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.5f, NULL, &dock_id);

		ImGui::DockBuilderDockWindow(("SceneGraph##" + std::to_string(window.entity.id)).c_str(), left);
		ImGui::DockBuilderDockWindow(("Properties Panel##" + std::to_string(window.entity.id)).c_str(), dock_id);
		ImGui::DockBuilderFinish(dock_id);
	}
	ImGui::DockSpace(id);
	ImGui::End();


	PrefabSceneGraph(window);


	PrefabPropertiesPanel(window);

	if (!opened) {
		ImGui::DockBuilderRemoveNode(dock_id);
		ClosePrefabEditorWindow(window.entity);
	}

}
