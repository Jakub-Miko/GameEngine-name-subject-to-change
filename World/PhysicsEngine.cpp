#include "PhysicsEngine.h"
#include <World/Components/PhysicsComponent.h>
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <World/World.h>
#include <Application.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/MeshComponent.h>
#include <Renderer/MeshManager.h>
#include <Core/Debug.h>

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

class TransformMotionState : public btMotionState {
public:

	TransformMotionState(Entity owning_entity) : owning_entity(owning_entity), stored_size(1.0f) {}

	virtual void getWorldTransform(btTransform& worldTrans) const override {
		glm::mat4 transform = Application::GetWorld().GetComponentSync<TransformComponent>(owning_entity).TransformMatrix;
		glm::vec3 old_size = stored_size;
		stored_size[0] = glm::length(glm::vec3(transform[0]));
		stored_size[1] = glm::length(glm::vec3(transform[1]));
		stored_size[2] = glm::length(glm::vec3(transform[2]));
		if (old_size != stored_size) {
			PhysicsComponent& phys_comp = Application::GetWorld().GetComponentSync<PhysicsComponent>(owning_entity);
			switch (phys_comp.shape_type)
			{
			case PhysicsShapeType::BOUNDING_BOX:
				SetBoundingBoxShapeSize(stored_size, phys_comp.physics_shape , phys_comp.physics_object ? btRigidBody::upcast(phys_comp.physics_object.get()) : nullptr);
				break;
			default:
				throw std::runtime_error("Scaling of this shape_type isn't supported");
			}
		}
		if ((stored_size.x * stored_size.y * stored_size.z) != 0.0f) {
			transform[0] /= glm::vec4(glm::vec3(stored_size[0]),1.0f);
			transform[1] /= glm::vec4(glm::vec3(stored_size[1]), 1.0f);
			transform[2] /= glm::vec4(glm::vec3(stored_size[2]), 1.0f);
#ifdef EDITOR
			error = false;
#endif
		}
		else {
			transform = glm::translate(glm::mat4(1.0f), glm::vec3(transform[3]));
			stored_size = glm::vec3(1.0f);
#ifdef EDITOR
			if (!error) {
				EDITOR_ERROR("Scaling of a physics object can't be 0 \nPlease set a valid scale");
				error = true;
			}
#endif
		}
		worldTrans.setFromOpenGLMatrix(glm::value_ptr(transform));
	}

	virtual void setWorldTransform(const btTransform& worldTrans) override {
		glm::mat4 transform;
		worldTrans.getOpenGLMatrix(glm::value_ptr(transform));
		transform = glm::scale(transform, stored_size);
		Application::GetWorld().SetEntityTransformSync(owning_entity, transform);
	}
private:

	void SetBoundingBoxShapeSize(glm::vec3 size, btCollisionShape* shape, btRigidBody* body) const {
		btVector3 btsize = *(btVector3*)(&size);
		shape->setLocalScaling(btsize);
		btVector3 inertia;
		if (body) {
			shape->calculateLocalInertia(body->getMass(), inertia);
			Application::GetWorld().GetPhysicsEngine().bullet_data->world->removeRigidBody(body);
			Application::GetWorld().GetPhysicsEngine().bullet_data->world->addRigidBody(body);
		}
		//TODO: Update inertia;
	}


	Entity owning_entity;
	mutable glm::vec3 stored_size;
#ifdef EDITOR
	mutable bool error = false;
#endif

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
	if (!running) return;
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
	if (!Application::GetWorld().HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("Entity doesn't have a Physics Component");
	PhysicsComponent& component = Application::GetWorld().GetComponent<PhysicsComponent>(ent);
	deletion_queue.push_front(deletion_queue_entry{ component });
	component.physics_object = nullptr; 
	component.physics_shape = nullptr;
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
		physics_comp.physics_shape = box_collision_shape;
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, box_collision_shape, inertia);
		btRigidBody* body = new btRigidBody(info);
		body->setUserPointer((void*)entity.id);
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

void PhysicsEngine::StopSim()
{
	running = false;
}

void PhysicsEngine::StartSim()
{
	running = true;
}

void PhysicsEngine::ResetSim()
{
	for (int i = 0; i < bullet_data->world->getCollisionObjectArray().size(); i++)
	{
		btCollisionObject* colObj = bullet_data->world->getCollisionObjectArray()[i];
		Entity ent = Entity((uint32_t)colObj->getUserPointer());
		Application::GetWorld().MarkEntityDirty(ent);
	}
	Application::GetWorld().GetSceneGraph()->CalculateMatricies();
	for (int i = 0; i < bullet_data->world->getCollisionObjectArray().size(); i++)
	{
		btCollisionObject* colObj = bullet_data->world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(colObj);
		if (body && body->getMotionState())
		{
			btTransform trans;
			body->clearForces();
			btVector3 vec(0.0f, 0.0f, 0.0f);
			body->setAngularVelocity(vec);
			body->setLinearVelocity(vec);
			body->getMotionState()->getWorldTransform(trans);
			body->setCenterOfMassTransform(trans);
			body->setWorldTransform(trans);
			Entity ent = Entity((uint32_t)colObj->getUserPointer());
			Application::GetWorld().MarkEntityDirty(ent);
		}
	}
	Application::GetWorld().GetSceneGraph()->CalculateMatricies();
}

void PhysicsEngine::PassiveMode()
{
	StopSim();
	ResetSim();
}

void PhysicsEngine::ActiveMode()
{
	ResetSim();
	StartSim();
}

void PhysicsEngine::RefreshObject(Entity ent)
{
	UnRegisterPhysicsComponent(ent);
	RegisterPhysicsComponent(ent);
}






