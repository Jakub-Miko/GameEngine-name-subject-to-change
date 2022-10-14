#pragma once 
#include <Core/RuntimeTag.h>
#include <World/Entity.h>
#include <Core/UnitConverter.h>
#include <memory>

class btCollisionObject;
class btCollisionShape;
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
	BOUNDING_BOX = 0, CONVEX_HULL = 1, TRIANGLE_MESH = 2
};

enum class PhysicsObjectState : char {
	UNINITIALIZED = 0, INITIALIZED = 1
};

struct PhysicsComponent {
	RuntimeTag("PhysicsComponent");
	PhysicsComponent() {}

	float mass = 0.0f;
	PhysicsObjectType object_type = PhysicsObjectType::RIGID_BODY;
	PhysicsShapeType shape_type = PhysicsShapeType::BOUNDING_BOX;
	bool is_kinematic = false;
	PhysicsObjectState state = PhysicsObjectState::UNINITIALIZED;
private:
	friend class PhysicsEngine;
	friend class TransformMotionState;
	std::shared_ptr<btCollisionObject> physics_object;
	btCollisionShape* physics_shape;
};

JSON_SERIALIZABLE(PhysicsComponent, mass, object_type, shape_type, is_kinematic);