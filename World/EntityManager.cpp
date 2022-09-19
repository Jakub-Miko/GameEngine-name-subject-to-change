#include "EntityManager.h"
#include <algorithm>
#include <json.hpp>
#include <sstream>
#include <stdexcept>
#include <Application.h>
#include <World/Components/ConstructionComponent.h>
#include <World/World.h>
#include <fstream>
#include <FileManager.h>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>
#include <World/Components/LoadedComponent.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/Components/PrefabComponent.h>
#include <World/Components/ShadowCasterComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/SerializableComponent.h>
#include <World/Systems/ScriptSystemManagement.h>

EntityManager* EntityManager::instance = nullptr;

void EntityManager::Initialize()
{
	if (!instance) {
		instance = new EntityManager();
	}
}

EntityManager* EntityManager::Get()
{
	return instance;
}

void EntityManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

Entity EntityManager::CreateEntity(const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto ent = Application::GetWorld().MakeEmptyEntity();
	Application::GetWorld().SetComponent<ConstructionComponent>(ent, path, parent);
	return ent;
}

Entity EntityManager::CreateEntityInplace(const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();

	if (!script_vm) {
		ScriptSystemManager::Get()->InitializeScriptSystemVM();
		script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
	}
	
	
	EntityTemplate entity_template = EntityManager::Get()->GetEntitySignature(path);
	World& world = Application::GetWorld();
	Entity new_ent = world.MakeEmptyEntity();
	world.SetComponent<PrefabComponent>(new_ent, PrefabComponent(path));
	world.CreateEntityFromEmpty(new_ent, parent);
	script_vm->SetEngineInitializationEntity(new_ent, path);
	world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
	script_vm->CallInitializationFunction(path, "OnConstruct");
	world.SetComponent<InitializationComponent>(new_ent);

	for (auto child : entity_template.children) {
		EntityTemplate child_entity_template = EntityManager::Get()->GetEntitySignature(child);
		Entity child_ent = world.CreateEntity(new_ent);
		script_vm->SetEngineInitializationEntity(child_ent, child);
		world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
		script_vm->CallInitializationFunction(child, "OnConstruct");
		world.SetComponent<InitializationComponent>(child_ent);
	}

	return new_ent;
}

Entity EntityManager::CreateEntityInplace(Entity base_entity, const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();

	if (!script_vm) {
		ScriptSystemManager::Get()->InitializeScriptSystemVM();
		script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
	}


	EntityTemplate entity_template = EntityManager::Get()->GetEntitySignature(path);
	World& world = Application::GetWorld();
	Entity new_ent = base_entity;
	world.SetComponent<PrefabComponent>(new_ent, PrefabComponent(path));
	world.CreateEntityFromEmpty(new_ent, parent);
	script_vm->SetEngineInitializationEntity(new_ent, path);
	world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
	script_vm->CallInitializationFunction(path, "OnConstruct");
	world.SetComponent<InitializationComponent>(new_ent);

	for (auto child : entity_template.children) {
		EntityTemplate child_entity_template = EntityManager::Get()->GetEntitySignature(child);
		Entity child_ent = world.CreateEntity(new_ent);
		script_vm->SetEngineInitializationEntity(child_ent, child);
		world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
		script_vm->CallInitializationFunction(child, "OnConstruct");
		world.SetComponent<InitializationComponent>(child_ent);
	}

	return new_ent;
}

Entity EntityManager::CreateEntity(const std::string& name, const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto ent = Application::GetWorld().MakeEmptyEntity();
	Application::GetWorld().SetComponent<ConstructionComponent>(ent, path, parent);
	Application::GetWorld().SetComponent<LabelComponent>(ent, name);
	return ent;
}

void EntityManager::AddConstructionScriptToPrefab(const std::string& prefab_name, const std::string& construction_script)
{
	auto entity_file = FileManager::Get()->OpenFileRaw(prefab_name);
	auto root_section = FileManager::Get()->GetFileSectionFromString(entity_file, "Root");
	auto construct_start = root_section.find("@Entity:Construction_Script");
	if (construct_start != root_section.npos) {
		construct_start += strlen("@Entity:Construction_Script");
		auto construct_end = root_section.find("@Entity",construct_start);
		root_section = root_section.replace(construct_start, construct_end - construct_start, "\n" + construction_script);
	}
	else {
		root_section.append("@Entity:Construction_Script\n" + construction_script);
	}
	FileManager::Get()->InsertOrReplaceSection(entity_file, root_section, "Root");
	std::ofstream out_stream(FileManager::Get()->GetPath(prefab_name));
	if (!out_stream.is_open()) throw std::runtime_error("File " + prefab_name + " could not be opened");
	out_stream << entity_file;
	out_stream.close();

	ScriptSystemManager::Get()->InvalidateConstructionScript(prefab_name);

	Application::GetWorld().ReloadPrefabs(prefab_name);
}

