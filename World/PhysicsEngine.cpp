#include <math.h>
#include "PhysicsEngine.h"
#include <Events/CollisionEvent.h>
#include <Events/MeshChangedEvent.h>
#include <World/Components/PhysicsComponent.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <memory>
#include <World/World.h>
#include <Application.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/SkeletalMeshComponent.h>
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



	std::shared_ptr<btConvexHullShape> GetHullShape(const std::string& mesh_path);
	void SaveHullShape(const std::string& save_absolute_path, btConvexHullShape* hull);
	btConvexHullShape* LoadHullShape(const std::string& save_absolute_path);

};

class CustomInnnerCapsule : public btCapsuleShape {
public:
	CustomInnnerCapsule() : btCapsuleShape() {

	}

	CustomInnnerCapsule(btScalar radius, btScalar height) : btCapsuleShape(radius, height) {
		original_size = { radius * 2, height + radius * 2 , radius * 2 };
	}

	virtual void setLocalScaling(const btVector3& scaling) override
	{
		btVector3 scaled_bounds = original_size * scaling;
		float min = std::min(std::min(scaled_bounds.x(), scaled_bounds.y()), scaled_bounds.z());
		scaled_bounds[0] = min;
		scaled_bounds[2] = min;
		scaled_bounds[1] = scaled_bounds[1] - min;
		scaled_bounds = scaled_bounds / btVector3(2.0f, 2.0f, 2.0f);

		btConvexInternalShape::setLocalScaling(scaling);
		m_implicitShapeDimensions = (scaled_bounds);
		//update m_collisionMargin, since entire radius==margin
		int radiusAxis = (m_upAxis + 2) % 3;
		m_collisionMargin = m_implicitShapeDimensions[radiusAxis];
	} 
private:
	btVector3 original_size = { 2,2,2 };
};

class CustomOuterCapsule : public btCapsuleShape {
public:
	CustomOuterCapsule() : btCapsuleShape() {

	}

	CustomOuterCapsule(btScalar radius, btScalar height) : btCapsuleShape(radius, height) {
		original_size = {2* radius , height  , 2*radius };
	}

	virtual void setLocalScaling(const btVector3& scaling) override
	{
		btVector3 scaled_bounds = original_size * scaling;
		float radius = std::max(scaled_bounds.x(), scaled_bounds.z());
		scaled_bounds[0] = radius;
		scaled_bounds[2] = radius;
		scaled_bounds[1] = scaled_bounds[1] ;
		scaled_bounds = scaled_bounds / btVector3(2.0f, 2.0f, 2.0f);

		btConvexInternalShape::setLocalScaling(scaling);
		m_implicitShapeDimensions = (scaled_bounds);
		//update m_collisionMargin, since entire radius==margin
		int radiusAxis = (m_upAxis + 2) % 3;
		m_collisionMargin = m_implicitShapeDimensions[radiusAxis];
	}
private:
	btVector3 original_size = { 2,2,2 };
};


class TransformMotionState : public btMotionState {
public:

	TransformMotionState(Entity owning_entity, const glm::vec3& offset = glm::vec3(0.0f)) : owning_entity(owning_entity), stored_size(1.0f) , shape_offset(offset) {}

