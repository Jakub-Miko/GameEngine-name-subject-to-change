#pragma once 
#include <string>
#include <World/Entity.h>
#include <vector>

struct PrefabEditorWindow {
	Entity entity;
	Entity selected_entity = Entity();
	char* mesh_path = nullptr;
	int buffer_size = 200;

	Entity last_entity = Entity();

	PrefabEditorWindow() = default;
	PrefabEditorWindow(PrefabEditorWindow&& other) : entity(other.entity), selected_entity(other.selected_entity), mesh_path(other.mesh_path), buffer_size(other.buffer_size) {
		other.mesh_path = nullptr;
	}

	PrefabEditorWindow& operator=(PrefabEditorWindow&& other) {
		entity = other.entity;
		selected_entity = other.selected_entity;
		mesh_path = other.mesh_path;
		buffer_size = other.buffer_size;
		last_entity = other.last_entity;
		other.mesh_path = nullptr;
		return *this;
	}

	~PrefabEditorWindow() {
		if (mesh_path) {
			delete[] mesh_path;
		}
	}

};

struct SceneNode;

class PrefabEditor {
public:
	PrefabEditor();
	~PrefabEditor();

	void Render();

	void OpenPrefabEditorWindow(Entity entity);

	void ClosePrefabEditorWindow(Entity entity);

	
private:

	void PrefabSceneGraph(PrefabEditorWindow& window);

	void PrefabSceneGraphNode(SceneNode* entity,PrefabEditorWindow& window);

	void PrefabPropertiesPanel(PrefabEditorWindow& window);

	void RenderWindow(PrefabEditorWindow& window);

	std::vector<PrefabEditorWindow> open_windows;
	int buffer_size = 200;
	char* save_prefab_buffer = nullptr;
};