void EntityManager::RemoveConstructionScriptToPrefab(const std::string& prefab_name)
{
	auto entity_file = FileManager::Get()->OpenFileRaw(prefab_name);
	auto root_section = FileManager::Get()->GetFileSectionFromString(entity_file, "Root");
	auto construct_start = root_section.find("@Entity:Construction_Script");
	if (construct_start != root_section.npos) {
		construct_start += strlen("@Entity:Construction_Script");
		auto construct_end = root_section.find("@Entity", construct_start);
		construct_start -= strlen("@Entity:Construction_Script");
		root_section = root_section.erase(construct_start, construct_end);
	
		FileManager::Get()->InsertOrReplaceSection(entity_file, root_section, "Root");
		std::ofstream out_stream(FileManager::Get()->GetPath(prefab_name));
		if (!out_stream.is_open()) throw std::runtime_error("File " + prefab_name + " could not be opened");
		out_stream << entity_file;
		out_stream.close();

		Application::GetWorld().ReloadPrefabs(prefab_name);
	}
}

void EntityManager::AddInlineScriptToPrefab(const std::string& prefab_name, const std::string& inline_script)
{
	auto entity_file = FileManager::Get()->OpenFileRaw(prefab_name);
	auto root_section = FileManager::Get()->GetFileSectionFromString(entity_file, "Root");
	auto inline_start = root_section.find("@Entity:Inline_Script");
	if (inline_start != root_section.npos) {
		inline_start += strlen("@Entity:Inline_Script");
		auto inline_end = root_section.find("@Entity", inline_start);
		root_section = root_section.replace(inline_start, inline_end - inline_start, "\n" + inline_script);
	}
	else {
		root_section.append("@Entity:Inline_Script\n" + inline_script);
	}
	FileManager::Get()->InsertOrReplaceSection(entity_file, root_section, "Root");
	std::ofstream out_stream(FileManager::Get()->GetPath(prefab_name));
	if (!out_stream.is_open()) throw std::runtime_error("File " + prefab_name + " could not be opened");
	out_stream << entity_file;
	out_stream.flush();
	out_stream.close();

	ScriptSystemManager::Get()->InvalidateInlineScript(prefab_name);

	Application::GetWorld().ReloadPrefabs(prefab_name);
}

void EntityManager::RemoveInlineScriptToPrefab(const std::string& prefab_name)
{
	auto entity_file = FileManager::Get()->OpenFileRaw(prefab_name);
	auto root_section = FileManager::Get()->GetFileSectionFromString(entity_file, "Root");
	auto inline_start = root_section.find("@Entity:Inline_Script");
	if (inline_start != root_section.npos) {
		inline_start += strlen("@Entity:Inline_Script");
		auto inline_end = root_section.find("@Entity", inline_start);
		inline_start -= strlen("@Entity:Inline_Script");
		root_section = root_section.erase(inline_start, inline_end);
		FileManager::Get()->InsertOrReplaceSection(entity_file, root_section, "Root");
		std::ofstream out_stream(FileManager::Get()->GetPath(prefab_name));
		if (!out_stream.is_open()) throw std::runtime_error("File " + prefab_name + " could not be opened");
		out_stream << entity_file;
		out_stream.close();

		Application::GetWorld().ReloadPrefabs(prefab_name);
	}
}

Entity EntityManager::CreateEntityInplace(const std::string& name, const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto ent = CreateEntityInplace(path, parent);
	Application::GetWorld().SetComponent<LabelComponent>(ent, name);
	return ent;
}

Entity EntityManager::CreateEntityInplace(const std::string& name, Entity base_entity, const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto ent = CreateEntityInplace(base_entity, path,parent);
	Application::GetWorld().SetComponent<LabelComponent>(ent, name);
	return ent;
}

void EntityManager::ClearPrefabCacheEntry(const std::string& name)
{
	std::unique_lock<std::mutex> lock(auxilary_registry_mutex);
	std::unique_lock<std::mutex> lock2(sync_mutex);
	auto fnd = m_entity_cache.find(name);
	if (fnd == m_entity_cache.end()) return;
	auxilary_registry.destroy((entt::entity)fnd->second.template_entity.id);
	m_entity_cache.erase(name);
}

void EntityManager::InitializeFromTemplate(Entity target_entity, Entity template_entity, const std::vector<std::string>& exclude_ids)
{
	entt::entity tg_ent = (entt::entity)(uint32_t)target_entity.id;
	entt::entity tmp_ent = (entt::entity)(uint32_t)template_entity.id;
	auto& reg = Application::GetWorld().GetRegistry();
	for (auto stor : auxilary_registry.storage()) {
		if (!exclude_ids.empty()) {
			if (std::find(exclude_ids.begin(), exclude_ids.end(), stor.second.type().name()) != exclude_ids.end()) {
				continue;
			}
		}
		
		if (stor.second.contains(tmp_ent) && !reg.storage(stor.first)->second.contains((entt::entity)target_entity.id)) {
			reg.storage(stor.first)->second.emplace(tg_ent, stor.second.get(tmp_ent));
		}
	}

}