	//Not thread-safe
	virtual void getWorldTransform(btTransform& worldTrans) const override {
		glm::mat4 transform = Application::GetWorld().GetComponentSync<TransformComponent>(owning_entity).TransformMatrix;
		transform = glm::translate(transform, shape_offset);
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
			case PhysicsShapeType::CONVEX_HULL:
				SetConvexHullShapeSize(stored_size, phys_comp.physics_shape, phys_comp.physics_object ? btRigidBody::upcast(phys_comp.physics_object.get()) : nullptr);
				break;
			case PhysicsShapeType::CAPSULE_INNER:
				SetCapsuleInnerShapeSize(stored_size, phys_comp.physics_shape, phys_comp.physics_object ? btRigidBody::upcast(phys_comp.physics_object.get()) : nullptr);
				break;
			case PhysicsShapeType::CAPSULE_OUTER:
				SetCapsuleOuterShapeSize(stored_size, phys_comp.physics_shape, phys_comp.physics_object ? btRigidBody::upcast(phys_comp.physics_object.get()) : nullptr);
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

	//Not thread-safe
	virtual void setWorldTransform(const btTransform& worldTrans) override {
		glm::mat4 transform;
		worldTrans.getOpenGLMatrix(glm::value_ptr(transform));
		transform = glm::translate(glm::scale(transform, stored_size),-shape_offset);
		Application::GetWorld().SetEntityTransformSync(owning_entity, transform);
	}
private:

	//Not thread-safe
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

	//Not thread-safe
	void SetConvexHullShapeSize(glm::vec3 size, btCollisionShape* shape, btRigidBody* body) const {
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

	//Not thread-safe
	void SetCapsuleOuterShapeSize(glm::vec3 size, btCollisionShape* shape, btRigidBody* body) const {
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

	//Not thread-safe
	void SetCapsuleInnerShapeSize(glm::vec3 size, btCollisionShape* shape, btRigidBody* body) const {;
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

	glm::vec3 shape_offset;
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

	mesh_change_observer.reset(MakeEventObserver<MeshChangedEvent>([this](MeshChangedEvent* e) -> bool {
		if (!Application::GetWorld().HasComponent<PhysicsComponent>(e->ent)) return false;
		UnRegisterPhysicsComponent(e->ent);
		RegisterPhysicsComponent(e->ent);
		return false;
		}));
	Application::Get()->RegisterObserver<MeshChangedEvent>(mesh_change_observer.get());
}

//Not thread-safe
void PhysicsEngine::UpdatePhysics(float delta_time)
{
	DeletionPhase();
	CreationPhase();
	if (!running) return;
	bullet_data->world->stepSimulation(delta_time *0.003f,3);
	PhysicsCollisionCallbackPhase();
}

//thread-safe
void PhysicsEngine::RegisterPhysicsComponent(Entity ent)
{
	std::lock_guard<std::mutex> lock(creation_mutex);
	creation_queue.push_front(creation_queue_entry{ ent });
}

//thread-safe
void PhysicsEngine::UnRegisterPhysicsComponent(Entity ent)
{
	std::lock_guard<std::mutex> lock(deletion_mutex);
	if (!Application::GetWorld().HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("Entity doesn't have a Physics Component");
	PhysicsComponent& component = Application::GetWorld().GetComponent<PhysicsComponent>(ent);
	deletion_queue.push_front(deletion_queue_entry{ std::move(component) });
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

//Not thread-safe
void PhysicsEngine::destroy()
{
	CreationPhase();
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

//thread-safe
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
//thread-safe
void PhysicsEngine::DeletionPhase()
{
	std::lock_guard<std::mutex> lock(deletion_mutex);
	for (auto& ent : deletion_queue) {
		DestroyPhysicsObject(ent.phys_comp);
	}
	deletion_queue.clear();
}

void PhysicsEngine::PhysicsCollisionCallbackPhase()
{
	int num_manifolds = bullet_data->world->getDispatcher()->getNumManifolds();
	for (int i = 0; i < num_manifolds; i++) {
		auto manifold = bullet_data->world->getDispatcher()->getManifoldByIndexInternal(i);
		auto phys_info_a_ent = ((PhysicsObjectInfo*)manifold->getBody0()->getUserPointer())->entity;
		auto phys_info_b_ent = ((PhysicsObjectInfo*)manifold->getBody1()->getUserPointer())->entity;
		auto phys_info_a = Application::GetWorld().GetComponent<PhysicsComponent>(phys_info_a_ent).props;
		auto phys_info_b = Application::GetWorld().GetComponent<PhysicsComponent>(phys_info_b_ent).props;


		if ((bool)(phys_info_a & PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS)) {
			CollisionEvent* col_event = new CollisionEvent;
			col_event->collision_point_number = std::min(4, manifold->getNumContacts());
			for (int x = 0; x < col_event->collision_point_number; x++) {
				btVector3 pos = manifold->getContactPoint(x).getPositionWorldOnA();
				col_event->collision_points[x] = { pos.x(), pos.y(),pos.z() };
			}
			col_event->reciever = phys_info_a_ent;
			col_event->other = phys_info_b_ent;
			;	   			Application::Get()->SendObservedEvent<CollisionEvent>(col_event);
			delete col_event;
		}

		if ((bool)(phys_info_b & PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS)) {
			CollisionEvent* col_event = new CollisionEvent;
			col_event->collision_point_number = std::min(4, manifold->getNumContacts());
			for (int x = 0; x < col_event->collision_point_number; x++) {
				btVector3 pos = manifold->getContactPoint(x).getPositionWorldOnB();
				col_event->collision_points[x] = { pos.x(), pos.y(),pos.z() };
			}
			col_event->other = phys_info_a_ent;
			col_event->reciever = phys_info_b_ent;
			Application::Get()->SendObservedEvent<CollisionEvent>(col_event);
			delete col_event;
		}

	}
}

//Not thread-safe
bool PhysicsEngine::CreatePhysicsObject(Entity entity)
{
	World& world = Application::GetWorld();
	PhysicsComponent& physics_comp = world.GetComponent<PhysicsComponent>(entity);

	std::shared_ptr<Mesh> mesh;
	BoundingBox bounding_box;
	Mesh_status status;
	std::string path;
	if (world.HasComponent<MeshComponent>(entity)) {
		MeshComponent& mesh_comp = world.GetComponent<MeshComponent>(entity);
		mesh = mesh_comp.GetMesh();
		bounding_box = mesh->GetBoundingBox();
		status = mesh->GetMeshStatus();
		path = mesh_comp.GetMeshPath();
	}
	else if(world.HasComponent<SkeletalMeshComponent>(entity)){
		SkeletalMeshComponent& mesh_comp = world.GetComponent<SkeletalMeshComponent>(entity);
		mesh = mesh_comp.GetMesh();
		bounding_box = mesh->GetBoundingBox();
		status = mesh->GetMeshStatus();
		path = mesh_comp.GetMeshPath();
	}
	else {
		throw std::runtime_error("Can't assign Physics component to an entity without a static or skeletal mesh");
	}

	btRigidBody* body =nullptr;
	switch (physics_comp.shape_type) {
	case PhysicsShapeType::BOUNDING_BOX:
	{
		if (status != Mesh_status::READY) return false;
		glm::vec3 box_size = bounding_box.GetBoxSize() / 2.0f;
		btVector3 half_extents = btVector3{ box_size.x,box_size.y,box_size.z };
		btBoxShape* box_collision_shape = new btBoxShape(half_extents);
		btVector3 inertia;
		box_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity,bounding_box.GetBoxOffset());
		physics_comp.physics_shape = box_collision_shape;
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, box_collision_shape, inertia);
		body = new btRigidBody(info);
		body->setUserPointer(new PhysicsObjectInfo{ entity });
		if (physics_comp.is_kinematic && physics_comp.mass == 0.0f) {
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		}

		physics_comp.state = PhysicsObjectState::INITIALIZED;

		bullet_data->world->addRigidBody(body);
		if (physics_comp.physics_object) {
			btRigidBody* redundant_body = btRigidBody::upcast(physics_comp.physics_object.get());
			btMotionState* state = redundant_body->getMotionState();
			btCollisionShape* shape = redundant_body->getCollisionShape();
			bullet_data->world->removeRigidBody(redundant_body);
			if (redundant_body->getUserPointer()) {
				delete redundant_body->getUserPointer();
			}
			delete state;
			delete shape;
		}
		physics_comp.physics_object.reset((btCollisionObject*)body);
		break;
	}
	case PhysicsShapeType::CONVEX_HULL:
	{
		if (status != Mesh_status::READY) return false;
		auto template_shape = bullet_data->GetHullShape(path);
		btConvexHullShape* hull_collision_shape = new btConvexHullShape((btScalar*)template_shape->getPoints(),template_shape->getNumPoints(), sizeof(btVector3));
		btVector3 inertia;
		hull_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity);
		physics_comp.physics_shape = hull_collision_shape;
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, hull_collision_shape, inertia);
		body = new btRigidBody(info);
		body->setUserPointer(new PhysicsObjectInfo{ entity });
		if (physics_comp.is_kinematic && physics_comp.mass == 0.0f) {
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		}

		bullet_data->world->addRigidBody(body);
		if (physics_comp.physics_object) {
			btRigidBody* redundant_body = btRigidBody::upcast(physics_comp.physics_object.get());
			btMotionState* state = redundant_body->getMotionState();
			btCollisionShape* shape = redundant_body->getCollisionShape();
			bullet_data->world->removeRigidBody(redundant_body);
			if (redundant_body->getUserPointer()) {
				delete redundant_body->getUserPointer();
			}
			delete state;
			delete shape;
		}

		physics_comp.physics_object.reset((btCollisionObject*)body);
		break;
	}
	case PhysicsShapeType::CAPSULE_INNER:
	{
		if (status != Mesh_status::READY) return false;
		glm::vec3 box_size = bounding_box.GetBoxSize() / 2.0f;
		float radius;
		float height;
		radius = std::min(std::min(box_size.x, box_size.y), box_size.z);
		height = 2 * box_size.y - radius*2.0f;

		CustomInnnerCapsule* capsule_collision_shape = new CustomInnnerCapsule(radius, height);
		btVector3 inertia;
		capsule_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity, bounding_box.GetBoxOffset());
		physics_comp.physics_shape = capsule_collision_shape;
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, capsule_collision_shape, inertia);
		body = new btRigidBody(info);
		body->setUserPointer(new PhysicsObjectInfo{ entity });
		if (physics_comp.is_kinematic && physics_comp.mass == 0.0f) {
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		}
		bullet_data->world->addRigidBody(body);
		if (physics_comp.physics_object) {
			btRigidBody* redundant_body = btRigidBody::upcast(physics_comp.physics_object.get());
			btMotionState* state = redundant_body->getMotionState();
			btCollisionShape* shape = redundant_body->getCollisionShape();
			bullet_data->world->removeRigidBody(redundant_body);
			if (redundant_body->getUserPointer()) {
				delete redundant_body->getUserPointer();
			}
			delete state;
			delete shape;
		}
		physics_comp.physics_object.reset((btCollisionObject*)body);
		break;
	}
	case PhysicsShapeType::CAPSULE_OUTER:
	{
		if (status != Mesh_status::READY) return false;
		glm::vec3 box_size = bounding_box.GetBoxSize() / 2.0f;
		float radius;
		float height;
		radius = sqrtf(std::pow(box_size.x,2)+ std::pow(box_size.z, 2));
		height = 2*box_size.y;

		CustomOuterCapsule* capsule_collision_shape = new CustomOuterCapsule(radius, height);
		btVector3 inertia;
		capsule_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity, bounding_box.GetBoxOffset());
		physics_comp.physics_shape = capsule_collision_shape;
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, capsule_collision_shape, inertia);
		body = new btRigidBody(info);
		body->setUserPointer(new PhysicsObjectInfo{entity});
		if (physics_comp.is_kinematic && physics_comp.mass == 0.0f) {
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
		}
		bullet_data->world->addRigidBody(body);
		if (physics_comp.physics_object) {
			btRigidBody* redundant_body = btRigidBody::upcast(physics_comp.physics_object.get());
			btMotionState* state = redundant_body->getMotionState();
			btCollisionShape* shape = redundant_body->getCollisionShape();
			bullet_data->world->removeRigidBody(redundant_body);
			if (redundant_body->getUserPointer()) {
				delete redundant_body->getUserPointer();
			}
			delete state;
			delete shape;
		}
		physics_comp.physics_object.reset((btCollisionObject*)body);
		break;
	}
	default:
		throw std::runtime_error("This Collision type is not supported");
	}
	physics_comp.state = PhysicsObjectState::INITIALIZED;
	if ((bool)(physics_comp.props & PhysicsObjectProperties::DISABLE_COLLISION_RESPONSE)) {
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}

	if (physics_comp.auxilary_props) {
		SetAngularFactor(entity, physics_comp.auxilary_props->angular_factor);
		SetLinearFactor(entity, physics_comp.auxilary_props->linear_factor);
		SetFriction(entity, physics_comp.auxilary_props->friction);
		if (physics_comp.auxilary_props->mass == 0.0f) {
			SetMass(entity, physics_comp.mass);
		}
		else {
			SetMass(entity, physics_comp.auxilary_props->mass);
		}
		physics_comp.auxilary_props.reset();

	}
	return true;

}

