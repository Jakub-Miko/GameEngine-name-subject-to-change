#include "Editor.h"
#include <dependencies/imgui/imgui.h>
#include <dependencies/Additional/impl_custom_imgui_backend.h>
#include <dependencies/Additional/impl_custom_imgui_platform.h>
#include <dependencies/imgui/backends/imgui_impl_glfw.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>
#include <fstream>
#include <FileManager.h>

#ifdef OpenGL
#include <GLFW/glfw3.h>
#include <platform/OpenGL/OpenGLRenderCommandQueue.h>
#include <platform/OpenGL/OpenGLRenderCommand.h>
#endif

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

void Editor::PreShutdown()
{
	viewport.reset();
	delete[] file_dialog_text_buffer;
	ImGui_ImplGlfw_Shutdown();
	impl_custom_imgui_backend::PreShutdown();
	impl_custom_imgui_backend::Shutdown();
	impl_custom_imgui_platform::shutdown_custom_imgui_platform();
	ImGui::DestroyContext();
	
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
	impl_custom_imgui_backend::NewFrame();
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
				bool enter_pressed = ImGui::InputText("Filepath", file_dialog_text_buffer, file_dialog_text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SameLine();
				if (ImGui::Button("Set Selected")) {
					std::string path = FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath());
					memcpy(file_dialog_text_buffer, path.c_str(), std::min((int)path.size(), file_dialog_text_buffer_size));
					file_dialog_text_buffer[std::min((int)path.size(), file_dialog_text_buffer_size)] = '\0';
				}

				if (enter_pressed || ImGui::Button("Save")) {
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

				bool enter_pressed = ImGui::InputText("Filepath", file_dialog_text_buffer, file_dialog_text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SameLine();
				if (ImGui::Button("Set Selected")) {
					std::string path = FileManager::Get()->GetRelativeFilePath(Editor::Get()->GetSelectedFilePath());
					memcpy(file_dialog_text_buffer, path.c_str(), std::min((int)path.size(), file_dialog_text_buffer_size));
					file_dialog_text_buffer[std::min((int)path.size(), file_dialog_text_buffer_size)] = '\0';
				}

				if (enter_pressed || ImGui::Button("Load")) {
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
		prefab_editor->Render();

		viewport->Render();
		if (!enabled) {
			is_viewport_focused = true;
		}
	}
	ImGui::Render();

	impl_custom_imgui_backend::DrawData(ImGui::GetDrawData());
#ifdef OpenGL
	
	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//NOTE: This synchronization is needed to keep the editor windows in sync but slows the engine down so its disabled when the editor is disabled. As for why its needed i have no idea.
		//IDEA: Maybe if I delay execution of UpdatePlatformWindows to after the current command queue I might not need this thing, but I cant really save previous window states unless I modify imgui.cpp.
		//another option is to delay window callbacks. Specificaly GetWindowPos and SetWindowPos in PlatformIO


		
	
		impl_custom_imgui_platform::UpdatePlatformWindows();
			
	}
#endif

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

Editor::Editor() : viewport(new Viewport), scene_graph(new SceneGraphViewer), properties_panel(new PropertiesPanel), explorer(new FileExplorer), prefab_editor(new PrefabEditor)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io_1 = ImGui::GetIO(); (void)io;
	file_dialog_text_buffer = new char[file_dialog_text_buffer_size];
	file_dialog_text_buffer[0] = '\0';
	ImGui::StyleColorsDark();
	io_1.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui_ImplGlfw_InitForOther(static_cast<GlfwWindow*>(Application::Get()->GetWindow())->GetHandle(), true);
	impl_custom_imgui_platform::init_custom_imgui_platform();
	impl_custom_imgui_backend::Init();
	if (std::filesystem::exists("asset:workspace.ini"_path)) {
		ImGui::LoadIniSettingsFromDisk("asset:workspace.ini"_path.c_str());
	}

	
	io = &io_1;



	io->IniFilename = NULL;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;


	
	Application::Get()->GetWindow()->RegistorDragAndDropCallback(&DropCallback);

}

Editor::~Editor()
{

}
