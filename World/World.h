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

class EntityType;
class World;

template<typename T>
class ComponentInitProxy {
public:
	using not_defined = void;

	static void OnCreate(World& world, Entity entity) {

	}

	static void OnDestroy(World& world, Entity entity) {

	}

};

template<typename T, typename = void>
struct has_ComponentInitProxy : std::true_type {};

template<typename T>
struct has_ComponentInitProxy<T, std::void_t<typename ComponentInitProxy<T>::not_defined>> : std::false_type {};

template<typename T>
constexpr bool has_ComponentInitProxy_v = has_ComponentInitProxy<T>::value;

template<typename T>
struct HasOnCreate
{
	template<typename T, T> struct helper;
	template<typename T> static std::uint8_t check(helper<void(*)(World&,Entity), &T::OnCreate>*); 
	template<typename T> static std::uint16_t check(...);
	static const bool value = sizeof(check<T>(0)) == sizeof(std::uint8_t);
};

template<typename T>
struct HasOnDestroy
{
	template<typename T, T> struct helper;
	template<typename T> static std::uint8_t check(helper<void(*)(World&, Entity), &T::OnDestroy>*);
	template<typename T> static std::uint16_t check(...);
	static const bool value = sizeof(check<T>(0)) == sizeof(std::uint8_t);
};

enum class RemoveEntityAction : char {
	REMOVE = 0, RELOAD_PREFAB = 1, REMOVE_PREFABS = 2, RELOAD_ALL_PREFABS_OF_THIS_TYPE = 3
};

class World {
	template<typename T>
	std::mutex& SyncPool() {
		static std::mutex pool_lock;
		return pool_lock;
	}

public:
	void Init();

	World();
	~World();

	World(const World& ref) = delete;
	World(World&& ref) = delete;
	World& operator=(const World& ref) = delete;
	World& operator=(World&& ref) = delete;

	void UpdateTransformMatricies();

	void SetEntityTranslation(Entity ent, const glm::vec3& translation);

	void SetEntityTranslationSync(Entity ent, const glm::vec3& translation);

	void SetEntityRotation(Entity ent, const glm::quat& rotation);

	void SetEntityRotationSync(Entity ent, const glm::quat& rotation);

	void SetEntityScale(Entity ent, const glm::vec3& scale);

	void SetEntityScaleSync(Entity ent, const glm::vec3& scale);

	void SetEntityTransform(Entity ent, const glm::mat4& transform);

	void SetEntityTransformSync(Entity ent, const glm::mat4& transform);

	template<typename T>
	auto RegisterComponentType() -> std::enable_if_t<!has_ComponentInitProxy_v<T>> {
		m_ECS.storage<T>();
	}

	template<typename T>
	auto RegisterComponentType() -> std::enable_if_t<has_ComponentInitProxy_v<T>> {
		m_ECS.storage<T>();
		if constexpr (HasOnCreate<ComponentInitProxy<T>>::value) {
			m_ECS.on_construct<T>().connect<&World::ConstructComponent<T, &ComponentInitProxy<T>::OnCreate>>(*this);
		}
		if constexpr (HasOnDestroy<ComponentInitProxy<T>>::value) {
			m_ECS.on_destroy<T>().connect<&World::DestroyComponent<T, &ComponentInitProxy<T>::OnDestroy>>(*this);
		}

	}

