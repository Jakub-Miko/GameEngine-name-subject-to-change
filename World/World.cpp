#include "World.h"
#include <Events/MeshChangedEvent.h>
#include <FileManager.h>
#include <Events/KeyCodes.h>
#include <Core/Extensions/EntitySerializationECSAdapter.h>
#include <Renderer/TextureManager.h>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/Animations/AnimationManager.h>
#include <GameStateMachine.h>
#include <World/EntityManager.h>
#include <World/ComponentTypes.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/ScriptModules/DefferedPropertySetModule.h>
#include <World/ScriptModules/IOModule.h>
#include <World/ScriptModules/ApplicationDataModule.h>
#include <World/ScriptModules/TimeModule.h>
#ifdef EDITOR
#include <Editor/Editor.h>
#endif
#include <fstream>

void World::Init()
{
	m_SpatialIndex.Init(SpatialIndexProperties());
}

World::World() : m_ECS(), m_SceneGraph(this), load_scene(std::make_shared<SceneProxy>()), deletion_queue(), deletion_mutex(), m_SpatialIndex(), m_PhysicsEngine(PhysicsEngineProps()), scene_lua_engine()
{
	BindLuaFunctions();
	RegisterComponents(Component_Types());
	//if((uint32_t)(m_ECS.create())!=0) throw std::runtime_error("A null Entity could not be reserved");
}

World::~World()
{

}

void World::UpdateTransformMatricies()
{
	m_SceneGraph.CalculateMatricies();
}

