#include "Editor.h"
#include <dependencies/imgui/imgui.h>
#include <dependencies/Additional/impl_custom_imgui_backend.h>
#include <dependencies/imgui/backends/imgui_impl_glfw.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>
#include <fstream>
#include <FileManager.h>

Editor* Editor::instance = nullptr;

void Editor::Init()
{
	if (!instance) {
		instance = new Editor;
	}
}

void Editor::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

bool Editor::OnEvent(Event* e)
{
	EventDispacher dispatch(e);
	if (!enabled) {
		dispatch.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
			if (e->key_code == KeyCode::KEY_ESCAPE && e->press_type == KeyPressType::KEY_PRESS) {
				enabled = true;
			}

			return false;
			});
		return false;
	}
	
	if (!IsViewportFocused()) {
		return true;
	}
	else {
		dispatch.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
			if (e->key_code == KeyCode::KEY_ESCAPE && e->press_type == KeyPressType::KEY_PRESS) {
				ImGui::SetWindowFocus(NULL);
			}

			return false;
			});
	}


	return e->handled;

}

void Editor::Run()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (enabled) {

		//ImGui::ShowDemoWindow();

		auto save_id = ImGui::GetID("Save Dialog");
		auto load_id = ImGui::GetID("Load Dialog");
		auto import_id = ImGui::GetID("Import Mesh Dialog");
		if (are_files_dropped) {
			are_files_dropped = false;
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern)) {
				ImGui::SetDragDropPayload("files", &drop_callback_strings, sizeof(std::vector<std::string>));
				ImGui::EndDragDropSource();
			}
		}

		ImGui::BeginMainMenuBar();


		if (ImGui::BeginMenu("Workspace")) {
			if (ImGui::MenuItem("Save Workspace")) {
				ImGui::SaveIniSettingsToDisk("asset:workspace.ini"_path.c_str());
			};

			ImGui::EndMenu();
		}



		if (ImGui::BeginMenu("Scene")) {
			if (ImGui::MenuItem("Save Scene")) {
				ImGui::OpenPopup(save_id);
			};
			if (ImGui::MenuItem("Load Scene")) {
				ImGui::OpenPopup(load_id);
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Import")) {
			if (ImGui::MenuItem("Import Mesh")) {
				ImGui::OpenPopup(import_id);
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Play")) {
			if (ImGui::MenuItem("Play Without editor")) {
				enabled = false;
				Renderer::Get()->SetDefaultFrameBuffer();
				auto queue = Renderer::Get()->GetCommandQueue();
				auto list = Renderer::Get()->GetRenderCommandList();
				list->SetDefaultRenderTarget();
				queue->ExecuteRenderCommandList(list);
			};


			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		try {
			if (ImGui::BeginPopupModal("Save Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				if (ImGui::InputText("Filepath", file_dialog_text_buffer, file_dialog_text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Save")) {
					Application::GetWorld().SaveScene(FileManager::Get()->GetPath(file_dialog_text_buffer));
					ImGui::CloseCurrentPopup();
					file_dialog_text_buffer[0] = '\0';
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) {
					ImGui::CloseCurrentPopup();
					file_dialog_text_buffer[0] = '\0';
				}
				ImGui::EndPopup();
			}
		}
		catch (...) {
			ImGui::EndPopup();
			ImGui::OpenPopup("Error##save");
		}
		try {
			if (ImGui::BeginPopupModal("Load Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				if (ImGui::InputText("Filepath", file_dialog_text_buffer, file_dialog_text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Load")) {
					Application::GetWorld().LoadSceneFromFile(FileManager::Get()->GetPath(file_dialog_text_buffer));
					ImGui::CloseCurrentPopup();
					file_dialog_text_buffer[0] = '\0';
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) {
					ImGui::CloseCurrentPopup();
					file_dialog_text_buffer[0] = '\0';
				}
				ImGui::EndPopup();
			}
		}
		catch (...) {
			ImGui::EndPopup();
			ImGui::OpenPopup("Error##load");
		}

		if (ImGui::BeginPopupModal("Error##load", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {


			ImGui::TextUnformatted("Could not Load File");
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Error##save", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {


			ImGui::TextUnformatted("Could not Save File");
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}



		ImGui::DockSpaceOverViewport();



		scene_graph->Render();
		properties_panel->Render();
		explorer->Render();

		viewport->Render();
		if (!enabled) {
			is_viewport_focused = true;
		}
	}
	ImGui::Render();
	impl_custom_imgui_backend::DrawData(ImGui::GetDrawData());

}

void Editor::Render()
{
}

void Editor::ViewportBegin()
{
	if (enabled) {
		viewport->BeginViewportFrameBuffer();
	}
}

void Editor::ViewportEnd()
{
	if (enabled) {
		viewport->EndViewportFrameBuffer();
	}
}

void Editor::Reset()
{
	properties_panel.reset(new PropertiesPanel());
	explorer.reset(new FileExplorer());
	scene_graph.reset(new SceneGraphViewer());
	viewport.reset(new Viewport());
	selected_entity = Entity();
}

Editor* Editor::Get()
{
	return instance;
}

void Editor::DropCallback(int count, std::vector<std::string> files)
{
	Editor::Get()->drop_callback_strings = files;
	Editor::Get()->are_files_dropped = true;
}

Editor::Editor() : viewport(new Viewport), scene_graph(new SceneGraphViewer), properties_panel(new PropertiesPanel), explorer(new FileExplorer)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io_1 = ImGui::GetIO(); (void)io;
	file_dialog_text_buffer = new char[file_dialog_text_buffer_size];
	file_dialog_text_buffer[0] = '\0';
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOther(static_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), true);
	impl_custom_imgui_backend::Init();
	if (std::filesystem::exists("asset:workspace.ini"_path)) {
		ImGui::LoadIniSettingsFromDisk("asset:workspace.ini"_path.c_str());
	}

	
	io = &io_1;



	io->IniFilename = NULL;
	io->ConfigFlags = ImGuiConfigFlags_DockingEnable;


	
	Application::Get()->GetWindow()->RegistorDragAndDropCallback(&DropCallback);

}

Editor::~Editor()
{
	delete[] file_dialog_text_buffer;
	impl_custom_imgui_backend::Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
