#pragma once
#include <World/Entity.h>

enum class ShowPropertyFlags {
	NONE = 0, HIDE_PREFABS = 1
};

struct PropertiesPanel_persistent_data {
	char* mesh_file_buffer;
	char* material_file_buffer;
	char* prefab_path;
	int buffer_size;
	ShowPropertyFlags show_flags = ShowPropertyFlags::NONE;
};

class PropertiesPanel {
public:
	PropertiesPanel();
	~PropertiesPanel();

	void Render();

	void Refresh();

	static void RenderProperties(Entity entity, const PropertiesPanel_persistent_data& data);

private:

	static void AddComponent(Entity entity, const PropertiesPanel_persistent_data& data);

	char* text_buffer = nullptr;
	char* prefab_path_buffer = nullptr;
	char* material_file_buffer = nullptr;
	int buffer_size = 200;
	Entity last_entity = Entity();

};