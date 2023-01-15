#include "UITextComponent.h"
#include <Application.h>
#include <World/World.h>

void ComponentInitProxy<UITextComponent>::OnCreate(World& world, Entity entity) {
	SceneNode* node = world.GetSceneGraph()->GetSceneGraphNode(entity);
	if (node) {
		node->state = node->state | SceneNodeState::UI_ENTITY;
	}
}

void ComponentInitProxy<UITextComponent>::OnDestroy(World& world, Entity entity) {
	SceneNode* node = world.GetSceneGraph()->GetSceneGraphNode(entity);
	if (node) {
		node->state = node->state & ~SceneNodeState::UI_ENTITY;
	}
}
