#include "PhysicsEngine.h"
#include <Events/MeshChangedEvent.h>
#include <World/Components/PhysicsComponent.h>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
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



	std::shared_ptr<btConvexHullShape> GetHullShape(const std::string& mesh_path);
	void SaveHullShape(const std::string& save_absolute_path, btConvexHullShape* hull);
	btConvexHullShape* LoadHullShape(const std::string& save_absolute_path);

};

class TransformMotionState : public btMotionState {
public:

	TransformMotionState(Entity owning_entity, const glm::vec3& offset = glm::vec3(0.0f)) : owning_entity(owning_entity), stored_size(1.0f) , shape_offset(offset) {}

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
		transform = glm::translate(glm::scale(transform, stored_size),-shape_offset);
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
		UnRegisterPhysicsComponent(e->ent);
		RegisterPhysicsComponent(e->ent);
		return false;
		}));
	Application::Get()->RegisterObserver<MeshChangedEvent>(mesh_change_observer.get());
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
		if (mesh_comp.GetMesh()->GetMeshStatus() != Mesh_status::READY) return false;
		glm::vec3 box_size = mesh_comp.GetMesh()->GetBoundingBox().GetBoxSize() / 2.0f;
		btVector3 half_extents = btVector3{ box_size.x,box_size.y,box_size.z };
		btBoxShape* box_collision_shape = new btBoxShape(half_extents);
		btVector3 inertia;
		box_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity,mesh_comp.GetMesh()->GetBoundingBox().GetBoxOffset());
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
	case PhysicsShapeType::CONVEX_HULL:
	{
		if (mesh_comp.GetMesh()->GetMeshStatus() != Mesh_status::READY) return false;
		auto template_shape = bullet_data->GetHullShape(mesh_comp.GetMeshPath());
		btConvexHullShape* hull_collision_shape = new btConvexHullShape((btScalar*)template_shape->getPoints(),template_shape->getNumPoints(), sizeof(btVector3));
		btVector3 inertia;
		hull_collision_shape->calculateLocalInertia(physics_comp.mass, inertia);
		btMotionState* motion_state = new TransformMotionState(entity);
		physics_comp.physics_shape = hull_collision_shape;
		btRigidBody::btRigidBodyConstructionInfo info(physics_comp.mass, motion_state, hull_collision_shape, inertia);
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

	btRigidBody* body = btRigidBody::upcast(physics_comp.physics_object.get());
	btCollisionShape* shape = body->getCollisionShape();
	btMotionState* state = body->getMotionState();
	bullet_data->world->removeRigidBody(body);
	delete shape;
	delete state;
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
		}
	}
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


