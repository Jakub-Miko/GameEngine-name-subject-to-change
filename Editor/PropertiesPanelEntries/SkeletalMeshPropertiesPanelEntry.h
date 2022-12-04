#pragma once
#include <Editor/Editor.h>
#include <Editor/PropertiesPanel.h>

class SkeletalMeshPropertiesPanelEntry : public PropertiesPanelEntry {
public:
	SkeletalMeshPropertiesPanelEntry();
	SkeletalMeshPropertiesPanelEntry(const SkeletalMeshPropertiesPanelEntry& other);

	virtual void RenderPanel(Entity ent) override;
	virtual bool IsAvailable(Entity ent) override;
	virtual bool IsAssigned(Entity ent) override;
	virtual PropertiesPanelEntry* clone() override;
	virtual void OnAssign(Entity ent) override;
	virtual void OnRemove(Entity ent) override;
	virtual bool IsAssignable() override;
	virtual ~SkeletalMeshPropertiesPanelEntry();
private:
	char* mesh_path_buffer = nullptr;
	char* material_path_buffer = nullptr;
	char* default_animation_path = nullptr;

};