void EntityManager::DeserializeEntityPrefab(Entity target_entity, const std::string& file_path, Entity parent)
{
	std::string path = FileManager::Get()->GetRelativeFilePath(FileManager::Get()->GetPath(file_path));
	auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();

	if (!script_vm) {
		ScriptSystemManager::Get()->InitializeScriptSystemVM();
		script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
	}

	std::string file_buffer = FileManager::Get()->OpenFileRaw(FileManager::Get()->GetFilePathFromSubPath(path));

	EntityTemplate entity_template = GetEntitySignatureLocal(path, file_buffer);
	Entity new_ent = target_entity;
	auto& world = Application::GetWorld();

	bool has_construction_script = !entity_template.construction_script.empty();
	bool has_script = !entity_template.inline_script.empty();

	if (entity_template.template_entity != Entity()) {
		InitializeFromTemplate(new_ent, entity_template.template_entity, {"struct TransformComponent","class LabelComponent"});
	}

	//For Deserialized entities
	if (world.HasComponentSynced<PrefabComponent>(new_ent)) {
		if (!world.HasComponentSynced<DynamicPropertiesComponent>(new_ent)) {
			world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
		}
		
		if (has_construction_script) {
			script_vm->SetEngineInitializationEntity(new_ent, path);
			script_vm->CallInitializationFunction(path, "OnConstruct");
		}
		if (has_script) {
			world.SetComponent<InitializationComponent>(new_ent);
		}
	}
	else { // for spawned entities
		world.SetComponent<PrefabComponent>(new_ent, PrefabComponent(path));
		world.CreateEntityFromEmpty(new_ent, parent);
		world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
		if (has_construction_script) {
			script_vm->SetEngineInitializationEntity(new_ent, path);
			script_vm->CallInitializationFunction(path, "OnConstruct");
		}
		if (has_script) {
			world.SetComponent<InitializationComponent>(new_ent);
		}
	}


	for (auto child : entity_template.children) {
		
		DeserializeEntityPrefab_impl(child, path,file_buffer, new_ent);
	}
}

void EntityManager::DeserializeEntityPrefab_impl(const std::string& path_in, const std::string& original_path, const std::string& local_file_buffer, Entity parent)
{
	auto path = path_in;
	auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();

	if (!script_vm) {
		ScriptSystemManager::Get()->InitializeScriptSystemVM();
		script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
	}

	auto& world = Application::GetWorld();

	EntityTemplate child_entity_template;
	if (FileManager::Get()->IsSubPath(path) && FileManager::Get()->GetFilePathFromSubPath(path) == "local") {
		child_entity_template = GetEntitySignatureLocal(path, local_file_buffer);
		path = FileManager::Get()->GetFilePathFromSubPath(original_path) + "#" + FileManager::Get()->GetFileSectionNameFromSubPath(path);
	} else {
		path = FileManager::Get()->GetPath(path);
		child_entity_template = GetEntitySignature(path);
	}

	bool has_construction_script = !child_entity_template.construction_script.empty();
	bool has_script = !child_entity_template.inline_script.empty();

	Entity child_ent = world.CreateEntity<PrefabChildEntityType>(parent);
	world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
	if (child_entity_template.template_entity != Entity()) {
		InitializeFromTemplate(child_ent, child_entity_template.template_entity);
	}

	if (!world.HasComponentSynced<TransformComponent>(child_ent)) {
		world.SetComponent<TransformComponent>(child_ent);
	}

	if (has_construction_script) {
		script_vm->SetEngineInitializationEntity(child_ent, path);
		script_vm->CallInitializationFunction(path, "OnConstruct");
	}
	if (has_script) {
		world.SetComponent<InitializationComponent>(child_ent);
	}


	for (auto child : child_entity_template.children) {

		DeserializeEntityPrefab_impl(child, original_path, local_file_buffer, child_ent);
	}

}

template<typename T, typename ... Args>
static void SerializeComponentToJson(Entity target_entity, nlohmann::json& json_object) {
	if (Application::GetWorld().HasComponent<T>(target_entity))
	{
		json_object[std::string(RuntimeTag<T>::GetName())] = Application::GetWorld().GetComponent<T>(target_entity);
	}
	if constexpr (sizeof...(Args) != 0) {
		SerializeComponentToJson<Args...>(target_entity, json_object);
	}
}