//Not thread-safe
void PhysicsEngine::DestroyPhysicsObject(const PhysicsComponent& physics_comp)
{
	if (physics_comp.physics_object.get() == nullptr) return;
	btRigidBody* body = btRigidBody::upcast(physics_comp.physics_object.get());
	btCollisionShape* shape = body->getCollisionShape();
	btMotionState* state = body->getMotionState();
	bullet_data->world->removeRigidBody(body);
	if (body->getUserPointer()) {
		delete body->getUserPointer();
	}
	delete shape;
	delete state;
}

//Not thread-safe
void PhysicsEngine::StopSim()
{
	running = false;
}

//Not thread-safe
void PhysicsEngine::StartSim()
{
	running = true;
}

//Not thread-safe
void PhysicsEngine::ResetSim()
{
	for (int i = 0; i < bullet_data->world->getCollisionObjectArray().size(); i++)
	{
		btCollisionObject* colObj = bullet_data->world->getCollisionObjectArray()[i];
		Entity ent = ((PhysicsObjectInfo*)colObj->getUserPointer())->entity;
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
			body->activate();
			body->setLinearVelocity(vec);
			body->getMotionState()->getWorldTransform(trans);
			body->setCenterOfMassTransform(trans);
			body->setWorldTransform(trans);
		}
	}
}

