#include "World.h"
#include <FileManager.h>
#include <Core/Extensions/EntitySerializationECSAdapter.h>
#include <World/Components/SerializableComponent.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/DynamicPropertiesComponent.h>
#include <World/Components/TransformComponent.h>
#include <Renderer/TextureManager.h>
#include <World/Components/BoundingVolumeComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/ConstructionComponent.h>
#include <World/Components/LabelComponent.h>
#include <World/Components/DefferedUpdateComponent.h>
#include <World/Components/InitializationComponent.h>
#include <World/Components/PrefabComponent.h>
#include <GameStateMachine.h>
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/Components/MousePressedScriptComponent.h>
#include <World/Components/ScriptComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/SquareComponent.h>
#include <World/EntityManager.h>
#ifdef EDITOR
#include <Editor/Editor.h>
#endif
#include <fstream>

void World::Init()
{
	m_SpatialIndex.Init(SpatialIndexProperties());
}

World::World() : m_ECS(), m_SceneGraph(this), load_scene(std::make_shared<SceneProxy>()), deletion_queue(), deletion_mutex(), m_SpatialIndex()
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
	MarkEntityDirty(ent);
}

void World::SetEntityTranslationSync(Entity ent, const glm::vec3& translation)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityTranslation(ent, translation);
}

void World::SetEntityRotation(Entity ent, const glm::quat& rotation)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).rotation = rotation;
	MarkEntityDirty(ent);
}

void World::SetEntityRotationSync(Entity ent, const glm::quat& rotation)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityRotation(ent, rotation);
}

void World::SetEntityScale(Entity ent, const glm::vec3& scale)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).size = scale;
	MarkEntityDirty(ent); 
}

void World::SetEntityScaleSync(Entity ent, const glm::vec3& scale)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityScale(ent, scale);
}

void World::RemoveEntity(Entity entity, RemoveEntityAction action)
{
	std::lock_guard<std::mutex> lock(deletion_mutex);
	deletion_queue.push(RemoveEntityRequest{entity, action});
}

void World::MarkEntityDirty(Entity entity)
{
	SceneNode* node = m_SceneGraph.GetSceneGraphNode(entity);
	m_SceneGraph.MarkEntityDirty(node);

}

static std::string GetPrefabSectionName(Entity ent) {
	std::string name;
	if (Application::GetWorld().HasComponent<LabelComponent>(ent)) {
		name = Application::GetWorld().GetComponent<LabelComponent>(ent).label + "_" + std::to_string(ent.id);
	}
	else {
		name = std::to_string(ent.id);
	}
	auto fnd = name.find(' ');
	while (fnd != name.npos) {
		name = name.replace(fnd, 1, "_");
		fnd = name.find(' ', fnd);
	}

	return name;
}

void World::SerializePrefab(Entity entity, const std::string& path)
{
	if (!HasComponent<PrefabComponent>(entity)) {
		throw std::runtime_error("Entity must contain Prefab Component");
	}
	
	std::vector<std::pair<std::string, std::string>> file_structure;

	EntityParseResult result;

	PrefabComponent& prefab = GetComponent<PrefabComponent>(entity);

	result.component_json = EntityManager::Get()->SerializeComponentsToJson(entity);
	if (HasComponent<DynamicPropertiesComponent>(entity)) {
		result.properties = GetComponent<DynamicPropertiesComponent>(entity);
	}
	SceneNode* node = GetSceneGraph()->GetSceneGraphNode(prefab.first_child);
	while (node) {
		result.children.push_back("local#" + GetPrefabSectionName(node->entity));
		SerializePrefabChild(node->entity, file_structure);
		node = node->next;
	}
	std::stringstream stream;
	stream << "@Entity \n";
	stream << "{\n";
	stream << "\"Components\": " << result.component_json << ",\n";
	nlohmann::json children_array = nlohmann::json::array();
	for (auto& child : result.children)
	{
		children_array.push_back(child);
	}
	stream << "\"Children\": " << children_array.dump() << "\n";
	stream << "}\n";

	file_structure.push_back(std::make_pair("Root", stream.str()));

	std::stringstream final_stream;
	for (auto pair : file_structure) {
		final_stream << "@Section:" << pair.first << "\n";
		final_stream << pair.second << "\n";
		final_stream << "@EndSection\n";
	}
	std::ofstream file_stream(path);
	if (!file_stream.is_open()) {
		throw std::runtime_error("Could not open file " + path);
	}

	file_stream << final_stream.str();

	file_stream.close();
}

