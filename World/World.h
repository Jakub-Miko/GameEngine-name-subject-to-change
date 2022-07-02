#pragma once 
#include <entt/entt.hpp>
#include <World/EntityTypes.h>
#include <World/SceneGraph.h>
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

	static void OnCreate(World& world, Entity entity) {

	}

};


class World {
	template<typename T>
	std::mutex& SyncPool() {
		static std::mutex pool_lock;
		return pool_lock;
	}

public:

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

	//Should only be used when no other threads are currently accesing the components
	void RemoveEntity(Entity entity);

	template<typename T, typename ... Args>
	void SetComponent(Entity entity, Args&& ... args) {
		std::lock_guard<std::mutex> lock(SyncPool<T>());
		m_ECS.emplace_or_replace<T>((entt::entity)entity.id, std::forward<Args>(args)...);
		ComponentInitProxy<T>::OnCreate(*this,entity);
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
		std::lock_guard<std::mutex> lock(SyncPool<T>());
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

private:
	friend class GameLayer;

	void RegistryWarmUp();

	void LoadSceneSystem();

	void SetPrimaryEntitySystem();

	Entity set_primary_entity = Entity();
	Entity primary_entity = Entity();

	std::shared_ptr<SceneProxy> current_scene = nullptr;
	std::shared_ptr<SceneProxy> load_scene = nullptr;
	std::mutex entity_mutex;
	SceneGraph m_SceneGraph;
	entt::registry m_ECS;
};