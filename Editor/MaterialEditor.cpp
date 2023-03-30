#include "MaterialEditor.h"
#include <World/World.h>
#include <Renderer/Renderer.h>
#include <FileManager.h>
#include <Renderer/Renderer3D/Renderer3D.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Application.h>
#include <unordered_map>
#include <Editor/Editor.h>
#include <imgui.h>

MaterialEditor::MaterialEditor() : windows(), open_materials()
{

}

MaterialEditor::~MaterialEditor()
{

}

void MaterialEditor::Render()
{
	for (int i = 0; i < windows.size();i++) {
		RenderWinow(windows[i], i);
	}
}

void MaterialEditor::OpenEditorWinow(const std::string& material_path)
{
	if (open_materials.find(material_path) != open_materials.end()) return;
	MaterialEditorWindow window;
	window.mat = MaterialManager::Get()->GetMaterial(material_path);
	window.text_buffers = std::vector<char*>();
	for (int i = 0; i < window.mat->parameters.size(); i++) {
		window.text_buffers.push_back(nullptr);
	}
	window.path = material_path;
	windows.push_back(window);
	open_materials.insert(material_path);
}

void MaterialEditor::CloseMaterialWinow(int index)
{
	open_materials.erase(windows[index].path);
	windows.erase(windows.begin() + index);
	
}

void MaterialEditor::RenderWinow(MaterialEditorWindow& window, int current_index)
{
	using param_type = typename MaterialTemplate::MaterialTemplateParameterType;
	bool open = true;
	ImGui::SetNextWindowSize({ 800,600 }, ImGuiCond_Once);
	ImGui::Begin((std::string("Material Editor##") + window.path).c_str(), &open, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save Material")) {
				MaterialManager::Get()->SerializeMaterial(window.path, window.mat);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	int i = 0;
	for (auto& parameter : window.mat->parameters) {
		if (ImGui::TreeNode(parameter.name.c_str())) {
			bool enabled = !bool(parameter.flags & Material::MaterialParameter_flags::DEFAULT);
			bool enable_item = enabled;
			bool change = false;
			ImGui::Checkbox("Enabled", &enable_item);
			if (enabled != enable_item) {
				if (enable_item) {
					window.mat->ActivateParameter(parameter.name);
				}
				else {
					window.mat->DeactivateParameter(parameter.name);
				}
			}
			switch (parameter.type) {
			case param_type::INT:
				change = ImGui::DragInt("value", &std::get<int>(parameter.resource));
				break;
			case param_type::SCALAR:
				change = ImGui::DragFloat("value", &std::get<float>(parameter.resource));
				break;
			case param_type::VEC2:
				change = ImGui::DragFloat2("value", glm::value_ptr(std::get<glm::vec2>(parameter.resource)));
				break;
			case param_type::VEC3:
				change = ImGui::DragFloat3("value", glm::value_ptr(std::get<glm::vec3>(parameter.resource)));
				break;
			case param_type::VEC4:
				change = ImGui::DragFloat4("value", glm::value_ptr(std::get<glm::vec4>(parameter.resource)));
				break;
			case param_type::TEXTURE:
				std::string& current_path = std::get<Material::Texture_type>(parameter.resource).path;
				char* buffer;
				bool pressed = false;
				if (window.text_buffers[i]) {
					buffer = window.text_buffers[i];
				}
				else {
					window.text_buffers[i] = new char[200];
					buffer = window.text_buffers[i];
					strcpy(buffer, current_path.c_str());
				}
				if (enabled != enable_item) {
					strcpy(buffer, current_path.c_str());
				}
				pressed = ImGui::InputText("value", buffer,200, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SameLine();
				if (ImGui::Button("Set Selected")) {
					std::string file_exp_path = FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath());
					strcpy(buffer, file_exp_path.c_str());
					pressed = true;
				}
				if (ImGui::Button("Reload") || pressed) {
					window.mat->SetTexture(parameter.name, buffer);
				}

				break;
			
			}
			if (change) {
				parameter.flags |= Material::MaterialParameter_flags::DIRTY;
			}

			ImGui::TreePop();
		}
		i++;
	}

	ImGui::End();

	if (!open) {
		CloseMaterialWinow(current_index);
	}

}

MaterialEditorWindow::MaterialEditorWindow()
{
}

MaterialEditorWindow::~MaterialEditorWindow()
{
	for (char* buffer : text_buffers) {
		if (buffer) {
			delete buffer;
		}
	}
}