//Not thread-safe
void PhysicsEngine::PassiveMode()
{
	StopSim();
	ResetSim();
}

//Not thread-safe
void PhysicsEngine::ActiveMode()
{
	ResetSim();
	StartSim();
}

//thread-safe
void PhysicsEngine::ApplyForce(Entity ent, const glm::vec3& direction)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Apply force");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		return;
	}
	btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
	btVector3 force = btVector3(direction.x, direction.y, direction.z);
	body->applyCentralForce(force);
}

//thread-safe
void PhysicsEngine::SetMass(Entity ent, float mass)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Set mass");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		if (!phys_comp.auxilary_props) {
			phys_comp.auxilary_props.reset(new AuxilaryPhysicsProps);
		}
		phys_comp.auxilary_props->mass = mass;
	}
	else {
		btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
		btVector3 inertia;
		body->getCollisionShape()->calculateLocalInertia(mass, inertia);
		body->setMassProps(mass, inertia);
		phys_comp.mass = mass;
	}
}

//thread-safe
void PhysicsEngine::SetFriction(Entity ent, float friction)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Set friction");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		if (!phys_comp.auxilary_props) {
			phys_comp.auxilary_props.reset(new AuxilaryPhysicsProps);
		}
		phys_comp.auxilary_props->friction = friction;
	}
	else {
		btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
		body->setFriction(friction);
	}
}