void World::UpdateSceneScript(float delta_time)
{
	if (has_script) {
		scene_lua_engine.TryCall<void>(nullptr,"OnUpdate", delta_time);
	}
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

void World::SetEntityTransform(Entity ent, const glm::mat4& transform)
{
	m_ECS.get<TransformComponent>((entt::entity)ent.id).TransformMatrix = transform;
	MarkEntityDirty(ent,true);
}

void World::SetEntityTransformSync(Entity ent, const glm::mat4& transform)
{
	std::lock_guard<std::mutex> lock(SyncPool<TransformComponent>());
	SetEntityTransform(ent, transform);
}

void World::SetEntityMesh(Entity ent, const std::string mesh_path)
{
	std::lock_guard<std::mutex> lock(SyncPool<MeshComponent>());
	if (HasComponent<MeshComponent>(ent)) {
		auto& mesh = GetComponent<MeshComponent>(ent);
		mesh.ChangeMesh(mesh_path);
		MeshChangedEvent ev(ent, mesh_path);
		Application::Get()->SendObservedEvent<MeshChangedEvent>(&ev);
	}
	else {
		SetComponent<MeshComponent>(ent, MeshComponent(mesh_path));
	}
}


void World::SetEntitySkeletalMesh(Entity ent, const std::string& mesh_path, const std::string& default_animation_path)
{
	std::unique_lock<std::mutex> lock(SyncPool<SkeletalMeshComponent>());
	if (HasComponent<SkeletalMeshComponent>(ent)) {
		auto& mesh = GetComponent<SkeletalMeshComponent>(ent);
		mesh.ChangeMesh(mesh_path);
		if (!default_animation_path.empty()) {
			mesh.SetDefaultAnimationPath(FileManager::Get()->GetRelativeFilePath(default_animation_path));
		}
		MeshChangedEvent ev(ent, mesh_path);
		Application::Get()->SendObservedEvent<MeshChangedEvent>(&ev);
	}
	else {
		lock.unlock();
		SetComponent<SkeletalMeshComponent>(ent, SkeletalMeshComponent(mesh_path, FileManager::Get()->GetRelativeFilePath(default_animation_path)));
	}
}

void World::ResetEntityPrefab(Entity ent, const std::string& prefab_path)
{
	//Reloading a prefab after changing the path does what we want, but we'll need to clear its dynamic properties component otherwise we'll corrupt it.
	std::lock_guard<std::mutex> lock(SyncPool<PrefabComponent>());
	if (!HasComponent<PrefabComponent>(ent)) throw std::runtime_error("Attempting to reset entity prefab of an entity which doesn't have a prefab component");
	GetComponent<PrefabComponent>(ent).file_path = prefab_path;
	RemoveEntity(ent, RemoveEntityAction::CHANGE_PREFAB);
}

void World::UpdateSkeletalMesh(Entity ent)
{
	if (!HasComponent<SkeletalMeshComponent>(ent)) throw std::runtime_error("Can't update mesh on an entity without one");
	if (GetComponent<SkeletalMeshComponent>(ent).UpdateState()) {
		SceneNode* node = GetSceneGraph()->GetSceneGraphNode(ent);
		if (node->spatial_index_node) {
			GetSpatialIndex().RemoveEntity(node->entity);
		}
		GetSpatialIndex().AddEntity(node->entity);
	}
}

Entity World::DuplicateEntity(Entity ent, Entity parent)
{
	if (!EntityIsValid(ent)) {
		return Entity();
	}
	if (parent == Entity()) {
		parent = GetSceneGraph()->GetSceneGraphNode(ent)->parent->entity;
	}
	Entity new_ent = CreateEntity(parent);

	DuplicateComponentsRecursive(ent, new_ent, Component_Types());

	if (HasComponent<PrefabComponent>(new_ent)) {
		Application::GetWorld().RemoveEntity(new_ent, RemoveEntityAction::RELOAD_PREFAB);
	}

	SceneNode* node = m_SceneGraph.GetSceneGraphNode(ent);
	if (node && node->first_child) {
		SceneNode* node_iter = node->first_child;
		while (node_iter) {
			if (node_iter->entity != new_ent) {
				DuplicateEntity(node_iter->entity, new_ent);
			}
			node_iter = node_iter->next;
		}
	}

	return new_ent;
}

Entity World::DuplicateEntityInPrefab(Entity ent, Entity parent )
{
	if (!EntityIsValid(ent)) {
		return Entity();
	}
	if (parent == Entity()) {
		parent = GetSceneGraph()->GetSceneGraphNode(ent)->parent->entity;
	}
	Entity new_ent = CreateEntity<PrefabChildEntityType>(parent, true);

	DuplicateComponentsRecursive(ent, new_ent, Component_Types());

	if (HasComponent<PrefabComponent>(new_ent)) {
		Application::GetWorld().RemoveEntity(new_ent, RemoveEntityAction::RELOAD_PREFAB);
	}

	SceneNode* node = m_SceneGraph.GetSceneGraphNode(ent);
	if (node && node->first_child) {
		SceneNode* node_iter = node->first_child;
		while (node_iter) {
			if (node_iter->entity != new_ent) {
				DuplicateEntityInPrefab(node_iter->entity, new_ent);
			}
			node_iter = node_iter->next;
		}
	}

	return new_ent;
}



template<typename T, typename ...Args>
void World::DuplicateComponentRecursive(Entity from, Entity to)
{
	if constexpr (ComponentInitProxy<T>::can_copy) {
		if (HasComponent<T>(from)) {
			if constexpr (std::is_empty_v<T>) {
				SetComponent<T>(to);
			}
			else {
				SetComponent<T>(to, GetComponent<T>(from));
			}
		}
	}

	if constexpr (sizeof...(Args) <= 0) {
		return;
	}
	else {
		DuplicateComponentRecursive<Args...>(from, to);
	}
}



void World::UpdateMesh(Entity ent)
{
	if (!HasComponent<MeshComponent>(ent)) throw std::runtime_error("Can't update mesh on an entity without one");
	if (GetComponent<MeshComponent>(ent).UpdateState()) {
		SceneNode* node = GetSceneGraph()->GetSceneGraphNode(ent);
		if (node->spatial_index_node) {
			GetSpatialIndex().RemoveEntity(node->entity);
		}
		GetSpatialIndex().AddEntity(node->entity);
	}
}

bool World::EntityIsValid(Entity ent)
{
	return m_ECS.valid((entt::entity)ent.id);
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

void World::ReloadPrefabs(const std::string& prefab_path)
{
	EntityManager::Get()->ClearPrefabCacheEntry(prefab_path);
	for (auto ent : m_ECS.view<PrefabComponent>()) {
		Entity entity = Entity((uint32_t)ent);
		auto& prefab_comp = GetComponent<PrefabComponent>(entity);
		if (prefab_comp.GetFilePath() != prefab_path) {
			continue;
		}
		RemoveEntity(ent, RemoveEntityAction::RELOAD_PREFAB);
	}
}

void World::MarkEntityDirty(Entity entity,bool dirty_transform)
{
	SceneNode* node = m_SceneGraph.GetSceneGraphNode(entity);
	if (dirty_transform) {
		m_SceneGraph.MarkEntityDirtyTransform(node);
	}
	else {
		m_SceneGraph.MarkEntityDirty(node);
	}
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

	if (!prefab.GetFilePath().empty()) {
		EntityTemplate ent_template = EntityManager::Get()->GetEntitySignature(prefab.GetFilePath());
		result.construction_script = ent_template.construction_script;
		result.inline_script = ent_template.inline_script;
	}


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

	if (!result.construction_script.empty()) {
		stream << "@Entity:Construction_Script\n" << result.construction_script << "\n";
	}
	if (!result.inline_script.empty()) {
		stream << "@Entity:Inline_Script\n" << result.inline_script << "\n";
	}

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
		if (default_camera != Entity() && EntityExists(default_camera)) {
			set_primary_entity = Entity();
			primary_entity = default_camera;
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

void World::BindLuaFunctions()
{

	scene_lua_engine.RunString(ScriptKeyBindings);
	scene_lua_engine.InitFFI();

	ModuleBindingProperties props;

	DefferedPropertySetModule().RegisterModule(props);
	IOModule().RegisterModule(props);
	ApplicationDataModule().RegisterModule(props);
	TimeModule().RegisterModule(props);

	scene_lua_engine.RegisterModule(props);


}

void World::ResetLuaEngine()
{
	scene_lua_engine = LuaEngine();
	BindLuaFunctions();
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
		m_PhysicsEngine.clear();
		default_camera = Entity();
		
		TextureManager::Get()->ClearTextureCache();
		MaterialManager::Get()->ClearMaterialCache();
		MeshManager::Get()->ClearMeshCache();
		AnimationManager::Get()->ClearAnimationCache(); 
		EntityManager::Get()->ClearPrefabCache();
		ScriptSystemManager::Get()->ResetAllScriptSystemVMs();

		ResetLuaEngine();

		SectionList sections;
		std::string file = FileManager::Get()->OpenFileRaw(load_scene->scene_path, &sections);
		std::string script = "";
		if (sections.find("Script") != sections.end()) {
			script = FileManager::Get()->GetFileSectionFromString(file, "Script");
			has_script = true;
			scene_lua_engine.RunString(script);
		}
		else {
			has_script = false;
		}
		if (sections.size() != 0) {
			file = FileManager::Get()->GetFileSection(FileManager::Get()->GetPath(load_scene->scene_path), "Root");
		}


		nlohmann::json json = nlohmann::json::parse(file);

		Entity primary;
		if (json.find("primary_entity") != json.end()) {
			primary = json["primary_entity"].get<Entity>();
		}


		RegisterComponents(Component_Types());

		ECS_Input_Archive archive(json["Entities"]);
		entt::snapshot_loader(m_ECS).component<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent,MeshComponent, CameraComponent, LightComponent, ShadowCasterComponent, PhysicsComponent,
		SkeletalMeshComponent, AudioComponent, UITextComponent>(archive);


		m_SceneGraph.Deserialize(json);

		SetPrimaryEntity(primary);
		if (primary != Entity()) {
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

		current_scene = load_scene;
		load_scene = nullptr;

		if (GameStateMachine::Get()->current_state) {
			GameStateMachine::Get()->ScriptOnAttach();
		}
		m_SceneGraph.CalculateMatricies();
		m_SpatialIndex.Rebuild();

	}
}

void World::DeletionSystem()
{
	std::unique_lock<std::mutex> lock(deletion_mutex);
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
		else if (action == RemoveEntityAction::RELOAD_PREFAB || action == RemoveEntityAction::CHANGE_PREFAB) {
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
					//Before we refresh the prefab we need to clear its dynamic properties if the prefab changes.
					if (action == RemoveEntityAction::RELOAD_PREFAB && stor.second.type() == entt::type_id<DynamicPropertiesComponent>()) {
						continue;
					}
					stor.second.remove((entt::entity)ent.id);
				}
				auto& prefab = GetComponent<PrefabComponent>(ent);
				prefab.first_child = Entity();
				EntityManager::Get()->DeserializeEntityPrefab(ent, GetComponent<PrefabComponent>(ent).GetFilePath());
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
		else if (action == RemoveEntityAction::RELOAD_ALL_PREFABS_OF_THIS_TYPE) {
			if (!HasComponent<PrefabComponent>(ent)) throw std::runtime_error("This entity doesn't have a prefab component");
			lock.unlock();
			ReloadPrefabs(GetComponent<PrefabComponent>(ent).GetFilePath());
			lock.lock();
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
	load_scene = std::make_shared<SceneProxy>("engine_asset:EmptyScene.json");
}

void World::SaveScene(const std::string& file_path)
{
	ECS_Output_Archive archive;
	entt::snapshot snapshot(m_ECS);
	auto view_serializable = m_ECS.view<SerializableComponent>();
	auto view_serializable_non_prefabs = m_ECS.view<SerializableComponent>(entt::exclude<PrefabComponent>);
	snapshot.component<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent,MeshComponent, CameraComponent, LightComponent, ShadowCasterComponent, PhysicsComponent,
		SkeletalMeshComponent, AudioComponent, UITextComponent>(archive, view_serializable.begin(), view_serializable.end());
	//snapshot.component<MeshComponent, CameraComponent, LightComponent>(archive, view_serializable_non_prefabs.begin(), view_serializable_non_prefabs.end());


	nlohmann::json json;
	json["Entities"] = archive.AsJson();


	m_SceneGraph.Serialize(json);

	if (primary_entity != Entity() && HasComponent<SerializableComponent>(primary_entity)) {
		json["primary_entity"] = primary_entity;
	}

	std::string script = "";

	if (has_script) {
		script = FileManager::Get()->GetFileSection(FileManager::Get()->GetPath(GetCurrentSceneProxy().scene_path), "Script");
	}
	std::ofstream file(file_path,std::ios_base::trunc);
	if (!file.is_open()) {
		throw std::runtime_error("File could not be opened: " + file_path);
	}
	file << "@Section:Root\n";
	file << json;
	file << "\n@EndSection\n";
	if (!script.empty()) {
		file << "\n@Section:Script\n";
		file << script;
		file << "\n@EndSection\n";
	}


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
