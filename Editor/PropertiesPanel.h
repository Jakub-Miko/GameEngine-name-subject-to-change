#pragma once
#include <World/Entity.h>
#include <type_traits>

class PropertiesPanelEntry {
public:
	PropertiesPanelEntry(const std::string& name) : name(name) {}

	PropertiesPanelEntry(const PropertiesPanelEntry& other) : name(name) {}

	virtual void RenderPanel(Entity ent) = 0;
	virtual bool IsAvailable(Entity ent) = 0;
	virtual bool IsAssigned(Entity ent) = 0;
	virtual PropertiesPanelEntry* clone() = 0;
	virtual void OnAssign(Entity ent) {};
	virtual void OnRemove(Entity ent) {};
	virtual bool IsAssignable() { return false; };
	virtual ~PropertiesPanelEntry() {};

	const std::string& GetName() const {
		return name;
	}

private:
	std::string name;

};

enum class ShowPropertyFlags {
	NONE = 0, HIDE_PREFABS = 1, IS_PREFAB_CHILD = 2
};

struct PropertiesPanel_persistent_data {
	char* mesh_file_buffer;
	char* material_file_buffer;
	char* prefab_path;
	int buffer_size;
	std::vector<std::unique_ptr<PropertiesPanelEntry>> panel_entries;
	ShowPropertyFlags show_flags = ShowPropertyFlags::NONE;
};

class PropertiesPanel {
public:
	PropertiesPanel();
	~PropertiesPanel();

	void Render();

	void Refresh();

	template<typename T>
	auto RegisterPanelEntry(const T& entry) -> std::enable_if_t<std::is_base_of_v<PropertiesPanelEntry,T>>
	{
		panel_entries.push_back(std::make_unique<T>(entry));
	}

	PropertiesPanel_persistent_data CreateDefaultProperties() {
		PropertiesPanel_persistent_data data;
		for (auto& entry : panel_entries) {
			data.panel_entries.push_back(std::unique_ptr<PropertiesPanelEntry>(entry->clone()));
		}
		return data;
	}

	static void RenderProperties(Entity entity, const PropertiesPanel_persistent_data& data);

private:
	void ClearPanelEntries() {
		panel_entries.clear();
	}

	static void AddComponent(Entity entity, const PropertiesPanel_persistent_data& data, bool show_prefabs);

	char* text_buffer = nullptr;
	char* prefab_path_buffer = nullptr;
	char* material_file_buffer = nullptr;
	int buffer_size = 200;
	Entity last_entity = Entity();
	std::vector<std::unique_ptr<PropertiesPanelEntry>> panel_entries;
};