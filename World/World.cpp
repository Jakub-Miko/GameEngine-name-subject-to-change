#include "World.h"
#include <FileManager.h>
#include <Core/Extensions/EntitySerializationECSAdapter.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/DefferedUpdateComponent.h>
#include <World/Components/InitializationComponent.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/ScriptComponent.h>
#include <World/Components/SquareComponent.h>
#include <fstream>

World::World() : m_ECS(), m_SceneGraph(this), load_scene(std::make_shared<SceneProxy>())
{
	WarmUp();
	//if((uint32_t)(m_ECS.create())!=0) throw std::runtime_error("A null Entity could not be reserved");
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

void World::WarmUp()
{
	m_ECS.prepare<BoundingVolumeComponent>();
	m_ECS.prepare<CameraComponent>();
	m_ECS.prepare<ConstructionComponent>();
	m_ECS.prepare<DefferedUpdateComponent>();
	m_ECS.prepare<DynamicPropertiesComponent>();
	m_ECS.prepare<InitializationComponent>();
	m_ECS.prepare<KeyPressedScriptComponent>();
	m_ECS.prepare<LoadedComponent>();
	m_ECS.prepare<MousePressedScriptComponent>();
	m_ECS.prepare<ScriptComponent>();
	m_ECS.prepare<SerializableComponent>();
	m_ECS.prepare<SquareComponent>();
	m_ECS.prepare<TransformComponent>();
}

void World::LoadSceneSystem()
{
	if (load_scene) {
		m_ECS.clear();
		m_ECS = entt::registry();
		m_SceneGraph.clear();

		std::ifstream file(FileManager::Get()->GetAssetFilePath(load_scene->scene_path));
		if (!file.is_open()) {
			throw std::runtime_error("File could not be opened: " + FileManager::Get()->GetAssetFilePath(load_scene->scene_path));
		}
		nlohmann::json json;
		json << file;
		file.close();

		ECS_Input_Archive archive(json["Entities"]);
		entt::snapshot_loader(m_ECS).component<TransformComponent, LoadedComponent, DynamicPropertiesComponent>(archive);

		WarmUp();

		m_SceneGraph.Deserialize(json);
		current_scene = load_scene;
		load_scene = nullptr;
	}
}

void World::LoadSceneFromFile(const std::string& file_path)
{
	load_scene = std::make_shared<SceneProxy>(file_path);
}

void World::SaveScene(const std::string& file_path)
{
	ECS_Output_Archive archive;
	entt::snapshot snapshot(m_ECS);
	auto view_serializable = m_ECS.view<SerializableComponent>();
	snapshot.component<TransformComponent, LoadedComponent, DynamicPropertiesComponent>(archive, view_serializable.begin(), view_serializable.end());

	nlohmann::json json;
	json["Entities"] = archive.AsJson();

	m_SceneGraph.Serialize(json);

	std::ofstream file(FileManager::Get()->GetAssetFilePath(file_path),std::ios_base::trunc);
	if (!file.is_open()) {
		throw std::runtime_error("File could not be opened: " + FileManager::Get()->GetAssetFilePath(file_path));
	}
	file << json;
	file.close();

}

Entity World::MakeEmptyEntity()
{
	std::lock_guard<std::mutex> lock(entity_mutex);
	return Entity((uint32_t)m_ECS.create());
}
