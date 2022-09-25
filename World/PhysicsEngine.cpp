#include "PhysicsEngine.h"
#include <World/Components/PhysicsComponent.h>
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <World/World.h>
#include <Application.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/MeshComponent.h>
#include <Renderer/MeshManager.h>

class TransformMotionState : public btMotionState {
public:

	TransformMotionState(Entity owning_entity) : owning_entity(owning_entity) {}

	virtual void getWorldTransform(btTransform& worldTrans) const override {
		worldTrans.setFromOpenGLMatrix(glm::value_ptr(Application::GetWorld().GetComponentSync<TransformComponent>(owning_entity).TransformMatrix));
	}

	virtual void setWorldTransform(const btTransform& worldTrans) override {
		glm::mat4 transform;
		worldTrans.getOpenGLMatrix(glm::value_ptr(transform));
		Application::GetWorld().SetEntityTransformSync(owning_entity, transform);
	}
private:
	Entity owning_entity;

};

struct PhysicsEngine_BulletData {
	std::unique_ptr<btCollisionConfiguration> collision_config;
	std::unique_ptr<btBroadphaseInterface> broadphase;
	std::unique_ptr<btCollisionDispatcher> collision_dispather;
	std::unique_ptr<btConstraintSolver> constraint_solver;
	std::unique_ptr<btDynamicsWorld> world;

	std::mutex triangle_shape_mutex;
	std::unordered_map<std::string, std::shared_ptr<btBvhTriangleMeshShape>> triangle_shapes;

	std::mutex box_shape_mutex;
	std::unordered_map<std::string, std::shared_ptr<btBoxShape>> box_shapes;

	std::mutex convex_hull_shape_mutex;
	std::unordered_map<std::string, std::shared_ptr<btConvexHullShape>> convex_hull_shapes;

};


PhysicsEngine::PhysicsEngine(const PhysicsEngineProps& props) : bullet_data(new PhysicsEngine_BulletData)
{
	bullet_data->broadphase = std::make_unique<btDbvtBroadphase>();
	bullet_data->collision_config = std::make_unique<btDefaultCollisionConfiguration>();
	bullet_data->collision_dispather = std::make_unique<btCollisionDispatcher>(bullet_data->collision_config.get());
	bullet_data->constraint_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	bullet_data->world = std::make_unique<btDiscreteDynamicsWorld>(bullet_data->collision_dispather.get(), bullet_data->broadphase.get(), bullet_data->constraint_solver.get(), bullet_data->collision_config.get());
	bullet_data->world->setGravity(btVector3(0.0f, -9.8f, 0.0f));
}

void PhysicsEngine::UpdatePhysics(float delta_time)
{
	DeletionPhase();
	CreationPhase();
	bullet_data->world->stepSimulation(delta_time);
}

void PhysicsEngine::RegisterPhysicsComponent(Entity ent)
{
	std::lock_guard<std::mutex> lock(creation_mutex);
	creation_queue.push_front(creation_queue_entry{ ent });
}

void PhysicsEngine::UnRegisterPhysicsComponent(Entity ent)
{
	std::lock_guard<std::mutex> lock(deletion_mutex);
	if (!Application::GetWorld().HasComponentSynced<PhysicsComponent>(ent)) throw std::runtime_error("Entity doesn't have a Physics Component");
	PhysicsComponent component = Application::GetWorld().GetComponent<PhysicsComponent>(ent);
	deletion_queue.push_front(deletion_queue_entry{ component });
}

PhysicsEngine::~PhysicsEngine()
{
	destroy();
}

void PhysicsEngine::clear()
{
	destroy();
	bullet_data = new PhysicsEngine_BulletData;
	bullet_data->broadphase = std::make_unique<btDbvtBroadphase>();
	bullet_data->collision_config = std::make_unique<btDefaultCollisionConfiguration>();
	bullet_data->collision_dispather = std::make_unique<btCollisionDispatcher>(bullet_data->collision_config.get());
	bullet_data->constraint_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	bullet_data->world = std::make_unique<btDiscreteDynamicsWorld>(bullet_data->collision_dispather.get(), bullet_data->broadphase.get(), bullet_data->constraint_solver.get(), bullet_data->collision_config.get());
	bullet_data->world->setGravity(btVector3(0.0f, -9.8f, 0.0f));
}

void PhysicsEngine::destroy()
{
	DeletionPhase();
	auto& col_array = bullet_data->world->getCollisionObjectArray();
	for (int i = 0; i < bullet_data->world->getNumCollisionObjects(); i++) {
		auto col_object = col_array[i];
		bullet_data->world->removeCollisionObject(col_object);
		delete col_object->getCollisionShape();
		btRigidBody* body = btRigidBody::upcast(col_object);
		if (body) {
			if (body->getMotionState()) delete body->getMotionState();
		}
		delete col_object;
	}
	delete bullet_data;
}

void PhysicsEngine::CreationPhase()
{
	std::lock_guard<std::mutex> lock(creation_mutex);
	for (auto& ent : creation_queue) {
		if(CreatePhysicsObject(ent.entity)) ent.processed = true;
	}

	while (!creation_queue.empty() && creation_queue.front().processed) {
		creation_queue.pop_front();
	}

}

void PhysicsEngine::DeletionPhase()
{
	std::lock_guard<std::mutex> lock(deletion_mutex);
	for (auto& ent : deletion_queue) {
		DestroyPhysicsObject(ent.phys_comp);
	}
	deletion_queue.clear();
}

bool PhysicsEngine::CreatePhysicsObject(Entity entity)
{
	PhysicsComponent& physics_comp = Application::GetWorld().GetComponent<PhysicsComponent>(entity);
	if (!Application::GetWorld().HasComponent<MeshComponent>(entity)) throw std::runtime_error("Can't assign Physics component to an entity without a mesh");
	MeshComponent& mesh_comp = Application::GetWorld().GetComponent<MeshComponent>(entity);
	switch (physics_comp.shape_type) {
	case PhysicsShapeType::BOUNDING_BOX:
	{
		if (mesh_comp.mesh->GetMeshStatus() != Mesh_status::READY) return false;
		glm::vec3 box_size = mesh_comp.mesh->GetBoundingBox().GetBoxSize() / 2.0f;
		btVector3 half_extents = btVector3{ box_size.x,box_size.y,box_size.z };
		btBoxShape* box_collision_shape = new btBoxShape(half_extents);
		btVector3 inertia;
		box_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity);
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, box_collision_shape, inertia);
		btRigidBody* body = new btRigidBody(info);
		if (physics_comp.is_kinematic && physics_comp.mass == 0.0f) {
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		}

		bullet_data->world->addRigidBody(body);

		physics_comp.physics_object.reset((btCollisionObject*)body);
		break;
	}
	default:
		throw std::runtime_error("This Collision type is not supported");
	}

	return true;

}

void PhysicsEngine::DestroyPhysicsObject(const PhysicsComponent& physics_comp)
{
	switch (physics_comp.shape_type) {
	case PhysicsShapeType::BOUNDING_BOX:
	{
		btRigidBody* body = btRigidBody::upcast(physics_comp.physics_object.get());
		btCollisionShape* shape = body->getCollisionShape();
		btMotionState* state = body->getMotionState();
		bullet_data->world->removeRigidBody(body);
		delete shape;
		delete state;
		break;
	}
	default:
		throw std::runtime_error("This Collision type is not supported");
	}
}
