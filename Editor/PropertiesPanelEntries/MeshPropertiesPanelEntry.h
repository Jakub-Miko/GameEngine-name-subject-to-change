#pragma once
#include <Editor/Editor.h>
#include <Editor/PropertiesPanel.h>

class MeshPropertiesPanelEntry : public PropertiesPanelEntry {
public:
	MeshPropertiesPanelEntry();
	MeshPropertiesPanelEntry(const MeshPropertiesPanelEntry& other);

	virtual void RenderPanel(Entity ent) override;
	virtual bool IsAvailable(Entity ent) override;
	virtual bool IsAssigned(Entity ent) override;
	virtual PropertiesPanelEntry* clone() override;
	virtual void OnAssign(Entity ent) override;
	virtual void OnRemove(Entity ent) override;
	virtual bool IsAssignable() override;
	virtual ~MeshPropertiesPanelEntry();
private:
	char* mesh_path_buffer = nullptr;
	char* material_path_buffer = nullptr;

};