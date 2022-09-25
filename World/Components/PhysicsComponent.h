#pragma once 
#include <Core/RuntimeTag.h>
#include <World/Entity.h>
#include <memory>

class btCollisionObject;
class PhysicsComponent;

template<typename T>
class ComponentInitProxy;

template<>
class ComponentInitProxy<PhysicsComponent> {
public:

	static void OnCreate(World& world, Entity entity);

	static void OnDestroy(World& world, Entity entity);


};

enum class PhysicsObjectType : char {
	RIGID_BODY = 0
};
enum class PhysicsShapeType : char {
	TRIANGLE_MESH = 0, CONVEX_HULL = 1, BOUNDING_BOX = 2
};

enum class PhysicsObjectState : char {
	UNINITIALIZED = 0, INITIALIZED = 1
};

struct PhysicsComponent {
	RuntimeTag("PhysicsComponent");
	PhysicsComponent()  {}

	float mass = 0.0f;
	PhysicsObjectType object_type;
	PhysicsShapeType shape_type;
	bool is_kinematic = false;
	PhysicsObjectState state = PhysicsObjectState::UNINITIALIZED;
private:
	friend class PhysicsEngine;
	std::shared_ptr<btCollisionObject> physics_object;
};

