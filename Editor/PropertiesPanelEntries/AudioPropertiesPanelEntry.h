#pragma once
#include <Editor/Editor.h>
#include <Editor/PropertiesPanel.h>

class AudioPropertiesPanelEntry : public PropertiesPanelEntry {
public:
	AudioPropertiesPanelEntry();
	AudioPropertiesPanelEntry(const AudioPropertiesPanelEntry& other);

	virtual void RenderPanel(Entity ent) override;
	virtual bool IsAvailable(Entity ent) override;
	virtual bool IsAssigned(Entity ent) override;
	virtual PropertiesPanelEntry* clone() override;
	virtual void OnAssign(Entity ent) override;
	virtual void OnRemove(Entity ent) override;
	virtual bool IsAssignable() override;
	virtual ~AudioPropertiesPanelEntry();
private:
	char* buffer = nullptr;

};