#include "World.h"

World::World() : m_ECS()
{

}

World::~World()
{

}

Entity World::CreateEntity()
{
	return Entity((uint32_t)m_ECS.create());
}

void World::RemoveEntity(Entity entity)
{
	m_ECS.destroy((entt::entity)entity.id);
}
