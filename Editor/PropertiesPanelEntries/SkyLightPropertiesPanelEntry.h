#pragma once
#include <Editor/Editor.h>
#include <Editor/PropertiesPanel.h>

class SkyLightPropertiesPanelEntry : public PropertiesPanelEntry {
public:
	SkyLightPropertiesPanelEntry();
	SkyLightPropertiesPanelEntry(const SkyLightPropertiesPanelEntry& other);

	virtual void RenderPanel(Entity ent) override;
	virtual bool IsAvailable(Entity ent) override;
	virtual bool IsAssigned(Entity ent) override;
	virtual PropertiesPanelEntry* clone() override;
	virtual void OnAssign(Entity ent) override;
	virtual void OnRemove(Entity ent) override;
	virtual bool IsAssignable() override;
	virtual ~SkyLightPropertiesPanelEntry();
private:
	char* buffer = nullptr;
};