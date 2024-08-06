#pragma once 
#include <queue>
#include <entt/entt.hpp>
#include <Core/TypeList.h>
#include <World/EntityTypes.h>
#include <World/SceneGraph.h>
#include <World/SpatialIndex.h>
#include <World/PhysicsEngine.h>
#include <World/Components/InitializationComponent.h>
#include <World/Components/LabelComponent.h>
#include <World/Entity.h>
#include <mutex>
#include <glm/glm.hpp>
#include <World/SceneProxy.h>
#include <memory>
#include <utility>
#include <LuaEngine.h>
#include <LuaEngineUtilities.h>

class EntityType;
class World;

/**
 * @brief A template specializations of which are used to equip component types with callback on their creation destruction and update
 * @tparam T component type used in specializations
*/
template<typename T>
class ComponentInitProxy {
public:
	using not_defined = void; ///< when no specialization for type exists, it defaults to this template and as such presence of this definition is used to detect missing specialization

	/**
	 * @brief Component creation callback 
	 * @param world World in which the component was added to the entity 
	 * @param entity Entity to which the component was added
	*/
	static void OnCreate(World& world, Entity entity) {

	}

	/**
	 * @brief Component destruction callback 
	 * @param world World in which the component was added to the entity 
	 * @param entity Entity to which the component was added
	*/
	static void OnDestroy(World& world, Entity entity) {

	}

	/**
	 * @brief Component update callback
	 * @param world World in which the component was added to the entity
	 * @param entity Entity to which the component was added
	*/
	static void OnUpdate(World& world, Entity entity) {

	}

	static constexpr bool can_copy = false; ///< determines if component will be copied when copying entities

};

/**
 * @brief Determines when the Component has a specialization of ComponentInitProxy
 * @tparam T Component type
 * 
 * if this template is used it means @ref has_ComponentInitProxy_specialization_1 specialization was not used
 * and the component has a specialization of ComponentInitProxy
 * @see has_ComponentInitProxy_v
*/
template<typename T, typename = void>
struct has_ComponentInitProxy : std::true_type {};

/**
 * @brief Determines when the Component doesn't have a specialization of ComponentInitProxy
 * @tparam T Component type
 * 
 * If this specialization is used, it means there exists the not_defined definition which means the default ComponentInitProxy was used and no proxy specialization was defined
 * @see ComponentInitProxy::not_defined
 * @see has_ComponentInitProxy_v
 * @anchor has_ComponentInitProxy_specialization_1
*/
template<typename T>
struct has_ComponentInitProxy<T, std::void_t<typename ComponentInitProxy<T>::not_defined>> : std::false_type {};



/**
 * @brief Represents a boolean which marks whether the component has a ComponentInitProxy
 * @tparam T Component type 
*/
template<typename T>
constexpr bool has_ComponentInitProxy_v = has_ComponentInitProxy<T>::value;

/**
 * @brief A template value of which tells if the component has a ComponentInitProxy::OnCreate callback 
 * @tparam T component type
*/
template<typename T>
struct HasOnCreate
{
	template<typename U, U> struct helper;
	template<typename U> static std::uint8_t check(helper<void(*)(World&,Entity), &U::OnCreate>*); 
	template<typename U> static std::uint16_t check(...);
	static const bool value = sizeof(check<T>(0)) == sizeof(std::uint8_t); ///< value 
};

/**
 * @brief A template value of which tells if the component has a ComponentInitProxy::OnOnDestroy callback
 * @tparam T component type
*/
template<typename T>
struct HasOnDestroy
{
	template<typename U, U> struct helper;
	template<typename U> static std::uint8_t check(helper<void(*)(World&, Entity), &U::OnDestroy>*);
	template<typename U> static std::uint16_t check(...);
	static const bool value = sizeof(check<T>(0)) == sizeof(std::uint8_t); ///< value 
};

/**
 * @brief A template value of which tells if the component has a ComponentInitProxy::OnUpdate callback
 * @tparam T component type
*/
template<typename T>
struct HasOnUpdate
{
	template<typename U, U> struct helper;
	template<typename U> static std::uint8_t check(helper<void(*)(World&, Entity), &U::OnUpdate>*);
	template<typename U> static std::uint16_t check(...);
	static const bool value = sizeof(check<T>(0)) == sizeof(std::uint8_t); ///< value 
};

