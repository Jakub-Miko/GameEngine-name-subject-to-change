#pragma once
#include <Editor/Viewport.h>
#include <Editor/SceneGraphViewer.h>
#include <Editor/PropertiesPanel.h>
#include <Editor/FileExplorer.h>
#include <Events/Event.h>
#include <World/Entity.h>

struct ImGuiIO;

class Editor {
public:
	Editor(const Editor& ref) = delete;
	Editor(Editor&& ref) = delete;
	Editor& operator=(const Editor& ref) = delete;
	Editor& operator=(Editor&& ref) = delete;

	static void Init();
	static void Shutdown();

	bool OnEvent(Event* e);

	void Run();

	void Render();

	void ViewportBegin();

	const Viewport* GetViewport() const {
		return &viewport;
	}

	bool IsViewportFocused() const {
		return is_viewport_focused;
	}

	Entity GetSelectedEntity() const {
		return selected_entity;
	}

	std::string GetSelectedFilePath() const {
		return explorer.GetSelectedFilePath();
	}

	void SetSelectedEntity(Entity ent) {
		selected_entity = ent;
	}

	std::vector<std::string> GetDragAndDropFiles() const {
		return drop_callback_strings;
	}

	void ViewportEnd();

	void ResetFilesDropped() {
		are_files_dropped = false;
	}

	static Editor* Get();
private:
	
	static void DropCallback(int count, std::vector<std::string> files);

	std::vector<std::string> drop_callback_strings;
	bool are_files_dropped = false;

	static Editor* instance;
	friend Viewport;

	Entity selected_entity = Entity();

	bool is_viewport_focused = false;

	char* file_dialog_text_buffer;
	int file_dialog_text_buffer_size = 100;

	bool enabled = true;

	Viewport viewport;
	SceneGraphViewer scene_graph;
	PropertiesPanel properties_panel;
	FileExplorer explorer;

	ImGuiIO* io = nullptr;
	Editor();
	~Editor();

};