	template<typename T = EntityType, typename ... Args>
	auto CreateEntity(Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(),std::declval<Entity>(), std::declval<Entity>(),std::declval<Args>()...), Entity())
	{
		auto ent = MakeEmptyEntity();
		T::CreateEntity(*this,ent,parent,std::forward<Args>(args)...);
		return ent;
	}

	template<typename T = EntityType, typename ... Args>
	auto CreateEntity(const std::string& name, Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		auto ent = MakeEmptyEntity();
		T::CreateEntity(*this, ent, parent, std::forward<Args>(args)...);
		SetComponent<LabelComponent>(ent, name);
		return ent;
	}


	template<typename T = EntityType, typename ... Args>
	auto CreateEntityFromEmpty(Entity ent, Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		T::CreateEntity(*this, ent, parent, std::forward<Args>(args)...);
		SetComponent<InitializationComponent>(ent);
		return ent;
	}

	template<typename T = EntityType, typename ... Args>
	auto CreateEntityFromEmpty(const std::string& name, Entity ent, Entity parent = Entity(), Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		T::CreateEntity(*this, ent, parent, std::forward<Args>(args)...);
		SetComponent<InitializationComponent>(ent);
		SetComponent<LabelComponent>(ent, name);
		return ent;
	}

	SceneGraph* GetSceneGraph() {
		return &m_SceneGraph;
	}

	void RemoveEntity(Entity entity, RemoveEntityAction action = RemoveEntityAction::REMOVE);

	void ReloadPrefabs(const std::string& prefab_path);

	template<typename T, typename ... Args>
	void SetComponent(Entity entity, Args&& ... args) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		m_ECS.emplace_or_replace<T>((entt::entity)entity.id, std::forward<Args>(args)...);
	}

	template<typename T>
	std::mutex& GetPoolSync() {
		return SyncPool<T>();
	}

	template<typename T>
	bool HasComponentSynced(Entity ent) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		return m_ECS.any_of<T>((entt::entity)ent.id);
	}

	template<typename T>
	bool HasComponent(Entity ent) {
		return m_ECS.any_of<T>((entt::entity)ent.id);
	}

	template<typename T>
	T& GetComponent(Entity entity) {
		return m_ECS.get<T>((entt::entity)entity.id);
	}

	template<typename T>
	T& GetComponentSync(Entity entity) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		return m_ECS.get<T>((entt::entity)entity.id);
	}

	template<typename T>
	void RemoveComponent(Entity entity) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		m_ECS.remove<T>((entt::entity)entity.id);
	}

	
	entt::registry& GetRegistry() {
		return m_ECS;
	}

	const SceneProxy& GetCurrentSceneProxy() const {
		return *current_scene;
	}

	void LoadSceneFromFile(const std::string& file_path = "Empty");

	void LoadEmptyScene();

	void SaveScene(const std::string& file_path);

	void SetPrimaryEntity(Entity entity);

	bool IsSceneLoaded() const {
		return load_scene.get();
	}
	
	Entity GetPrimaryEntity() const {
		return primary_entity;
	}

	Entity MakeEmptyEntity();

	bool EntityExists(Entity entity) {
		return m_ECS.valid((entt::entity)entity.id);
	}

	void MarkEntityDirty(Entity entity, bool dirty_transform = false);

	void SerializePrefab(Entity entity, const std::string& path);

	SpatialIndex& GetSpatialIndex() {
		return m_SpatialIndex;
	}

	PhysicsEngine& GetPhysicsEngine() {
		return m_PhysicsEngine;
	}

	void CheckCamera();

private:
	friend class GameLayer;

	template<typename T, auto func>
	void ConstructComponent(entt::registry& reg, entt::entity ent) {
		func(*this, Entity((uint32_t)ent));
	}

	template<typename T, auto func>
	void DestroyComponent(entt::registry& reg, entt::entity ent) {
		func(*this, Entity((uint32_t)ent));
	}

	template<typename ... Args>
	void RegisterComponents(TypeList<Args...> list) {
		(RegisterComponentType<Args>(),...);
	}
	
	void SerializePrefabChild(Entity child, std::vector<std::pair<std::string, std::string>>& file_structure);

	void LoadSceneSystem();

	struct RemoveEntityRequest {
		Entity entity;
		RemoveEntityAction action;
	};
	
	void DeletionSystem();

	void DeleteNode(SceneNode* node);

	void SetPrimaryEntitySystem();

	Entity set_primary_entity = Entity();
	Entity primary_entity = Entity();
	Entity default_camera = Entity();

	std::shared_ptr<SceneProxy> current_scene = nullptr;
	std::shared_ptr<SceneProxy> load_scene = nullptr;
	std::mutex deletion_mutex;
	std::queue<RemoveEntityRequest> deletion_queue;
	std::mutex entity_mutex;
	SceneGraph m_SceneGraph;
	PhysicsEngine m_PhysicsEngine;
	SpatialIndex m_SpatialIndex;
	entt::registry m_ECS;
};