bool PhysicsEngine::IsObjectRecievingCollisions(Entity ent)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use IsObjectRecievingCollisions");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	return (bool)(phys_comp.props & PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS);
	return false;
}

void PhysicsEngine::SetRecieveCollision(Entity ent, bool enable)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use IsObjectRecievingCollisions");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		return;
	}
	else {
		if (enable) {
			phys_comp.props = phys_comp.props | PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS;
		}
		else {
			phys_comp.props = phys_comp.props & (~PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS);
		}
	}
}

bool PhysicsEngine::RayCast(const glm::vec3& from_in, const glm::vec3& to_in, PhysicsRayTestResultArray& results)
{
	btVector3 from = btVector3(from_in.x, from_in.y, from_in.z);
	btVector3 to = btVector3(to_in.x, to_in.y, to_in.z);

	btCollisionWorld::AllHitsRayResultCallback result(from, to);
	bullet_data->world->rayTest(from, to, result);
	if (!result.hasHit()) return false;
	for (int i = 0; i < result.m_collisionObjects.size();i++) {
		auto object = result.m_collisionObjects[i];
		if (object->getUserPointer()) {
			Entity ent = ((PhysicsObjectInfo*)object->getUserPointer())->entity;
			if (Application::GetWorld().EntityExists(ent)) {
				PhysicsRayTestResult res;
				res.ent = ent;
				btVector3 convert = result.m_hitPointWorld[i];
				res.position = glm::vec3(convert.x(), convert.y(), convert.z());
				convert = result.m_hitNormalWorld[i];
				res.normal = glm::vec3(convert.x(), convert.y(), convert.z());
				results.push_back(res);
			}
		}

	}
	return true;
}

