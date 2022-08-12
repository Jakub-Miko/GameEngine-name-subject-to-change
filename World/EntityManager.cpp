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

Entity EntityManager::CreateEntity(const std::string& path, Entity parent)
{
	auto ent = Application::GetWorld().MakeEmptyEntity();
	Application::GetWorld().SetComponent<ConstructionComponent>(ent, path, parent);
	return ent;
}

Entity EntityManager::CreateEntityInplace(const std::string& path, Entity parent)
{
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

Entity EntityManager::CreateEntityInplace(Entity base_entity, const std::string& path, Entity parent)
{
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

Entity EntityManager::CreateEntity(const std::string& name, const std::string& path, Entity parent)
{
	auto ent = Application::GetWorld().MakeEmptyEntity();
	Application::GetWorld().SetComponent<ConstructionComponent>(ent, path, parent);
	Application::GetWorld().SetComponent<LabelComponent>(ent, name);
	return ent;
}

Entity EntityManager::CreateEntityInplace(const std::string& name, const std::string& path, Entity parent)
{
	auto ent = CreateEntityInplace(path, parent);
	Application::GetWorld().SetComponent<LabelComponent>(ent, name);
	return ent;
}

Entity EntityManager::CreateEntityInplace(const std::string& name, Entity base_entity, const std::string& path, Entity parent)
{
	auto ent = CreateEntityInplace(base_entity, path,parent);
	Application::GetWorld().SetComponent<LabelComponent>(ent, name);
	return ent;
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

void EntityManager::DeserializeEntityPrefab(Entity target_entity, const std::string& path, Entity parent)
{
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

	SerializeComponentToJson<TransformComponent, LabelComponent, MeshComponent, CameraComponent>(entity, json_object);
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

	DeserializeComponentToTemplate<TransformComponent, LabelComponent, MeshComponent, CameraComponent>(auxilary_registry, target_entity, json_object);
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
		
	DeserializeComponent<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent, MeshComponent, CameraComponent>(target_entity, json_object);
}


EntityManager::EntityManager() : auxilary_registry(), auxilary_registry_mutex()
{

}

EntityTemplate EntityManager::ParseEntityTemplate(const std::string& raw_string)
{
	EntityParseResult result = EntityParser::ParseEntity(raw_string);
	EntityTemplate temp;
	temp.children = result.children;
	temp.construction_script = result.construction_script;
	temp.has_inline = result.has_inline;
	temp.inline_script = result.inline_script;
	temp.properties = result.properties;
	temp.template_entity = Entity();

	if (!result.component_json.empty()) {
		entt::entity ent = auxilary_registry.create();
		DeserializeComponentsToTemplate(ent, result.component_json);
		temp.template_entity = Entity((uint32_t)ent);
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

		EntityTemplate result = ParseEntityTemplate(script_raw);

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

		EntityTemplate result = ParseEntityTemplate(script_raw);

		lock.lock();
		auto out = m_entity_cache.insert_or_assign(path, result);
		return (*(out.first)).second;
	}
}

