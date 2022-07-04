#include "PrefabEditor.h"
#include <Editor/Editor.h>
#include <imgui.h>
#include <imgui_internal.h>


PrefabEditor::PrefabEditor() : open_windows()
{
}

PrefabEditor::~PrefabEditor()
{
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
	open_windows.push_back(PrefabEditorWindow{ entity });
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

void PrefabEditor::RenderWindow(PrefabEditorWindow& window)
{
	bool opened = true;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,0 });

	auto dock_id = ImGui::GetID(("Prefab Editor##" + std::to_string(window.entity.id)).c_str());
	ImGui::Begin(("Prefab Editor##" + std::to_string(window.entity.id)).c_str(),&opened);
	ImGui::PopStyleVar();
	auto id = dock_id;
	if (ImGui::DockBuilderGetNode(dock_id) == nullptr) {

		ImGui::DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);

		ImGui::DockBuilderSetNodeSize(dock_id, ImGui::GetContentRegionAvail());
		auto left = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Left, 0.5f, NULL, &dock_id);

		ImGui::DockBuilderDockWindow("Window2", left);
		ImGui::DockBuilderDockWindow("Window3", dock_id);
		ImGui::DockBuilderFinish(dock_id);
	}
	ImGui::DockSpace(id);
	ImGui::Begin("Window2",nullptr, ImGuiWindowFlags_None);
	ImGui::End();
	ImGui::Begin("Window3", nullptr, ImGuiWindowFlags_None);
	ImGui::End();

	ImGui::End();
	if (!opened) {
		ImGui::DockBuilderRemoveNode(dock_id);
		ClosePrefabEditorWindow(window.entity);
	}

}
