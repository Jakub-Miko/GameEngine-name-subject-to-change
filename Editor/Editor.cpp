#include "Editor.h"
#include <Renderer/Renderer3D/MaterialManager.h>
#include <dependencies/imgui/imgui.h>
#include <dependencies/Additional/impl_custom_imgui_backend.h>
#include <dependencies/Additional/impl_custom_imgui_platform.h>
#include <dependencies/imgui/backends/imgui_impl_glfw.h>
#include <platform/GLFW/GlfwWindow.h>
#include <Application.h>
#include <fstream>
#include <FileManager.h>
#include <ImGuizmo.h>

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
	material_editor.reset();
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
				EnableEditor();
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
	ImGuizmo::BeginFrame();
	if (enabled) {

		//ImGui::ShowDemoWindow();
		
		auto save_id = ImGui::GetID("Save Dialog");
		auto load_id = ImGui::GetID("Load Dialog");
		auto mat_id = ImGui::GetID("Empty Material Dialog");
		auto import_id = ImGui::GetID("Import Mesh Dialog");
		auto Viewport_Settings_id = ImGui::GetID("Viewport Settings");
		auto import_text_id = ImGui::GetID("Import Texture Dialog");
		if (are_files_dropped) {
			are_files_dropped = false;
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern)) {
				ImGui::SetDragDropPayload("files", &drop_callback_strings, sizeof(std::vector<std::string>));
				ImGui::EndDragDropSource();
			}
		}
		
		if (ImGui::BeginPopupModal("Error")) {

			for (auto& error : error_messages) {
				ImGui::TextUnformatted(error.c_str());
			}

			ImGui::Separator();
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				error_messages.clear();
			}
			ImGui::EndPopup();
		}

		if (!error_messages.empty()) {
			ImGui::OpenPopup("Error");
		}

		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("Settings")) {
			if (ImGui::MenuItem("Viewport Settings")) {
				ImGui::OpenPopup(Viewport_Settings_id);
			};

			ImGui::EndMenu();
		}

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

			SceneScriptOptions();

			if (ImGui::MenuItem("Empty Scene")) {
				Application::GetWorld().LoadEmptyScene();
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Import")) {
			if (ImGui::MenuItem("Import Mesh")) {
				ImGui::OpenPopup(import_id);
			};

			if (ImGui::MenuItem("Import Texture")) {
				ImGui::OpenPopup(import_text_id);
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("Create Empty Material")) {
				ImGui::OpenPopup(mat_id);
			};

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Play")) {
			if (ImGui::MenuItem("Play Without editor")) {
				DisableEditor();
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

		if (ImGui::BeginPopupModal("Viewport Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

			ImGui::Checkbox("Spatial Index Visualization", &spatial_index_visualization);
			ImGui::Checkbox("Lighting Bounds Visualization", &light_bounds_visualization);

			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				file_dialog_text_buffer[0] = '\0';
			}
			ImGui::EndPopup();
		}

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
					Application::GetWorld().LoadSceneFromFile(file_dialog_text_buffer);
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

		bool extension_error = false;
		try {
			if (ImGui::BeginPopupModal("Empty Material Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				bool enter_pressed = ImGui::InputText("Material path", file_dialog_text_buffer, file_dialog_text_buffer_size, ImGuiInputTextFlags_EnterReturnsTrue);
				
				if (enter_pressed || ImGui::Button("Create")) {
					if(std::filesystem::path(FileManager::Get()->GetPath(file_dialog_text_buffer)).extension().generic_string() != ".mat") {
						extension_error = true;
						throw std::runtime_error("Extension Error");
					}
					auto mat = MaterialManager::Get()->CreateEmptyMaterial(file_dialog_text_buffer, ShaderManager::Get()->GetShader("shaders/GeometryPassShader.glsl"));
					material_editor->OpenEditorWinow(mat->GetFilePath());
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
			file_dialog_text_buffer[0] = '\0';
			ImGui::EndPopup();
			if (extension_error) {
				ImGui::OpenPopup("Error##materialextension");
			}
			else {
				ImGui::OpenPopup("Error##material");
			}
		}

		if (ImGui::BeginPopupModal("Error##material", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {


			ImGui::TextUnformatted("Could not create a material");
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal("Error##materialextension", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {


			ImGui::TextUnformatted("Material name must end in .mat");
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
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
		material_editor->Render();

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

void Editor::RenderDebugView(float delta_time)
{
	if (enabled && spatial_index_visualization) {
		Application::GetWorld().GetSpatialIndex().Visualize();
	}
	if (enabled && light_bounds_visualization) {
		
	}
}

void Editor::EditorError(const std::string& message)
{
	error_messages.push_back(message);
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

PrefabEditorWindow* Editor::GetOpenPrefabWindow(Entity ent)
{
	return prefab_editor->GetOpenWindow(ent);
}

void Editor::DisableEditor()
{
	Application::Get()->GetWindow()->AdjustWidowToDisabledEditor();
	enabled = false;
}

void Editor::EnableEditor()
{
	Application::Get()->GetWindow()->AdjustWidowToEnabledEditor();
	enabled = true;
}

Entity Editor::GetSelectedEntity() const
{
	if (Application::GetWorld().EntityExists(selected_entity)) {
		return selected_entity;
	}
	else {
		return Entity();
	}
}

void Editor::ViewportEnd()
{
	if (enabled) {
		viewport->EndViewportFrameBuffer();
	}
}

void Editor::CopyEntity(Entity ent)
{
	if (Application::GetWorld().EntityExists(ent)) {
		entity_clipboard = ent;
	}
}

void Editor::PasteEntity(Entity parent)
{
	SceneNode* parent_node = Application::GetWorld().GetSceneGraph()->GetSceneGraphNode(parent);
	if (parent_node && Application::GetWorld().EntityExists(entity_clipboard)) {
		if ((bool)(parent_node->state & (SceneNodeState::PREFAB_CHILD | SceneNodeState::PREFAB))) {
			Application::GetWorld().DuplicateEntityInPrefab(entity_clipboard, parent);
		}
		else {
			Application::GetWorld().DuplicateEntity(entity_clipboard, parent);
		}
	}
}

void Editor::Reset()
{
	properties_panel.reset(new PropertiesPanel());
	explorer.reset(new FileExplorer());
	scene_graph.reset(new SceneGraphViewer());
	viewport.reset(new Viewport());
	prefab_editor.reset(new PrefabEditor());
	material_editor.reset(new MaterialEditor());
	selected_entity = Entity();
}

void Editor::Refresh()
{
	properties_panel->Refresh();
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

void Editor::SceneScriptOptions()
{
	auto& world = Application::GetWorld();
	bool has_script = world.HasSceneScript();
	const SceneProxy& scene = world.GetCurrentSceneProxy();
	std::string temp_path = FileManager::Get()->GetTempFilePath(FileManager::Get()->GetPathHash(scene.scene_path) + ".lua");
	bool temp_file_exist = std::filesystem::exists(temp_path);
	if (ImGui::MenuItem(has_script ? "Edit scene script" : "Create scene script")) {
		std::ofstream file(temp_path);
		if (!file.is_open()) throw std::runtime_error("File " + temp_path + " doesn't exist");
		if (has_script) {
			file << FileManager::Get()->GetFileSection(FileManager::Get()->GetPath(scene.scene_path),"Script");
		}
		else {
			file << "function OnUpdate(delta_time) \n\nend\n";
		}
		file.close();
		Application::Get()->GetOsApi()->OpenFileInDefaultApp(temp_path);
	}
	if (temp_file_exist && ImGui::MenuItem("Apply scene script")) {
		std::string scene_string = FileManager::Get()->OpenFileRaw(scene.scene_path);
		std::string new_script = FileManager::Get()->OpenFileRaw(FileManager::Get()->GetRelativeFilePath(temp_path));
		FileManager::Get()->InsertOrReplaceSection(scene_string, new_script, "Script");
		std::ofstream file(FileManager::Get()->GetPath(scene.scene_path));
		if (!file.is_open()) throw std::runtime_error("File " + FileManager::Get()->GetPath(scene.scene_path) + " doesn't exist");
		file << scene_string;
		file.close();
		Application::GetWorld().ResetLuaEngine();
		Application::GetWorld().scene_lua_engine.RunString(new_script);
	}


}

Editor::Editor() : viewport(new Viewport), scene_graph(new SceneGraphViewer), properties_panel(new PropertiesPanel), explorer(new FileExplorer), prefab_editor(new PrefabEditor), material_editor(new MaterialEditor), error_messages()
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
