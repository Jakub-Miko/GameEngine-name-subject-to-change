#pragma once
#include "Editor/PropertiesPanel.h"

class AnimationPropertiesPanelEntry : public PropertiesPanelEntry {
public:
    AnimationPropertiesPanelEntry();
    AnimationPropertiesPanelEntry(const AnimationPropertiesPanelEntry& other);

    void RenderPanel(Entity ent) override;
    bool IsAssignable() override;
    bool IsAvailable(Entity ent) override;
    void OnAssign(Entity ent) override;
    void OnRemove(Entity ent) override;
    bool IsAssigned(Entity ent) override;
    PropertiesPanelEntry* clone() override;
    ~AnimationPropertiesPanelEntry() override;
private:
    char* path_buffer = nullptr;
};