bool PhysicsEngine::RayCastSingle(const glm::vec3& from_in, const glm::vec3& to_in, PhysicsRayTestResult& in_result)
{
	btVector3 from = btVector3(from_in.x, from_in.y, from_in.z);
	btVector3 to = btVector3(to_in.x, to_in.y, to_in.z);

	btCollisionWorld::ClosestRayResultCallback result(from, to);
	bullet_data->world->rayTest(from, to, result);
	if (!result.hasHit()) return false;
	auto object = result.m_collisionObject;
	if (object->getUserPointer()) {
		Entity ent = ((PhysicsObjectInfo*)object->getUserPointer())->entity;
		if (Application::GetWorld().EntityExists(ent)) {
			in_result.ent = ent;
			btVector3 convert = result.m_hitPointWorld;
			in_result.position = glm::vec3(convert.x(), convert.y(), convert.z());
			convert = result.m_hitNormalWorld;
			in_result.normal = glm::vec3(convert.x(), convert.y(), convert.z());
		}
	}
	return true;
}

//thread-safe
void PhysicsEngine::SetLinearFactor(Entity ent, const glm::vec3& linear_factor)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Set linear factor");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		if (!phys_comp.auxilary_props) {
			phys_comp.auxilary_props.reset(new AuxilaryPhysicsProps);
		}
		phys_comp.auxilary_props->linear_factor = linear_factor;
	}
	else {
		btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
		btVector3 factor = btVector3(linear_factor.x, linear_factor.y, linear_factor.z);
		body->setLinearFactor(factor);
	}
}

//thread-safe
void PhysicsEngine::SetAngularFactor(Entity ent, const glm::vec3& angular_factor)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Set angular factor");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		if (!phys_comp.auxilary_props) {
			phys_comp.auxilary_props.reset(new AuxilaryPhysicsProps);
		}
		phys_comp.auxilary_props->angular_factor = angular_factor;
	}
	else {
		btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
		btVector3 factor = btVector3(angular_factor.x, angular_factor.y, angular_factor.z);
		body->setAngularFactor(factor);
	}
}

//thread-safe
void PhysicsEngine::SetLinearVelocity(Entity ent, const glm::vec3& linear_velocity)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Set linear velocity");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		return;
	}
	btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
	btVector3 velocity = btVector3(linear_velocity.x, linear_velocity.y, linear_velocity.z);
	body->setLinearVelocity(velocity);
}

//thread-safe
void PhysicsEngine::SetAngularVelocity(Entity ent, const glm::vec3& angular_velocity)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Set angular velocity");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		return;
	}
	btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
	btVector3 velocity = btVector3(angular_velocity.x, angular_velocity.y, angular_velocity.z);
	body->setAngularVelocity(velocity);
}

glm::vec3 PhysicsEngine::GetAngularVelocity(Entity ent)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Get angular velocity");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		return glm::vec3(0.0f);
	}
	btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
	btVector3 velocity = body->getAngularVelocity();
	return glm::vec3(velocity.x(), velocity.y(), velocity.z());
}

glm::vec3 PhysicsEngine::GetLinearVelocity(Entity ent)
{
	auto& world = Application::GetWorld();
	if (!world.HasComponent<PhysicsComponent>(ent)) throw std::runtime_error("An entity requires a physics component to use Get linear velocity");
	auto& phys_comp = world.GetComponent<PhysicsComponent>(ent);
	if (phys_comp.state == PhysicsObjectState::UNINITIALIZED) {
		return glm::vec3(0.0f);
	}
	btRigidBody* body = btRigidBody::upcast(phys_comp.physics_object.get());
	btVector3 velocity = body->getLinearVelocity();
	return glm::vec3(velocity.x(), velocity.y(), velocity.z());
}

