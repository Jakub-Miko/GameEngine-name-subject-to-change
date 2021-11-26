#include "World.h"

World::World() : m_ECS()
{

}

World::~World()
{

}

void World::RemoveEntity(Entity entity)
{
	m_ECS.destroy((entt::entity)entity.id);
}

Entity World::MakeEntity()
{
	return Entity((uint32_t)m_ECS.create());
}