void World::CheckCamera()
{
	if (!m_ECS.valid((entt::entity)primary_entity.id)) {
		if (default_camera != Entity()) {
			set_primary_entity = default_camera;
		}
		else {
			auto ent = CreateEntity();
			SetComponent<CameraComponent>(ent);
			SetComponent<LabelComponent>(ent, "Default Camera");
			default_camera = ent;
			primary_entity = default_camera;
			set_primary_entity = Entity();
		}
	}
}

void World::SerializePrefabChild(Entity child, std::vector<std::pair<std::string, std::string>>& file_structure)
{
	EntityParseResult result;

	result.component_json = EntityManager::Get()->SerializeComponentsToJson(child);
	if (!result.properties.m_Properties.empty()) {
		result.properties = GetComponent<DynamicPropertiesComponent>(child);
	}
	SceneNode* node = GetSceneGraph()->GetSceneGraphNode(child)->first_child;
	while (node) {
		result.children.push_back("local#" + GetPrefabSectionName(node->entity));
		SerializePrefabChild(node->entity, file_structure);
		node = node->next;
	}
	std::stringstream stream;
	stream << "@Entity \n";
	stream << "{\n";
	stream << "\"Components\": " << result.component_json << ",\n";
	nlohmann::json children_array = nlohmann::json::array();
	for (auto& child : result.children)
	{
		children_array.push_back(child);
	}
	stream << "\"Children\": " << children_array.dump() << "\n";
	stream << "}\n";
	std::string name;



	file_structure.push_back(std::make_pair(GetPrefabSectionName(child), stream.str()));
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
	m_ECS.storage<MousePressedScriptComponent>();
	m_ECS.storage<ScriptComponent>();
	m_ECS.storage<SerializableComponent>();
	m_ECS.storage<SquareComponent>();
	m_ECS.storage<PrefabComponent>();
	m_ECS.storage<MeshComponent>();
	m_ECS.storage<LightComponent>();
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
		default_camera = Entity();
		TextureManager::Get()->ClearTextureCache();
		MaterialManager::Get()->ClearMaterialCache();
		MeshManager::Get()->ClearMeshCache();
		EntityManager::Get()->ClearPrefabCache();

		std::ifstream file(load_scene->scene_path);
		if (!file.is_open()) {
			throw std::runtime_error("File could not be opened: " + load_scene->scene_path);
		}
		nlohmann::json json;
		file >> json;
		file.close();

		RegistryWarmUp();

		ECS_Input_Archive archive(json["Entities"]);
		entt::snapshot_loader(m_ECS).component<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent,MeshComponent, CameraComponent, LightComponent>(archive);


		m_SceneGraph.Deserialize(json);

		SetPrimaryEntity(load_scene->primary_entity);
		if (load_scene->primary_entity != Entity()) {
			if (!HasComponent<CameraComponent>(set_primary_entity)) {
				CheckCamera();
			}
			else {
				primary_entity = set_primary_entity;
				set_primary_entity = Entity();
			}
		}
		else {
			CheckCamera();
		}


		for (auto& light : m_ECS.view<LightComponent>()) {
			ComponentInitProxy<LightComponent>::OnCreate(*this, light);
		}

		current_scene = load_scene;
		load_scene = nullptr;

		if (GameStateMachine::Get()->current_state) {
			GameStateMachine::Get()->ScriptOnAttach();
		}
		m_SpatialIndex.Rebuild();

	}
}

