#include "FileExplorer.h"
#include <imgui.h>
#include <FileManager.h>
#include <ConfigManager.h>
#include <fstream>
#include <filesystem>
#include <Application.h>
#include <Editor/Editor.h>
#include <Window.h>
#include <Renderer/MeshManager.h>

FileExplorer::FileExplorer() : current_path("asset:"_path)
{
	import_dest_buffer = new char[text_buffer_size];
	import_source_buffer = new char[text_buffer_size];
	import_dest_buffer[0] = '\0';
	import_source_buffer[0] = '\0';
}

FileExplorer::~FileExplorer()
{
	delete[] import_dest_buffer;
	delete[] import_source_buffer;
}

void FileExplorer::Render()
{
	if (import_id == 0) {
		import_id = ImGui::GetID("Import Mesh Dialog");
	}
	
	if (ImGui::BeginPopupModal("Error##import", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {


		ImGui::TextUnformatted("Could not Import File");
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	try {
		if (ImGui::BeginPopupModal("Import Mesh Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

			bool source_enter = ImGui::InputText("Source_file", import_source_buffer, text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			bool dest_enter = ImGui::InputText("Destination_file", import_dest_buffer, text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
			
			if (source_enter) {
				ImGui::SetKeyboardFocusHere(-1);
			}
			
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				import_dest_buffer[0] = '\0';
				import_source_buffer[0] = '\0';
			}
			ImGui::SameLine();
			if (ImGui::Button("Import") | dest_enter) {
				std::string source_absolute = FileManager::Get()->GetPath(import_source_buffer);
				if (!(std::filesystem::exists(std::filesystem::path(source_absolute)) && std::filesystem::path(source_absolute).extension() == ".obj")) throw std::runtime_error("Source file either doesn't exist or is not an .obj file");
				std::string dest_absolute = FileManager::Get()->GetPath(import_dest_buffer);

				MeshManager::Get()->MakeMeshFromObjectFile(source_absolute, dest_absolute, *VertexLayoutFactory<MeshPreset>::GetLayout());

				ImGui::CloseCurrentPopup();
				import_dest_buffer[0] = '\0';
				import_source_buffer[0] = '\0';
			}
			ImGui::EndPopup();
		}
	}
	catch (...) {
		ImGui::EndPopup();
		ImGui::OpenPopup("Error##import");
	}
	
	
	
	auto drag_error_id = ImGui::GetID("Error##drag");
	if (ImGui::BeginPopupModal("Error##drag", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImGui::Text("Multiple file import currently isn't supported");

		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}


	ImGui::Begin("File Explorer");
	ImGui::SetCursorPos({ ImGui::GetStyle().WindowPadding.x ,ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y });
	ImGui::Dummy(ImGui::GetWindowSize());
	if (ImGui::BeginDragDropTarget()) {
		if (auto payload = ImGui::AcceptDragDropPayload("files")) {
			std::vector<std::string> files = *static_cast<std::vector<std::string>*>(payload->Data);
			if (files.size() != 1) {
				ImGui::OpenPopup(drag_error_id);
			}
			else {
				if (std::filesystem::path(files[0]).extension() == ".obj") {
					std::string file_name = std::filesystem::path(files[0]).filename().generic_string();
					auto find = file_name.find("obj");
					file_name = file_name.replace(find, 3, "mesh");
					std::string out_path = "absolute:" + std::filesystem::absolute(std::filesystem::path(current_path)).generic_string() + "/" + file_name;

					OpenImportDialog("absolute:" + files[0], out_path);

				}
			}
		}


		ImGui::EndDragDropTarget();
	}
	
	ImGui::SetCursorPos({ ImGui::GetStyle().WindowPadding.x ,ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y });

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5,5 });
	float file_icon_size = Application::Get()->GetWindow()->GetProperties().resolution_x / 25;
	bool disabled = current_path + std::string("/") == ""_path;
	if (disabled) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Back")) {
		current_path = std::filesystem::path(current_path).parent_path().generic_string();
		selected_path = "Unknown";
	}

	if (disabled) {
		ImGui::EndDisabled();
	}

	ImGui::SameLine();
	if (ImGui::Button("Assets")) {
		current_path = "asset:"_path;
		selected_path = "Unknown";
	}

	ImGui::Text((std::string("Selected file: ") + selected_path).c_str());
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip(selected_path.c_str());
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
			selected_path = "Unknown";
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
		ImVec4 color = ImVec4{ 0.0f,0.0f, 1.0f, 1.0f };
		if (file.path().generic_string() == selected_path) {
			color = ImVec4{ 0.3f,0.3f, 1.0f, 1.0f };
		}
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		if (ImGui::Button(name.c_str(), ImVec2{ file_icon_size , file_icon_size })) {
			if (file.path().generic_string() == selected_path) {
				if (std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::steady_clock::now() - time_selected)).count() < 500) {
					if (!Application::Get()->GetOsApi()->OpenFileInDefaultApp(file.path().generic_string())) {
						ImGui::OpenPopup("File not opened");
					}
				}
				else
				{
					time_selected = std::chrono::steady_clock::now();
				}
			}
			else {
				selected_path = file.path().generic_string();
				time_selected = std::chrono::steady_clock::now();
			}
		}
		ImGui::PopStyleColor();
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

void FileExplorer::OpenImportDialog(const std::string source_relative_path_template, const std::string destination_relative_path_template){

	memcpy(import_source_buffer, source_relative_path_template.c_str(), std::min((int)source_relative_path_template.size() + 1, text_buffer_size));
	memcpy(import_dest_buffer, destination_relative_path_template.c_str(), std::min((int)destination_relative_path_template.size() + 1, text_buffer_size));
	ImGui::OpenPopup(import_id);
}
