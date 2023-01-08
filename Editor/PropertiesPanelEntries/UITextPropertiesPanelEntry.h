#pragma once
#include <Editor/Editor.h>
#include <Editor/PropertiesPanel.h>

class UITextPropertiesPanelEntry : public PropertiesPanelEntry {
public:
	UITextPropertiesPanelEntry();
	UITextPropertiesPanelEntry(const UITextPropertiesPanelEntry& other);

	virtual void RenderPanel(Entity ent) override;
	virtual bool IsAvailable(Entity ent) override;
	virtual bool IsAssigned(Entity ent) override;
	virtual PropertiesPanelEntry* clone() override;
	virtual void OnAssign(Entity ent) override;
	virtual void OnRemove(Entity ent) override;
	virtual bool IsAssignable() override;
	virtual ~UITextPropertiesPanelEntry();
private:
	char* buffer = nullptr;
	char* text_buffer = nullptr;
};