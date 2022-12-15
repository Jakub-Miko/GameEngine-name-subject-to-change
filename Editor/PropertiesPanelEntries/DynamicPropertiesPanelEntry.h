#pragma once
#include <Editor/Editor.h>
#include <Editor/PropertiesPanel.h>

class DynamicPropertiesPanelEntry : public PropertiesPanelEntry {
public:
	DynamicPropertiesPanelEntry();
	DynamicPropertiesPanelEntry(const DynamicPropertiesPanelEntry& other);

	virtual void RenderPanel(Entity ent) override;
	virtual bool IsAvailable(Entity ent) override;
	virtual bool IsAssigned(Entity ent) override;
	virtual PropertiesPanelEntry* clone() override;
	virtual void OnAssign(Entity ent) override;
	virtual void OnRemove(Entity ent) override;
	virtual bool IsAssignable() override;
	virtual ~DynamicPropertiesPanelEntry();
private:


};