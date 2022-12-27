#pragma once
#include <Editor/Viewport.h>
#include <Editor/SceneGraphViewer.h>
#include <Editor/PropertiesPanel.h>
#include <Editor/FileExplorer.h>
#include <Editor/MaterialEditor.h>
#include <Events/Event.h>
#include <World/Entity.h>
#include <Editor/PrefabEditor.h>

struct ImGuiIO;

class Editor {
public:
	Editor(const Editor& ref) = delete;
	Editor(Editor&& ref) = delete;
	Editor& operator=(const Editor& ref) = delete;
	Editor& operator=(Editor&& ref) = delete;

	static void Init();
	static void Shutdown();

	void PreShutdown();

	bool OnEvent(Event* e);

	void Run();

	void RenderDebugView(float delta_time);

	void EditorError(const std::string& message);

	void Render();

	void ViewportBegin();

	void DisableEditor();

	void EnableEditor();

	const Viewport* GetViewport() const {
		return viewport.get();
	}

	bool IsViewportFocused() const {
		return is_viewport_focused;
	}

	bool IsEditorEnabled() const {
		return enabled;
	}

	Entity GetSelectedEntity() const {
		return selected_entity;
	}

	std::string GetSelectedFilePath() const {
		return explorer->GetSelectedFilePath();
	}

	void SetSelectedEntity(Entity ent) {
		selected_entity = ent;
	}

	std::vector<std::string> GetDragAndDropFiles() const {
		return drop_callback_strings;
	}

	void ViewportEnd();

	void OpenPrefabEditorWindow(Entity entity) {
		prefab_editor->OpenPrefabEditorWindow(entity);
	}

	void ClosePrefabEditorWindow(Entity entity) {
		prefab_editor->ClosePrefabEditorWindow(entity);
	}

	void OpenMaterialEditorWindow(const std::string& material_path) {
		material_editor->OpenEditorWinow(material_path);
	}


	void CloseMaterialEditorWindow(int index) {
		material_editor->CloseMaterialWinow(index);
	}

	void ResetFilesDropped() {
		are_files_dropped = false;
	}

	PropertiesPanel& GetPropertiesPanel() {
		return *properties_panel;
	}

	void Reset();

	void Refresh();

	static Editor* Get();
private:
	
	static void DropCallback(int count, std::vector<std::string> files);

	void SceneScriptOptions();

	std::vector<std::string> drop_callback_strings;
	bool are_files_dropped = false;

	static Editor* instance;
	friend Viewport;

	Entity selected_entity = Entity();

	bool is_viewport_focused = false;

	char* file_dialog_text_buffer;
	int file_dialog_text_buffer_size = 100;

	bool enabled = true;
	bool spatial_index_visualization = false;
	bool light_bounds_visualization = false;

	std::unique_ptr<Viewport> viewport;
	std::unique_ptr<MaterialEditor> material_editor;
	std::unique_ptr<PrefabEditor> prefab_editor;
	std::unique_ptr<SceneGraphViewer> scene_graph;
	std::unique_ptr<PropertiesPanel> properties_panel;
	std::unique_ptr<FileExplorer> explorer;

	std::list<std::string> error_messages;

	ImGuiIO* io = nullptr;
	Editor();
	~Editor();

};