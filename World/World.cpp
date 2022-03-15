#include "World.h"
#include <FileManager.h>

World::World() : m_ECS(), m_SceneGraph(this), load_scene(std::make_shared<SceneProxy>())
{
	if((uint32_t)(m_ECS.create())!=0) throw std::runtime_error("A null Entity could not be reserved");
}

World::~World()
{

}

void World::UpdateTransformMatricies()
{
	m_SceneGraph.CalculateMatricies();
}

void World::SetEntityTranslation(Entity ent, const glm::vec3& translation)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).translation = translation;
	m_SceneGraph.MarkEntityDirty(m_ECS.get<TransformComponent>((entt::entity)ent.id).scene_node);
}

void World::SetEntityTranslationSync(Entity ent, const glm::vec3& translation)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityTranslation(ent, translation);
}

void World::SetEntityRotation(Entity ent, const glm::quat& rotation)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).rotation = rotation;
	m_SceneGraph.MarkEntityDirty(m_ECS.get<TransformComponent>((entt::entity)ent.id).scene_node);
}

void World::SetEntityRotationSync(Entity ent, const glm::quat& rotation)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityRotation(ent, rotation);
}

void World::SetEntityScale(Entity ent, const glm::vec3& scale)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).size = scale;
	m_SceneGraph.MarkEntityDirty(m_ECS.get<TransformComponent>((entt::entity)ent.id).scene_node);
}

void World::SetEntityScaleSync(Entity ent, const glm::vec3& scale)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityScale(ent, scale);
}

void World::RemoveEntity(Entity entity)
{
	std::lock_guard<std::mutex> lock(entity_mutex);
	m_ECS.destroy((entt::entity)entity.id);
}

void World::LoadSceneSystem()
{
	if (load_scene) {
		m_ECS.clear();
		m_ECS = entt::registry();
		m_SceneGraph.clear();
		if ((uint32_t)(m_ECS.create()) != 0) throw std::runtime_error("A null Entity could not be reserved");

		m_SceneGraph.Deserialize(FileManager::Get()->GetAssetFilePath(load_scene->scene_path));
		current_scene = load_scene;
		load_scene = nullptr;
	}
}

void World::LoadSceneFromFile(const std::string& file_path)
{
	load_scene = std::make_shared<SceneProxy>(file_path);
}

Entity World::MakeEmptyEntity()
{
	std::lock_guard<std::mutex> lock(entity_mutex);
	return Entity((uint32_t)m_ECS.create());
}