/**
 * @brief Enum which identifies what operation should be performed at a deffered World::RemoveEntity call
*/
enum class RemoveEntityAction : char {
	REMOVE = 0, ///< Remove the entity with all its children and prefab children
	RELOAD_PREFAB = 1, ///< Reload prefab by removing all its prefab children a loading them from the prefab file again
	REMOVE_PREFABS = 2, ///< Remove prefab component and all its prefab children, but retain the entity and its scene children
	RELOAD_ALL_PREFABS_OF_THIS_TYPE = 3, ///< Reload all prefabs with a type (used when a change in a prefab file was made)
	CHANGE_PREFAB = 5 ///< Same as RemoveEntityAction::RELOAD_PREFAB ,but instead of loading the same prefab load a different one.
};

/**
 * @brief The world represents the current state of the loaded scene and its contents
 * 
 * It containts the ECS(Entity Component System) which stores data on all entities, SceneGraph which stores the scene hierarchy, SpatialIndex which accelerates spatial lookups,
 * PhysicsEngine which simulates and keeps track of physically simulated objects, and takes care of creating destroying and manipulating entities within the scene.
 * 
 * @warning A world instance can not be copied of moved
*/
class World {
	
	/**
	 * @brief Returns a mutex specific to a type
	 * 
	 * Used for synchronizing access to components
	 * @tparam T Type to return a unique mutex for
	*/
	template<typename T>
	std::mutex& SyncPool() {
		static std::mutex pool_lock;
		return pool_lock;
	}

public:
	/**
	 * @brief World instance initialization, which runs after construction
	 * @todo Can be merged into the constructor
	*/
	void Init();

	/**
	 * @brief World instance constructor
	*/
	World();

	/**
	 * @brief World instance destructor
	*/
	~World();


	World(const World& ref) = delete;
	World(World&& ref) = delete;
	World& operator=(const World& ref) = delete;
	World& operator=(World&& ref) = delete;

	/**
	 * @brief Calculates new worldspace transform matricies for all entities from the scene hierarchy adn local transforms
	 * 
	 * @see SceneGraph
	*/
	void UpdateTransformMatricies();

	/**
	 * @brief Runs update on a scene specific Lua script
	 * @param delta_time 
	*/
	void UpdateSceneScript(float delta_time);

	/**
	 * @brief Sets the local translation of the Entity without synchronization
	 * @param ent Entity to set the translation of
	 * @param translation local translation to set
	 * @warning This method is not thread-safe and should only be used when it is guaranteed the entity data wont be accessed across multiple threads.
	*/
	void SetEntityTranslation(Entity ent, const glm::vec3& translation);

	/**
	 * @brief Sets the local translation of the Entity with synchronization
	 * @param ent Entity to set the translation of
	 * @param translation local translation to set
	 * @note This function is threadsafe if no other thread uses the SetEntityTranslation function
	*/
	void SetEntityTranslationSync(Entity ent, const glm::vec3& translation);

	/**
	 * @brief Sets the local rotation of the Entity without synchronization
	 * @param ent Entity to set the rotation of
	 * @param rotation local rotation to set
	 * @warning This method is not thread-safe and should only be used when it is guaranteed the entity data wont be accessed across multiple threads.
	*/
	void SetEntityRotation(Entity ent, const glm::quat& rotation);

	/**
	 * @brief Sets the local rotation of the Entity with synchronization
	 * @param ent Entity to set the rotation of
	 * @param rotation local rotation to set
	 * @note This function is threadsafe if no other thread uses the SetEntityRotation function
	*/
	void SetEntityRotationSync(Entity ent, const glm::quat& rotation);

	/**
	 * @brief Sets the local scale of the Entity without synchronization
	 * @param ent Entity to set the scale of
	 * @param scale local scale to set
	 * @warning This method is not thread-safe and should only be used when it is guaranteed the entity data wont be accessed across multiple threads.
	*/
	void SetEntityScale(Entity ent, const glm::vec3& scale);
	
	/**
	 * @brief Sets the local scale of the Entity with synchronization
	 * @param ent Entity to set the scale of
	 * @param scale local scale to set
	 * @note This function is threadsafe if no other thread uses the SetEntityScale function
	*/
	void SetEntityScaleSync(Entity ent, const glm::vec3& scale);

	/**
	 * @brief Sets the world transform of the Entity without synchronization
	 * @param ent Entity to set the transform of
	 * @param transform world transform to set
	 * @note cant be used in conjunction with scale, rotation and translation since it overrides them and vice versa
	 * @warning This method is not thread-safe and should only be used when it is guaranteed the entity data wont be accessed across multiple threads.
	*/
	void SetEntityTransform(Entity ent, const glm::mat4& transform);

