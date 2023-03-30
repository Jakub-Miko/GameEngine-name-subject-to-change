#pragma once 
#include <Core/RuntimeTag.h>
#include <World/Entity.h>
#include <Core/UnitConverter.h>
#include <memory>

class btCollisionObject;
class btCollisionShape;
class PhysicsComponent;
class World;

template<typename T>
class ComponentInitProxy;

template<>
class ComponentInitProxy<PhysicsComponent> {
public:

	static void OnCreate(World& world, Entity entity);

	static void OnDestroy(World& world, Entity entity);

	static constexpr bool can_copy = true;

};

enum class PhysicsObjectProperties : char {
	NONE = 0, RECIEVE_COLLISION_EVENTS = 1, DISABLE_COLLISION_RESPONSE = 2
};

inline PhysicsObjectProperties operator|(const PhysicsObjectProperties& first, const PhysicsObjectProperties& second) {
	return PhysicsObjectProperties((char)first | (char)second);
}

inline PhysicsObjectProperties operator&(const PhysicsObjectProperties& first, const PhysicsObjectProperties& second) {
	return PhysicsObjectProperties((char)first & (char)second);
}

inline PhysicsObjectProperties operator~(const PhysicsObjectProperties& first) {
	return PhysicsObjectProperties(~(char)first);
}

enum class PhysicsObjectType : char {
	RIGID_BODY = 0
};
enum class PhysicsShapeType : char {
	BOUNDING_BOX = 0, CONVEX_HULL = 1, CAPSULE_OUTER = 2, CAPSULE_INNER = 3, TRIANGLE_MESH = 4
};

enum class PhysicsObjectState : char {
	UNINITIALIZED = 0, INITIALIZED = 1
};

struct AuxilaryPhysicsProps {
	float friction = 0.7f;
	float mass = 0.0f;
	glm::vec3 angular_factor = glm::vec3(1.0f);
	glm::vec3 linear_factor = glm::vec3(1.0f);
};

struct PhysicsComponent {
	RUNTIME_TAG("PhysicsComponent");
	PhysicsComponent() {}

	PhysicsComponent(const PhysicsComponent& other) : mass(other.mass), object_type(other.object_type), 
		shape_type(other.shape_type), is_kinematic(other.is_kinematic), state(PhysicsObjectState::UNINITIALIZED), props(other.props) {}
	PhysicsComponent(PhysicsComponent&& other) noexcept : mass(other.mass), object_type(other.object_type), 
		shape_type(other.shape_type), is_kinematic(other.is_kinematic), state(other.state), physics_shape(other.physics_shape), physics_object(other.physics_object), props(other.props)
	{
		other.physics_object = nullptr;
		other.physics_shape = nullptr;
		other.state = PhysicsObjectState::UNINITIALIZED;
	}

	PhysicsComponent& operator=(PhysicsComponent&& other) noexcept
	{
		mass = other.mass; 
		object_type = other.object_type;
		shape_type = other.shape_type;
		is_kinematic = other.is_kinematic;
		state = other.state;
		physics_shape = other.physics_shape; 
		physics_object = other.physics_object;
		props = other.props;
		other.physics_object = nullptr;
		other.physics_shape = nullptr;
		other.state = PhysicsObjectState::UNINITIALIZED;
		return *this;
	}

	PhysicsComponent& operator=(const PhysicsComponent& other)
	{
		mass = other.mass;
		object_type = other.object_type;
		shape_type = other.shape_type;
		props = other.props;
		is_kinematic = other.is_kinematic;
		state = PhysicsObjectState::UNINITIALIZED;
		return *this;
	}
	std::unique_ptr<AuxilaryPhysicsProps> auxilary_props = nullptr;
	float mass = 0.0f;
	PhysicsObjectType object_type = PhysicsObjectType::RIGID_BODY;
	PhysicsShapeType shape_type = PhysicsShapeType::BOUNDING_BOX;
	PhysicsObjectProperties props = PhysicsObjectProperties(0);
	bool is_kinematic = false;
	PhysicsObjectState state = PhysicsObjectState::UNINITIALIZED;
private:
	friend class PhysicsEngine;
	friend class TransformMotionState;
	std::shared_ptr<btCollisionObject> physics_object = nullptr;
	btCollisionShape* physics_shape = nullptr;
};

JSON_SERIALIZABLE(PhysicsComponent, mass, object_type, shape_type, is_kinematic, props);