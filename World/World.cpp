#include "World.h"

World::World() : m_ECS()
{

}

World::~World()
{

}

void World::RemoveEntity(Entity entity)
{
	std::lock_guard<std::mutex> lock(entity_mutex);
	m_ECS.destroy((entt::entity)entity.id);
}

Entity World::MakeEmptyEntity()
{
	std::lock_guard<std::mutex> lock(entity_mutex);
	return Entity((uint32_t)m_ECS.create());
}
