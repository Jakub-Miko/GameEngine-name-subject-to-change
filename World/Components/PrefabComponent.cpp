#include "PrefabComponent.h"
#include <Application.h>
#include <World/World.h>

void ComponentInitProxy<PrefabComponent>::OnCreate(World& world, Entity entity) {
	SceneNode* node = world.GetSceneGraph()->GetSceneGraphNode(entity);
	if (node) {
		node->state = node->state | SceneNodeState::PREFAB;
	}
}

void ComponentInitProxy<PrefabComponent>::OnDestroy(World& world, Entity entity) {
	SceneNode* node = world.GetSceneGraph()->GetSceneGraphNode(entity);
	if (node) {
		node->state = node->state & ~SceneNodeState::PREFAB;
	}
}