void World::DeletionSystem()
{
	std::lock_guard<std::mutex> lock(deletion_mutex);
	if (deletion_queue.empty()) {
		return;
	}
	Entity ent;
	RemoveEntityAction action;
	while (!deletion_queue.empty()) {
		ent = deletion_queue.front().entity;
		action = deletion_queue.front().action;
		deletion_queue.pop();
		
		if (action == RemoveEntityAction::REMOVE) {
			SceneNode* first_node = m_SceneGraph.GetSceneGraphNode(ent)->first_child;
			while (first_node)
			{
				SceneNode* temp_node = first_node;
				first_node = first_node->next;
				DeleteNode(temp_node);
			}
			if (HasComponent<PrefabComponent>(ent)) {
#ifdef EDITOR
				Editor::Get()->ClosePrefabEditorWindow(ent);
#endif
				SceneNode* node = m_SceneGraph.GetSceneGraphNode(GetComponent<PrefabComponent>(ent).first_child);
				while (node) {
					SceneNode* temp_node = node;
					node = node->next;
					DeleteNode(temp_node);
				}
			}
			m_SceneGraph.RemoveEntity(ent);
			m_ECS.destroy((entt::entity)ent.id);
		}
		else if (action == RemoveEntityAction::RELOAD_PREFAB) {
			try {

				if (!HasComponent<PrefabComponent>(ent)) {
					throw std::runtime_error("Cannot Reload Prefab if entity is not a prefab");
				}
#ifdef EDITOR
				Editor::Get()->ClosePrefabEditorWindow(ent);
#endif
				SceneNode* node = m_SceneGraph.GetSceneGraphNode(GetComponent<PrefabComponent>(ent).first_child);
				while (node) {
					SceneNode* temp_node = node;
					node = node->next;
					DeleteNode(temp_node);
				}
			
				for (auto stor : m_ECS.storage()) {
					if (stor.second.type() == entt::type_id<TransformComponent>() || stor.second.type() == entt::type_id<LabelComponent>() || stor.second.type() == entt::type_id<PrefabComponent>() 
						|| stor.second.type() == entt::type_id<SerializableComponent>()) {
						continue;
					}
					stor.second.remove((entt::entity)ent.id);
				}
				auto& prefab = GetComponent<PrefabComponent>(ent);
				prefab.first_child = Entity();
				EntityManager::Get()->DeserializeEntityPrefab(ent, GetComponent<PrefabComponent>(ent).file_path);
			}
			catch (...) {
				if (HasComponent<PrefabComponent>(ent)) {
					auto& comp = GetComponent<PrefabComponent>(ent);
					comp.status = PrefabStatus::ERROR;
					comp.file_path = "Unknown";
					for (auto stor : m_ECS.storage()) {
						if (stor.second.type() == entt::type_id<TransformComponent>() || stor.second.type() == entt::type_id<LabelComponent>() || stor.second.type() == entt::type_id<PrefabComponent>() 
							|| stor.second.type() == entt::type_id<SerializableComponent>()) {
							continue;
						}
						stor.second.remove((entt::entity)ent.id);
					}
					auto& prefab = GetComponent<PrefabComponent>(ent);
					prefab.first_child = Entity();
				}
			}

		}
		else if (action == RemoveEntityAction::REMOVE_PREFABS) {
			if (!HasComponent<PrefabComponent>(ent)) {
				throw std::runtime_error("Cannot Reload Prefab if entity is not a prefab");
			}
#ifdef EDITOR
			Editor::Get()->ClosePrefabEditorWindow(ent);
#endif
			SceneNode* node = m_SceneGraph.GetSceneGraphNode(GetComponent<PrefabComponent>(ent).first_child);
			while (node) {
				SceneNode* temp_node = node;
				node = node->next;
				DeleteNode(temp_node);
			}
			Application::GetWorld().RemoveComponent<PrefabComponent>(ent);

		}
		CheckCamera();
	}


}

void World::DeleteNode(SceneNode* node)
{

	SceneNode* first_node = node->first_child;
	while (first_node)
	{
		SceneNode* temp_node = first_node;
		first_node = first_node->next;
		DeleteNode(temp_node);
	}
	if (HasComponent<PrefabComponent>(node->entity)) {
#ifdef EDITOR
		Editor::Get()->ClosePrefabEditorWindow(node->entity);
#endif
		SceneNode* node_iter = m_SceneGraph.GetSceneGraphNode(GetComponent<PrefabComponent>(node->entity).first_child);
		while (node_iter) {
			DeleteNode(node_iter);
			node_iter = node_iter->next;
		}
	}
	auto entity = node->entity;
	m_SceneGraph.RemoveEntity(node->entity);
	m_ECS.destroy((entt::entity)entity.id);

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
	auto view_serializable_non_prefabs = m_ECS.view<SerializableComponent>(entt::exclude<PrefabComponent>);
	snapshot.component<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent,MeshComponent, CameraComponent, LightComponent>(archive, view_serializable.begin(), view_serializable.end());
	snapshot.component<MeshComponent, CameraComponent, LightComponent>(archive, view_serializable_non_prefabs.begin(), view_serializable_non_prefabs.end());


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
