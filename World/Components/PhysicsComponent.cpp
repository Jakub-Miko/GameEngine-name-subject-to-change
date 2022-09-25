#include "PhysicsComponent.h"
#include <Application.h>
#include <World/World.h>
#include <World/PhysicsEngine.h>

void ComponentInitProxy<PhysicsComponent>::OnCreate(World& world, Entity entity) {
	Application::GetWorld().GetPhysicsEngine().RegisterPhysicsComponent(entity);
}

void ComponentInitProxy<PhysicsComponent>::OnDestroy(World& world, Entity entity) {
	Application::GetWorld().GetPhysicsEngine().UnRegisterPhysicsComponent(entity);
}