	/**
	 * @brief Sets the world transform of the Entity with synchronization
	 * @param ent Entity to set the transform of
	 * @param transform world transform to set
	 * @note cant be used in conjunction with scale, rotation and translation since it overrides them and vice versa
	 * @note This function is threadsafe if no other thread uses the SetEntityTransform function
	*/
	void SetEntityTransformSync(Entity ent, const glm::mat4& transform);

	/**
	 * @brief Changes or sets the MeshComponent of the Entity to the mesh at the filepath
	 * @param ent Entity to set the MeshComponent of
	 * @param mesh the filepath to the mesh
	*/
	void SetEntityMesh(Entity ent, const std::string mesh);

	/**
	 * @brief Changes or sets the SkeletalMeshComponent and default animation of the Entity to the skeletal mesh at the filepath
	 * @param ent Entity to set the SkeletalMeshComponent of
	 * @param mesh the filepath to the skeletal mesh
	 * @param default_animation_path The filepath to the default animation (T-pose if none specified)
	*/
	void SetEntitySkeletalMesh(Entity ent, const std::string& mesh, const std::string& default_animation_path = "");

	/**
	 * @brief Changes the prefab file of the entity and reloads it
	 * @param ent 
	 * @param prefab_path 
	 * @see Uses RemoveEntityAction::CHANGE_PREFAB
	*/
	void ResetEntityPrefab(Entity ent, const std::string& prefab_path);

	/**
	 * @brief When mesh changes after it has asynchronously been loaded this function is called to update it in the SpatialIndex, since its bounding box may have changes
	 * @param ent Entity to update the Mesh of
	 * 
	 * @todo make sure this function gets called at proper times, since currently it only updated when its about to get rendered
	*/
	void UpdateMesh(Entity ent);

	/**
	 * @brief When skeletal mesh changes after it has asynchronously been loaded this function is called to update it in the SpatialIndex, since its bounding box may have changes
	 * @param ent Entity to update the SkeletalMesh of
	 *
	 * @todo make sure this function gets called at proper times, since currently it only updated when its about to get rendered
	*/
	void UpdateSkeletalMesh(Entity ent);

	/**
	 * @brief Duplicates a non prefab Entity alongside with prefab and scene children
	 * @param ent Entity to duplicate
	 * @param parent parent to the new Entity, if left at default the parent of to copied entity is used
	 * @return the new Entity
	 * @warning undefined behaviour: when prefab child is used 
	 * @todo fix undefined behaviour
	*/
	Entity DuplicateEntity(Entity ent, Entity parent = Entity());

	/**
	 * @brief Duplicates a prefab Entity alongside with prefab and scene children
	 * @param ent Entity to duplicate
	 * @param parent parent to the new Entity, if left at default the parent of to copied entity is used
	 * @return the new Entity
	 * @warning undefined behaviour: when non prefab child is used 
	 * @todo fix undefined behaviour
	*/
	Entity DuplicateEntityInPrefab(Entity ent, Entity parent = Entity());

	/**
	 * @brief Check if entity contains a valid entity id
	 * @param ent Entity to check
	*/
	bool EntityIsValid(Entity ent);

	/**
	 * @brief Registers callbacks specified in components ComponentInitProxy specialization 
	 * @tparam T component type
	 * @anchor RegisterComponentType_specialization_1
	*/
	template<typename T>
	auto RegisterComponentType() -> std::enable_if_t<has_ComponentInitProxy_v<T>> {
		m_ECS.storage<T>();
		if constexpr (HasOnCreate<ComponentInitProxy<T>>::value) {
			m_ECS.on_construct<T>().template connect<&World::ConstructComponent<T, &ComponentInitProxy<T>::OnCreate>>(*this);
		}
		if constexpr (HasOnDestroy<ComponentInitProxy<T>>::value) {
			m_ECS.on_destroy<T>().template connect<&World::DestroyComponent<T, &ComponentInitProxy<T>::OnDestroy>>(*this);
		}
		if constexpr (HasOnUpdate<ComponentInitProxy<T>>::value) {
			m_ECS.on_destroy<T>().template connect<&World::UpdateComponent<T, &ComponentInitProxy<T>::OnUpdate>>(*this);
		}
	}

	/**
	 * @brief Registers components which dont have ComponentInitProxy specializations, serves as fallback to @ref RegisterComponentType_specialization_1
	 * @tparam T component type
	*/
	template<typename T>
	auto RegisterComponentType() -> std::enable_if_t<!has_ComponentInitProxy_v<T>> {
		m_ECS.storage<T>();
	}

