#pragma once
#include <Events/SubjectObserver.h>
#include <World/Entity.h>
#include <deque>
#include <mutex>
#include <World/Components/PhysicsComponent.h>
#include <vector>

struct PhysicsComponent;
struct PhysicsEngine_BulletData;

/**
 * @brief The initialization properties of the PhysicsEngine
 * @note not currently used
*/
struct PhysicsEngineProps {

};

/**
 * @brief Additional information attached to each Physics object 
*/
struct PhysicsObjectInfo {
	Entity entity; ///< Used to translate between Bullet physics objects and entities
};

/**
 * @brief Result of a RayTest performed by the Physics Engine
 * 
 * @note Unlike Raycast in a SpatialIndex, this only applies on simulated objects, but is tested against their collider instead of a bounding box so it can be more precise.
*/
struct PhysicsRayTestResult {
	glm::vec3 position; ///< Position of where a Physics object was hit
	glm::vec3 normal; ///< Normal of the surface on the hit point
	Entity ent = Entity(); ///< Entity that was hit
};

/**
 * @brief Typedef for a PhysicsRayTestResult vector as a result of PhysicsEngine::RayCast
*/
using PhysicsRayTestResultArray = typename std::template vector<PhysicsRayTestResult>;

/**
 * @brief Object encapsulating a Bullet Physics simulation world, responsible for all Physics simulation in the World
*/
class PhysicsEngine {
public:

	/**
	 * @brief Create an Empty Physics world
	 * @param props 
	*/
	PhysicsEngine(const PhysicsEngineProps& props);

	/**
	 * @brief Run a Physics timestep. 
	 * @param delta_time time difference between the last and the current frame in milliseconds
	 * 
	 * Internally a multiple fixed timesteps can be taken or even skipped in order to allows more stable physics simulation. 
	 * The higher the delta_time is the more timesteps are taken. Bullet then handles interpolation to make it appear as if variable timesteps were being used, 
	 * by interpolating the difference.
	*/
	void UpdatePhysics(float delta_time);

	/**
	 * @brief Pause the simulation
	*/
	void StopSim();
	
	/**
	 * @brief Resume the simulation
	*/
	void StartSim();

	/**
	 * @brief Reset all simulated objects to their initial position and state
	*/
	void ResetSim();

	/**
	 * @brief Pause and Reset the simulation
	 * 
	 * Used by the Editor to allow for editing of simulated objects.
	*/
	void PassiveMode();

	/**
	 * @brief Reset and Start the simulation
	*/
	void ActiveMode();

	/**
	 * @brief Applies a force in a direction on a simulated object
	 * @param ent Simulated Entity to apply force onto
	 * @param direction Direction with magnitude to apply the force in.
	*/
	void ApplyForce(Entity ent, const glm::vec3& direction);

	/**
	 * @brief Sets the mass of a simulated object
	 * @param ent Simulated Entity to set the mass of
	 * @param mass The mass to set
	*/
	void SetMass(Entity ent, float mass);

	/**
	 * @brief Sets the friction coefficient of a simulated object
	 * @param ent Simulated Entity to set the friction coefficient of
	 * @param friction The friction coefficient to set
	*/
	void SetFriction(Entity ent, float friction);

	/**
	 * @brief Check if an objects recieves a callback on collision
	 * @param ent Entity to check 
	*/
	bool IsObjectRecievingCollisions(Entity ent);

	/**
	 * @brief Enables/Disables a callback for an entity on collision
	 * @param ent entity to enable/disable the callback on 
	 * @param enable true if enable, false if disable
	*/
	void SetRecieveCollision(Entity ent, bool enable);

	/**
	 * @brief Casts a Ray and scans for all physics object intersections
	 * @param from point at the beginning of the ray
	 * @param to point at the end of the ray
	 * @param results the vector of hit entities and their hit info of type PhysicsRayTestResult
	 * @return Whether at least one intersection was detected
	*/
	bool RayCast(const glm::vec3& from, const glm::vec3& to, PhysicsRayTestResultArray& results);

	/**
	 * @brief Casts a Ray and scans for closest physics object intersection
	 * @param from point at the beginning of the ray
	 * @param to point at the end of the ray
	 * @param results the hit entity and their its info of type PhysicsRayTestResult
	 * @return Whether an intersection was detected
	*/
	bool RayCastSingle(const glm::vec3& from, const glm::vec3& to, PhysicsRayTestResult& results);

	/**
	 * @brief Set the factor of linear movement restricting a simulated objects translation
	 * @param ent Simulated Entity to set the linear factor of
	 * @param linear_factor linear factor to set
	*/
	void SetLinearFactor(Entity ent, const glm::vec3& linear_factor);

