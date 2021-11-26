#pragma once 
#include <entt/entt.hpp>
#include <World/EntityTypes.h>
#include <World/Entity.h>
#include <utility>

class EntityType;

class World {
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
		auto ent = MakeEntity();
		T::CreateEntity(*this,ent,std::forward<Args>(args)...);
		return ent;
	}



	void RemoveEntity(Entity entity);

	template<typename T, typename ... Args>
	void SetComponent(Entity entity, Args&& ... args) {
		m_ECS.emplace<T>((entt::entity)entity.id, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent(Entity entity) {
		return m_ECS.get<T>((entt::entity)entity.id);
	}

	template<typename T>
	void RemoveComponent(Entity entity) {
		m_ECS.remove<T>((entt::entity)entity.id);
	}

	
	entt::registry& GetRegistry() {
		return m_ECS;
	}

private:

	Entity MakeEntity();

	entt::registry m_ECS;
};