#pragma once
#include <Editor/Viewport.h>
#include <Editor/SceneGraphViewer.h>
#include <Editor/PropertiesPanel.h>
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

	void SetSelectedEntity(Entity ent) {
		selected_entity = ent;
	}

	void ViewportEnd();

	static Editor* Get();
private:
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

	ImGuiIO* io = nullptr;
	Editor();
	~Editor();

};