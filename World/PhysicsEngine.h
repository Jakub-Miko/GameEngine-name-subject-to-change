#pragma once
#include <World/Entity.h>
#include <deque>
#include <mutex>
#include <World/Components/PhysicsComponent.h>

struct PhysicsComponent;
struct PhysicsEngine_BulletData;

struct PhysicsEngineProps {

};



class PhysicsEngine {
public:

	PhysicsEngine(const PhysicsEngineProps& props);

	void UpdatePhysics(float delta_time);

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

	bool CreatePhysicsObject(Entity entity);
	void DestroyPhysicsObject(const PhysicsComponent& physics_comp);

	PhysicsEngine_BulletData* bullet_data;
	std::mutex creation_mutex;
	std::deque<creation_queue_entry> creation_queue;

	std::mutex deletion_mutex;
	std::deque<deletion_queue_entry> deletion_queue;

};