//Not thread-safe
void PhysicsEngine::RefreshObject(Entity ent)
{
	UnRegisterPhysicsComponent(ent);
	RegisterPhysicsComponent(ent);
}

//thread-safe
std::shared_ptr<btConvexHullShape> PhysicsEngine_BulletData::GetHullShape(const std::string& mesh_path)
{
	std::lock_guard<std::mutex> lock(convex_hull_shape_mutex);
	auto path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(mesh_path));
	auto absolute_path = FileManager::Get()->GetPathHash(path);
	auto fnd = convex_hull_shapes.find(path);
	if (fnd != convex_hull_shapes.end()) {
		return fnd->second;
	}

	std::string convex_hull_cache_path = FileManager::Get()->GetTempFilePath(absolute_path + ".hull");
	bool is_cached = std::filesystem::exists(convex_hull_cache_path);

	if (is_cached) {
		auto loaded_shape = LoadHullShape(convex_hull_cache_path);
		auto shape = std::shared_ptr<btConvexHullShape>(loaded_shape);
		convex_hull_shapes.insert(std::make_pair(path, shape));
		return shape;
	}
	else {
		MeshManager::mesh_native_input_data mesh_data = MeshManager::Get()->Fetch_Native_Data(FileManager::Get()->GetPath(mesh_path));
		btConvexHullShape convexHullShape;
		int pos_offset = mesh_data.layout.GetElement("position").offset;
		int stride = mesh_data.layout.stride;
		for (int i = 0; i < mesh_data.index_count; i++) {
			unsigned int current_index = mesh_data.index_buffer[i];
			btVector3* current_vertex = (btVector3*)((char*)mesh_data.vertex_buffer + ((long)stride * current_index) + pos_offset);
			convexHullShape.addPoint(*current_vertex,false);
		}
		convexHullShape.recalcLocalAabb();

		convexHullShape.setMargin(0);
		btShapeHull* hull = new btShapeHull(&convexHullShape); 
		hull->buildHull(0); 
		btConvexHullShape* pConvexHullShape = new btConvexHullShape((const btScalar*)hull->getVertexPointer(), hull->numVertices(), sizeof(btVector3));
		SaveHullShape(convex_hull_cache_path, pConvexHullShape);
		auto shape = std::shared_ptr<btConvexHullShape>(pConvexHullShape);
		convex_hull_shapes.insert(std::make_pair(path, shape));
		delete hull;
		return shape;

	}

}
//thread-safe
void PhysicsEngine_BulletData::SaveHullShape(const std::string& save_absolute_path, btConvexHullShape* hull)
{
	std::ofstream file(save_absolute_path, std::ios_base::binary | std::ios_base::out);
	if (!file.is_open()) throw std::runtime_error("File " + save_absolute_path + " could not be opened");
	file << "Convex_Hull_Cache\n";
	file << hull->getNumPoints() << "\n";
	file.write((char*)hull->getUnscaledPoints(), hull->getNumPoints() * sizeof(btVector3));
	file << "\nend\n";
	file.flush();

	file.close();
}
//thread-safe
btConvexHullShape* PhysicsEngine_BulletData::LoadHullShape(const std::string& save_absolute_path)
{
	std::ifstream file(save_absolute_path, std::ios_base::binary | std::ios_base::in);
	if (!file.is_open()) throw std::runtime_error("File " + save_absolute_path + " could not be opened");
	std::string check = "";

	file >> check;
	if (check != "Convex_Hull_Cache") throw std::runtime_error("Convex Hull cache at path " + save_absolute_path + " was corrupted");
	int num_of_points;
	file >> num_of_points;
	file.get();
	btVector3* vertex_data = (btVector3*)new char[num_of_points * sizeof(btVector3)];
	file.read((char*)vertex_data, num_of_points * sizeof(btVector3));
	file.get();
	file >> check;
	if (check != "end") throw std::runtime_error("Convex Hull cache at path " + save_absolute_path + " was corrupted");
	file.close();
	btConvexHullShape* shape = new btConvexHullShape((btScalar*)vertex_data, num_of_points, sizeof(btVector3));
	delete[] (char*)vertex_data;
	return shape;
}


