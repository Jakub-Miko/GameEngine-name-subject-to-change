#include "World.h"
#include <FileManager.h>
#include <Core/Extensions/EntitySerializationECSAdapter.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/TransformComponent.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/LabelComponent.h>
#include <World/Components/DefferedUpdateComponent.h>
#include <World/Components/InitializationComponent.h>
#include <GameStateMachine.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/ScriptComponent.h>
#include <World/Components/SquareComponent.h>
#ifdef EDITOR
#include <Editor/Editor.h>
#endif
#include <fstream>

World::World() : m_ECS(), m_SceneGraph(this), load_scene(std::make_shared<SceneProxy>())
{
	RegistryWarmUp();
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
	m_SceneGraph.MarkEntityDirty(m_SceneGraph.GetSceneGraphNode(ent));
}

void World::SetEntityTranslationSync(Entity ent, const glm::vec3& translation)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityTranslation(ent, translation);
}

void World::SetEntityRotation(Entity ent, const glm::quat& rotation)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).rotation = rotation;
	m_SceneGraph.MarkEntityDirty(m_SceneGraph.GetSceneGraphNode(ent));
}

void World::SetEntityRotationSync(Entity ent, const glm::quat& rotation)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityRotation(ent, rotation);
}

void World::SetEntityScale(Entity ent, const glm::vec3& scale)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).size = scale;
	m_SceneGraph.MarkEntityDirty(m_SceneGraph.GetSceneGraphNode(ent));
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

void World::RegistryWarmUp()
{
	m_ECS.storage<BoundingVolumeComponent>();
	m_ECS.storage<CameraComponent>();
	m_ECS.storage<ConstructionComponent>();
	m_ECS.storage<DefferedUpdateComponent>();
	m_ECS.storage<DynamicPropertiesComponent>();
	m_ECS.storage<InitializationComponent>();
	m_ECS.storage<KeyPressedScriptComponent>();
	m_ECS.storage<LoadedComponent>();
	m_ECS.storage<MousePressedScriptComponent>();
	m_ECS.storage<ScriptComponent>();
	m_ECS.storage<SerializableComponent>();
	m_ECS.storage<SquareComponent>();
	m_ECS.storage<MeshComponent>();
	m_ECS.storage<TransformComponent>();
}

void World::LoadSceneSystem()
{
	if (load_scene) {
		if (GameStateMachine::Get()->current_state) {
			GameStateMachine::Get()->ScriptOnDeattach();
		}
#ifdef EDITOR
		Editor::Get()->Reset();
#endif
		primary_entity = Entity();
		m_ECS.clear();
		m_ECS = entt::registry();
		m_SceneGraph.clear();

		std::ifstream file(load_scene->scene_path);
		if (!file.is_open()) {
			throw std::runtime_error("File could not be opened: " + load_scene->scene_path);
		}
		nlohmann::json json;
		file >> json;
		file.close();

		RegistryWarmUp();

		ECS_Input_Archive archive(json["Entities"]);
		entt::snapshot_loader(m_ECS).component<TransformComponent, LoadedComponent, DynamicPropertiesComponent, LabelComponent,MeshComponent, CameraComponent>(archive);


		m_SceneGraph.Deserialize(json);

		SetPrimaryEntity(load_scene->primary_entity);

		primary_entity = load_scene->primary_entity;
		current_scene = load_scene;
		if (load_scene->primary_entity == Entity()) {
			auto ent = CreateEntity();
			SetComponent<CameraComponent>(ent);
			SetComponent<LabelComponent>(ent,"Default Camera");
			primary_entity = ent;
		}
		load_scene = nullptr;

		if (GameStateMachine::Get()->current_state) {
			GameStateMachine::Get()->ScriptOnAttach();
		}
	}
}

void World::SetPrimaryEntitySystem()
{
	if (set_primary_entity != Entity()) {
		if (!HasComponent<CameraComponent>(set_primary_entity)) {
			throw std::runtime_error("Primary entity doesn't have a camera component");
		}
		primary_entity = set_primary_entity;
		set_primary_entity = Entity();
	}
}

void World::LoadSceneFromFile(const std::string& file_path)
{
	load_scene = std::make_shared<SceneProxy>(file_path);
}

void World::LoadEmptyScene()
{
	load_scene = std::make_shared<SceneProxy>("engine_asset:EmptyScene.json"_path);
}

void World::SaveScene(const std::string& file_path)
{
	ECS_Output_Archive archive;
	entt::snapshot snapshot(m_ECS);
	auto view_serializable = m_ECS.view<SerializableComponent>();
	auto view_serializable_non_prefabs = m_ECS.view<SerializableComponent>(entt::exclude<LoadedComponent>);
	snapshot.component<TransformComponent, LoadedComponent, DynamicPropertiesComponent, LabelComponent,MeshComponent, CameraComponent>(archive, view_serializable.begin(), view_serializable.end());
	snapshot.component<MeshComponent, CameraComponent>(archive, view_serializable_non_prefabs.begin(), view_serializable_non_prefabs.end());


	nlohmann::json json;
	json["Entities"] = archive.AsJson();

	m_SceneGraph.Serialize(json);

	if (primary_entity != Entity() && HasComponent<SerializableComponent>(primary_entity)) {
		json["primary_entity"] = primary_entity;
	}

	std::ofstream file(file_path,std::ios_base::trunc);
	if (!file.is_open()) {
		throw std::runtime_error("File could not be opened: " + file_path);
	}
	file << json;
	file.close();

}

void World::SetPrimaryEntity(Entity entity)
{
	set_primary_entity = entity;
}

Entity World::MakeEmptyEntity()
{
	std::lock_guard<std::mutex> lock(entity_mutex);
	return Entity((uint32_t)m_ECS.create());
}
