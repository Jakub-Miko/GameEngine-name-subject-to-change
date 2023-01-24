#pragma once
#include <Events/SubjectObserver.h>
#include <World/Entity.h>
#include <deque>
#include <mutex>
#include <World/Components/PhysicsComponent.h>
#include <vector>

struct PhysicsComponent;
struct PhysicsEngine_BulletData;

struct PhysicsEngineProps {

};

struct PhysicsObjectInfo {
	Entity entity;
};

struct PhysicsRayTestResult {
	glm::vec3 position;
	glm::vec3 normal;
	Entity ent = Entity();
};

using PhysicsRayTestResultArray = typename std::template vector<PhysicsRayTestResult>;

class PhysicsEngine {
public:

	PhysicsEngine(const PhysicsEngineProps& props);

	void UpdatePhysics(float delta_time);

	void StopSim();

	void StartSim();

	void ResetSim();

	void PassiveMode();

	void ActiveMode();

	void ApplyForce(Entity ent, const glm::vec3& direction);

	void SetMass(Entity ent, float mass);

	void SetFriction(Entity ent, float friction);

	bool IsObjectRecievingCollisions(Entity ent);

	void SetRecieveCollision(Entity ent, bool enable);

	bool RayCast(const glm::vec3& from, const glm::vec3& to, PhysicsRayTestResultArray& results);
	bool RayCastSingle(const glm::vec3& from, const glm::vec3& to, PhysicsRayTestResult& results);

	void SetLinearFactor(Entity ent, const glm::vec3& linear_factor);
	void SetAngularFactor(Entity ent, const glm::vec3& angular_factor);

	void SetLinearVelocity(Entity ent, const glm::vec3& linear_velocity);
	
	void SetAngularVelocity(Entity ent, const glm::vec3& angular_velocity);

	glm::vec3 GetAngularVelocity(Entity ent);
	glm::vec3 GetLinearVelocity(Entity ent);

	void RefreshObject(Entity ent);

	bool IsPhysicsActive() const { return running; }

	void RegisterPhysicsComponent(Entity ent);

	void UnRegisterPhysicsComponent(Entity ent);

	~PhysicsEngine();

private:
	void clear();
	void destroy();
	friend class World;
	friend class TransformMotionState;
	struct creation_queue_entry {
		Entity entity;
		bool processed = false;
	};

	struct deletion_queue_entry {
		PhysicsComponent phys_comp;
	};

	void CreationPhase();
	void DeletionPhase();

	void PhysicsCollisionCallbackPhase();

	bool CreatePhysicsObject(Entity entity);
	void DestroyPhysicsObject(const PhysicsComponent& physics_comp);

	bool running = true;

	std::unique_ptr<EventObserverBase> mesh_change_observer;

	PhysicsEngine_BulletData* bullet_data;
	std::mutex creation_mutex;
	std::deque<creation_queue_entry> creation_queue;

	std::mutex deletion_mutex;
	std::deque<deletion_queue_entry> deletion_queue;

};