std::string EntityManager::SerializeComponentsToJson(Entity entity)
{
	nlohmann::json json_object;

	SerializeComponentToJson<TransformComponent, LabelComponent, MeshComponent, CameraComponent, LightComponent, ShadowCasterComponent>(entity, json_object);
	return json_object.dump();
}

template<typename T, typename ... Args>
static void DeserializeComponentToTemplate(entt::registry& reg, Entity target_entity, const nlohmann::json& json_object) {
	if (json_object.contains(std::string(RuntimeTag<T>::GetName()))) {
		T component_object = json_object[std::string(RuntimeTag<T>::GetName())].get<T>();
		reg.emplace<T>((entt::entity)target_entity.id, component_object);
	}
	if constexpr (sizeof...(Args) != 0) {
		DeserializeComponentToTemplate<Args...>(reg,target_entity, json_object);
	}
}

void EntityManager::DeserializeComponentsToTemplate(Entity target_entity, const std::string& json_string)
{
	nlohmann::json json_object = nlohmann::json::parse(json_string);

	DeserializeComponentToTemplate<TransformComponent, LabelComponent, MeshComponent, CameraComponent, LightComponent, ShadowCasterComponent>(auxilary_registry, target_entity, json_object);
}


template<typename T, typename ... Args>
static void DeserializeComponent(Entity target_entity, const nlohmann::json& json_object) {
	if (json_object.contains(std::string(RuntimeTag<T>::GetName()))) {
		T component_object = json_object[std::string( RuntimeTag<T>::GetName())].get<T>();
		Application::GetWorld().SetComponent<T>(target_entity, component_object);
	}
	if constexpr (sizeof...(Args) != 0) {
		DeserializeComponent<Args...>(target_entity, json_object);
	}
}


void EntityManager::DeserializeComponents(Entity target_entity, const std::string& json_string)
{
	std::lock_guard<std::mutex> lock(auxilary_registry_mutex);
	nlohmann::json json_object = nlohmann::json::parse(json_string);
		
	DeserializeComponent<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent, MeshComponent, CameraComponent, LightComponent, ShadowCasterComponent, ScriptComponent>(target_entity, json_object);
}


EntityManager::EntityManager() : auxilary_registry(), auxilary_registry_mutex()
{

}

EntityTemplate EntityManager::ParseEntityTemplate(const std::string& raw_string, const std::string& path)
{
	EntityParseResult result = EntityParser::ParseEntity(raw_string);
	EntityTemplate temp;
	temp.children = result.children;
	temp.construction_script = result.construction_script;
	temp.has_inline = result.has_inline;
	temp.inline_script = result.inline_script;
	temp.properties = result.properties;
	temp.template_entity = Entity();
	entt::entity ent;
	if (!result.component_json.empty()) {
		ent = auxilary_registry.create();
		DeserializeComponentsToTemplate(ent, result.component_json);
		temp.template_entity = Entity((uint32_t)ent);
	}
	if (temp.has_inline) {
		auxilary_registry.emplace<ScriptComponent>(ent, ScriptComponent(path));
	}
	return temp;
}

const EntityTemplate& EntityManager::GetEntitySignature(const std::string& path) 
{
	std::unique_lock<std::mutex> lock(sync_mutex);
	auto ent = m_entity_cache.find(path);
	if (ent != m_entity_cache.end()) {
		return (*ent).second;
	}
	else {
		lock.unlock();
		std::string script_raw = FileManager::Get()->OpenFile(path);

		EntityTemplate result = ParseEntityTemplate(script_raw, path);

		lock.lock();
		auto out = m_entity_cache.insert_or_assign(path, result);
		return (*(out.first)).second;
	}
}
void EntityManager::ClearPrefabCache()
{
	std::lock_guard<std::mutex> lock1(auxilary_registry_mutex);
	std::lock_guard<std::mutex> lock2(sync_mutex);
	auxilary_registry.clear();
	m_entity_cache.clear();
}
const EntityTemplate& EntityManager::GetEntitySignatureLocal(const std::string& path, const std::string& file_buffer)
{
	std::unique_lock<std::mutex> lock(sync_mutex);
	auto ent = m_entity_cache.find(path);
	if (ent != m_entity_cache.end()) {
		return (*ent).second;
	}
	else {
		lock.unlock();
		std::string script_raw;
		if (FileManager::Get()->IsSubPath(path)) {
			script_raw = FileManager::Get()->GetFileSectionFromString(file_buffer, FileManager::Get()->GetFileSectionNameFromSubPath(path));
		}
		else {
			script_raw = FileManager::Get()->GetFileSectionFromString(file_buffer, "Root");
		}

		EntityTemplate result = ParseEntityTemplate(script_raw, path);

		lock.lock();
		auto out = m_entity_cache.insert_or_assign(path, result);
		return (*(out.first)).second;
	}
}

