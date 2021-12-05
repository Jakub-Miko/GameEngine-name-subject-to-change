#pragma once 
#include <entt/entt.hpp>
#include <World/EntityTypes.h>
#include <World/Components/InitializationComponent.h>
#include <World/Entity.h>
#include <mutex>
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

	template<typename T = EntityType, typename ... Args>
	auto CreateEntity(Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(),std::declval<Entity>(),std::declval<Args>()...), Entity())
	{
		auto ent = MakeEmptyEntity();
		T::CreateEntity(*this,ent,std::forward<Args>(args)...);
		SetComponent<InitializationComponent>(ent);
		return ent;
	}


	template<typename T = EntityType, typename ... Args>
	auto CreateEntityFromEmpty(Entity ent, Args&& ... args) -> decltype(T::CreateEntity(std::declval<World&>(), std::declval<Entity>(), std::declval<Args>()...), Entity())
	{
		T::CreateEntity(*this, ent, std::forward<Args>(args)...);
		SetComponent<InitializationComponent>(ent);
		return ent;
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

	Entity MakeEmptyEntity();

private:

	std::mutex entity_mutex;

	entt::registry m_ECS;
};