	/**
	 * @brief Make a new entity from an EntityType
	 * @tparam T EntityType to create an entity of
	 * @tparam ...Args EntityType constructor parameters types
	 * @param parent Parent of the new entity, root if set to Entity() or left out
	 * @param ...args EntityType constructor arguments
	 * @return the new entity
	 * @todo does this not need InitializationComponent
	*/
	template<typename T = EntityType, typename ... Args>
	auto CreateEntity(Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(),std::declval<Entity>(), std::declval<Entity>(),std::declval<Args>()...), Entity())
	{
		auto ent = MakeEmptyEntity();
		T::CreateEntity(*this,ent,parent,std::forward<Args>(args)...);
		return ent;
	}
	/**
	 * @brief Make a new named entity from an EntityType
	 * @tparam T EntityType to create an entity of
	 * @tparam ...Args EntityType constructor parameters types
	 * @param name Name to assign to the entity as LabelComponent
	 * @param parent Parent of the new entity, root if set to Entity() or left out
	 * @param ...args EntityType constructor arguments
	 * @return the new entity
	 * @todo does this not need InitializationComponent
	*/
	template<typename T = EntityType, typename ... Args>
	auto CreateEntity(const std::string& name, Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		auto ent = MakeEmptyEntity();
		T::CreateEntity(*this, ent, parent, std::forward<Args>(args)...);
		SetComponent<LabelComponent>(ent, name);
		return ent;
	}

	/**
	 * @brief Initialize an existing entity from an EntityType
	 * @tparam T EntityType to initialize an entity from
	 * @tparam ...Args EntityType constructor parameters types
	 * @param ent the entity to initialize
	 * @param parent Parent of the new entity, root if set to Entity() or left out
	 * @param ...args EntityType constructor arguments
	 * @return the new entity
	 * @todo Check if entity is actually empty
	*/
	template<typename T = EntityType, typename ... Args>
	auto CreateEntityFromEmpty(Entity ent, Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		T::CreateEntity(*this, ent, parent, std::forward<Args>(args)...);
		SetComponent<InitializationComponent>(ent);
		return ent;
	}

	/**
	 * @brief Initialize an existing entity from an EntityType and assign a name
	 * @tparam T EntityType to initialize an entity from
	 * @tparam ...Args EntityType constructor parameters types
	 * @param name the name to give the entity
	 * @param ent the entity to initialize
	 * @param parent Parent of the new entity, root if set to Entity() or left out
	 * @param ...args EntityType constructor arguments
	 * @return the new entity
	 * @todo Check if entity is actually empty
	*/
	template<typename T = EntityType, typename ... Args>
	auto CreateEntityFromEmpty(const std::string& name, Entity ent, Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		T::CreateEntity(*this, ent, parent, std::forward<Args>(args)...);
		SetComponent<InitializationComponent>(ent);
		SetComponent<LabelComponent>(ent, name);
		return ent;
	}

	/**
	 * @brief Get the SceneGraph representing the scene hierarchy
	*/
	SceneGraph* GetSceneGraph() {
		return &m_SceneGraph;
	}

	/**
	 * @brief Execute Remove action based on the action parameter 
	 * @param entity entity to apply the action on
	 * @param action action to perform the Remove with @see RemoveEntityAction
	*/
	void RemoveEntity(Entity entity, RemoveEntityAction action = RemoveEntityAction::REMOVE);

	/**
	 * @brief Reload all prefabs loaded from a prefab path
	 * @param prefab_path path used to load the prefabs
	*/
	void ReloadPrefabs(const std::string& prefab_path);

	/**
	 * @brief Assign or replace a component of an Entity
	 * @tparam T Component to replace or assign
	 * @tparam ...Args Parameter types of the component constructor
	 * @param entity entity to assign the component to 
	 * @param ...args Arguments of the component constructor
	*/
	template<typename T, typename ... Args>
	void SetComponent(Entity entity, Args&& ... args) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		m_ECS.emplace_or_replace<T>((entt::entity)entity.id, std::forward<Args>(args)...);
	}

	/**
	 * @brief Gets a mutex unique to a type
	 * @tparam T type for which to return a unique mutex
	 * @return unique mutex
	 * @see World::SyncPool
	*/
	template<typename T>
	std::mutex& GetPoolSync() {
		return SyncPool<T>();
	}

	/**
	 * @brief Check if an Entity contains a Component with synchronization
	 * @tparam T Type of component to check 
	 * @param ent entity to check 
	 * @note This method is thread-safe if HasComponent is not used in parallel with it
	*/
	template<typename T>
	bool HasComponentSynced(Entity ent) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		return m_ECS.any_of<T>((entt::entity)ent.id);
	}

	/**
	 * @brief Check if an Entity contains a Component without synchronization
	 * @tparam T Type of component to check
	 * @param ent entity to check
	 * @warning This method is not thread-safe and should only be used when it is guaranteed the entity data wont be accessed across multiple threads.
	*/
	template<typename T>
	bool HasComponent(Entity ent) {
		return m_ECS.any_of<T>((entt::entity)ent.id);
	}

	/**
	 * @brief Gets a component of an entity without synchronization
	 * @tparam T component type to get
	 * @param entity entity to get the component of
	 * @warning This method is not thread-safe and should only be used when it is guaranteed the entity data wont be written from other threads.
	*/
	template<typename T>
	T& GetComponent(Entity entity) {
		return m_ECS.get<T>((entt::entity)entity.id);
	}

	/**
	 * @brief Gets a component of an entity with synchronization
	 * @tparam T component type to get
	 * @param entity entity to get the component of
	 * @note This method is thread-safe if HasComponent is not used in parallel with it
	*/
	template<typename T>
	T& GetComponentSync(Entity entity) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		return m_ECS.get<T>((entt::entity)entity.id);
	}

	/**
	 * @brief Removes a component of an entity
	 * @tparam T component to remove
	 * @param entity entity to remove the component from 
	*/
	template<typename T>
	void RemoveComponent(Entity entity) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		m_ECS.remove<T>((entt::entity)entity.id);
	}

	/**
	 * @brief Get the Entity Component System implemntation
	 * 
	 * @note Should only be used in case World doesnt not expose enough functionality, but should generally be avoided
	 * @warning improper utilization of this implementation can cause undefined behaviour
	 * @todo Remove from public API
	*/
	entt::registry& GetRegistry() {
		return m_ECS;
	}

	/**
	 * @brief Returns a current SceneProxy representing the info of the currently loaded scene.
	*/
	const SceneProxy& GetCurrentSceneProxy() const {
		return *current_scene;
	}

	/**
	 * @brief Load a Scene into the world from a file
	 * 
	 * This also unload the current scene
	 * 
	 * @param file_path path to the new scene
	*/
	void LoadSceneFromFile(const std::string& file_path = "Empty");

	/**
	 * @brief Closes the current scene and replaces it with a blank scene
	*/
	void LoadEmptyScene();

	/**
	 * @brief Saves the current serializable state of the world into a scene file
	 * @param file_path path to save the scene file
	*/
	void SaveScene(const std::string& file_path);

	/**
	 * @brief Sets the primary entity from the viewpoint of which the scene gets rendered
	 * @param entity new primary entity
	 * @warning This entity must have a CameraComponent
	*/
	void SetPrimaryEntity(Entity entity);

	/**
	 * @brief Check if scene load process has finished
	 * @return 
	 * @todo Is this not flipped ?
	*/
	bool IsSceneLoaded() const {
		//return load_scene.get(); // Previously
		return !load_scene.get(); // This is probably how it should be
	}
	
	/**
	 * @brief Get the primary Entity
	 * @return Primary Entity
	*/
	Entity GetPrimaryEntity() const {
		return primary_entity;
	}

	/**
	 * @brief Creates a new entity empty in the ECS 
	 * 
	 * @note this does not associate entity into the scene hierarchy, so a World::CreateEntityFromEmpty should be called on it afterwards
	*/
	Entity MakeEmptyEntity();

	/**
	 * @brief Checks if entity exists in the ECS
	 * @param entity entity to check
	 * @todo duplicate of World::EntityIsValid
	*/
	bool EntityExists(Entity entity) {
		return m_ECS.valid((entt::entity)entity.id);
	}

	/**
	 * @brief Marks Entity local transformation elements(translation, rotation, scale) or the transform itself as changed to recompute a new world space matrix
	 * @param entity entity to mark dirty
	 * @param dirty_transform if we want to manipulate the transform directly to i.e. sync entity movement with another entity, we can update the world space matrix directly
	 * @note dirty_transform disregards the transformation elements(translation, rotation, scale) 
	 * and prevents scenegraph from changing the transform matrix while still updating its children
	*/
	void MarkEntityDirty(Entity entity, bool dirty_transform = false);

	/**
	 * @brief Takes the current prefab structure under an entity and saves it into a prefab file
	 * @param entity Prefab entity
	 * @param path path to save the prefab
	*/
	void SerializePrefab(Entity entity, const std::string& path);

	/**
	 * @brief Gets the SpatialIndex associated with the world
	*/
	SpatialIndex& GetSpatialIndex() {
		return m_SpatialIndex;
	}

	/**
	 * @brief Gets the Physiscs engine associated with the world
	*/
	PhysicsEngine& GetPhysicsEngine() {
		return m_PhysicsEngine;
	}

	/**
	 * @brief Ensure the Primary entity has a valid CameraComponent and if not reset the Primary entity to a temporary default camera.
	*/
	void CheckCamera();

	/**
	 * @brief Check if the current scene has a scene script
	 * @return 
	*/
	bool HasSceneScript() const {
		return has_script;
	}

