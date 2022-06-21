#include "FileExplorer.h"
#include <imgui.h>
#include <FileManager.h>
#include <ConfigManager.h>
#include <fstream>
#include <filesystem>
#include <Application.h>
#include <Window.h>

FileExplorer::FileExplorer() : current_path("asset:"_path)
{

}

FileExplorer::~FileExplorer()
{
}

void FileExplorer::Render()
{
	ImGui::Begin("File Explorer");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5,5 });
	float file_icon_size = Application::Get()->GetWindow()->GetProperties().resolution_x / 25;
	bool disabled = current_path + std::string("/") == ""_path;
	if (disabled) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Back")) {
		current_path = std::filesystem::path(current_path).parent_path().generic_string();
	}

	if (disabled) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	if (ImGui::Button("Assets")) {
		current_path = "asset:"_path;
	}

	ImGui::Separator();
	int id = 0;
	auto iter = std::filesystem::directory_iterator(current_path);

	std::vector<std::filesystem::directory_entry> files;
	std::vector<std::filesystem::directory_entry> directories;
	files.reserve(20);
	directories.reserve(20);
	for (auto entry : iter) {
		if (iter->is_regular_file()) {
			files.push_back(entry);
		}
		else if (iter->is_directory()) {
			directories.push_back(entry);
		}
	}



	ImGuiStyle& style = ImGui::GetStyle();
	float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

	for (auto directory : directories) {
		std::string name = directory.path().filename().generic_string();
		float last_button_x2 = ImGui::GetItemRectMax().x;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0,0.6,1.0,1.0 });
		if (ImGui::Button(name.c_str(), ImVec2{ file_icon_size , file_icon_size })) {
			current_path = directory.path().generic_string();
		};
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(name.c_str());
		last_button_x2 = ImGui::GetItemRectMax().x;

		id++;
		float next_button_x2 = last_button_x2 + style.ItemSpacing.x + file_icon_size;
		if (next_button_x2 < window_visible_x2)
			ImGui::SameLine();
	}

	for (auto file : files) {
		std::string name = file.path().filename().generic_string();
		float last_button_x2 = ImGui::GetItemRectMax().x;
		if (ImGui::Button(name.c_str(), ImVec2{ file_icon_size , file_icon_size })) {
			if (!Application::Get()->GetOsApi()->OpenFileInDefaultApp(file.path().generic_string())) {
				ImGui::OpenPopup("File not opened");
			}
		};
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(name.c_str());
		last_button_x2 = ImGui::GetItemRectMax().x;

		id++;
		float next_button_x2 = last_button_x2 + style.ItemSpacing.x + file_icon_size;
		if (next_button_x2 < window_visible_x2)
			ImGui::SameLine();
	}

	if (ImGui::BeginPopupModal("File not opened")) {

		ImGui::Text("File could not be opened");

		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	ImGui::End();
}