	/**
	 * @brief Set the factor of angular movement restricting a simulated objects rotation
	 * @param ent Simulated Entity to set the angular factor of
	 * @param angular_factor angular factor to set
	*/
	void SetAngularFactor(Entity ent, const glm::vec3& angular_factor);

	/**
	 * @brief Sets the objects linear velocity
	 * @param ent Simulated Entity to set th  linear velocity of
	 * @param linear_velocity linear velocity to set
	*/
	void SetLinearVelocity(Entity ent, const glm::vec3& linear_velocity);
	
	/**
	 * @brief Sets the objects angular velocity
	 * @param ent Simulated Entity to set the angular velocity of
	 * @param angular_velocity angular velocity to set
	*/
	void SetAngularVelocity(Entity ent, const glm::vec3& angular_velocity);

	/**
	 * @brief Get the angular velocity of an object 
	 * @param ent Simulated Entity of which to get the angular velocity
	 * @return the angular velocity of ent
	*/
	glm::vec3 GetAngularVelocity(Entity ent);

	/**
	 * @brief Get the linear velocity of an object
	 * @param ent Simulated Entity of which to get the linear velocity
	 * @return the linear velocity of ent
	*/
	glm::vec3 GetLinearVelocity(Entity ent);

	/**
	 * @brief Removes and Adds an Entity to and from a simulation world, used for syncing an entity after a change was made to it
	 * @param ent Entity to refresh
	*/
	void RefreshObject(Entity ent);

	/**
	 * @brief Checks if the physics simulation is paused or running
	*/
	bool IsPhysicsActive() const { return running; }

	/**
	 * @brief Add an entity with a PhysicsComponent to this simulation world, this action is deffered
	 * @param ent Entity to add 
	 * @warning ent needs to have a PhysicsComponent
	*/
	void RegisterPhysicsComponent(Entity ent);

	/**
	 * @brief Remove an entity with a PhysicsComponent from this simulation world, this action is deffered
	 * @param ent Entity to Remove
	 * @warning ent must a registered simulated Entity
	*/
	void UnRegisterPhysicsComponent(Entity ent);

	/**
	 * @brief PhysicsEngine Destructor, deletes all simulated objects and the world itself.
	*/
	~PhysicsEngine();

private:
	/**
	 * @brief calls PhysicsEngine::destroy and then reinitializes the PhysicsEngine with an empty physics world again
	*/
	void clear();

	/**
	 * @brief deletes all simulated objects and the world itself, is called by the destructor 
	*/
	void destroy();

	friend class World;
	friend class TransformMotionState;

	/**
	 * @brief Represents a deffered request to add an Entity to a physics world
	 * @see PhysicsEngine::RegisterPhysicsComponent
	*/
	struct creation_queue_entry {
		Entity entity; ///< Entity to add
		bool processed = false; ///< state of this request
	};

	/**
	 * @brief Represents a deffered request to remove an Entity from a physics world
	 * @see PhysicsEngine::UnRegisterPhysicsComponent
	*/
	struct deletion_queue_entry {
		PhysicsComponent phys_comp; ///< since the removed entity no longer has a PhysicsComponent, its copy is stored in order to remove its data in the PhysicsEngine later
	};

	void CreationPhase(); ///< internal function Processes PhysicsEngine::creation_queue_entry requests
	void DeletionPhase(); ///< internal function Processes PhysicsEngine::deletion_queue_entry requests

	void PhysicsCollisionCallbackPhase(); ///< internal function Callback from Bullet which informs the Engine of all collisions so the Engine can dispatch collision events

	bool CreatePhysicsObject(Entity entity); ///< Internally called by PhysicsEngine::CreationPhase
	void DestroyPhysicsObject(const PhysicsComponent& physics_comp); ///< Internally called by PhysicsEngine::DeletionPhase

	bool running = true; ///< state of the simulation (false = paused, true = running)

	std::unique_ptr<EventObserverBase> mesh_change_observer; ///< Observes MeshChangedEvent, to update colliders

	PhysicsEngine_BulletData* bullet_data; ///< Abstracted internal state of Bullet Physics Engine
	std::mutex creation_mutex; ///< Mutex for PhysicsEngine::creation_queue
	std::deque<creation_queue_entry> creation_queue; ///< Queue containing PhysicsEngine::creation_queue_entry

	std::mutex deletion_mutex; ///< Mutex for PhysicsEngine::deletion_queue
	std::deque<deletion_queue_entry> deletion_queue; ///< Queue containing PhysicsEngine::deletion_queue_entry

};