private:
	friend class GameLayer;
#ifdef EDITOR
	friend class Editor;
#endif
	/**
	 * @brief Binds Engine functionality into the Scene LuaEngine
	*/
	void BindLuaFunctions();
	/**
	 * @brief Resets the Scene LuaEngine on Scene Load
	*/
	void ResetLuaEngine();


	/**
	 * @brief Wrapper for the ComponentInitProxy::OnCreate
	*/
	template<typename T, auto func>
	void ConstructComponent(entt::registry& reg, entt::entity ent) {
		func(*this, Entity((uint32_t)ent));
	}

	/**
	 * @brief Wrapper for the ComponentInitProxy::OnDestroy
	*/
	template<typename T, auto func>
	void DestroyComponent(entt::registry& reg, entt::entity ent) {
		func(*this, Entity((uint32_t)ent));
	}

	/**
	 * @brief Wrapper for the ComponentInitProxy::OnUpdate
	*/
	template<typename T, auto func>
	void UpdateComponent(entt::registry& reg, entt::entity ent) {
		func(*this, Entity((uint32_t)ent));
	}

	/**
	 * @brief Template to Register multiple component types
	 * @tparam ...Args Component Types
	 * @param list type list
	 * @see World::RegisterComponentType
	*/
	template<typename ... Args>
	void RegisterComponents(TypeList<Args...> list) {
		(RegisterComponentType<Args>(),...);
	}
	
	/**
	 * @brief Recursive template for copying components between entities
	*/
	template<typename T, typename ... Args>
	void DuplicateComponentRecursive(Entity from, Entity to);

	/**
	 * @brief Recursive template for copying components between entities
	*/
	template<typename ... Args>
	void DuplicateComponentsRecursive(Entity from, Entity to, TypeList<Args...> types) {
		 DuplicateComponentRecursive<Args...>(from, to);
	}

	/**
	 * @brief Internal function used to produce serializable output for individual Prefav childred
	*/
	void SerializePrefabChild(Entity child, std::vector<std::pair<std::string, std::string>>& file_structure);

	/**
	 * @brief Called by the engine to Process a deffered LoadScene request
	*/
	void LoadSceneSystem();

	/**
	 * @brief Internal structure to store deffered Remove requests
	*/
	struct RemoveEntityRequest {
		Entity entity;
		RemoveEntityAction action;
	};
	
	/**
	 * @brief Called by the engine to Process a deffered Deletion request 
	*/
	void DeletionSystem();

	/**
	 * @brief Internal function used by the DeletionSystem
	*/
	void DeleteNode(SceneNode* node);

	/**
	 * @brief Processes deffered SetPrimary entity request
	*/
	void SetPrimaryEntitySystem();

	Entity set_primary_entity = Entity(); ///< Primary entity to set 
	Entity primary_entity = Entity(); ///< Current Primary entity 
	Entity default_camera = Entity(); ///< Default Primary Entity used when Current Primary Entity is not valid

	LuaEngine scene_lua_engine; ///< Scene Script LuaEngine
	bool has_script = false; ///< Whether current scene has a Script

	std::shared_ptr<SceneProxy> current_scene = nullptr; ///< Current Scene load info
	std::shared_ptr<SceneProxy> load_scene = nullptr; ///< Scene to be loaded info
	std::mutex deletion_mutex; 
	std::queue<RemoveEntityRequest> deletion_queue;
	std::mutex entity_mutex;
	SceneGraph m_SceneGraph; 
	PhysicsEngine m_PhysicsEngine; 
	SpatialIndex m_SpatialIndex; 
	entt::registry m_ECS;
};