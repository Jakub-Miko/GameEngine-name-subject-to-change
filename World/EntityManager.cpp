#include "EntityManager.h"
#include <sstream>
#include <stdexcept>
#include <Application.h>
#include <World/Components/ConstructionComponent.h>
#include <World/World.h>
#include <fstream>
#include <FileManager.h>
#include <World/Components/LoadedComponent.h>
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
	
	
	EntityParseResult entity_template = EntityManager::Get()->GetEntitySignature(path);
	World& world = Application::GetWorld();
	Entity new_ent = world.MakeEmptyEntity();
	world.SetComponent<LoadedComponent>(new_ent, LoadedComponent(path));
	world.CreateEntityFromEmpty(new_ent, parent);
	script_vm->SetEngineInitializationEntity(new_ent, path);
	world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
	script_vm->CallInitializationFunction(path, "OnConstruct");
	world.SetComponent<InitializationComponent>(new_ent);

	for (auto child : entity_template.children) {
		EntityParseResult child_entity_template = EntityManager::Get()->GetEntitySignature(child);
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


	EntityParseResult entity_template = EntityManager::Get()->GetEntitySignature(path);
	World& world = Application::GetWorld();
	Entity new_ent = base_entity;
	world.SetComponent<LoadedComponent>(new_ent, LoadedComponent(path));
	world.CreateEntityFromEmpty(new_ent, parent);
	script_vm->SetEngineInitializationEntity(new_ent, path);
	world.SetComponent<DynamicPropertiesComponent>(new_ent, DynamicPropertiesComponent(entity_template.properties));
	script_vm->CallInitializationFunction(path, "OnConstruct");
	world.SetComponent<InitializationComponent>(new_ent);

	for (auto child : entity_template.children) {
		EntityParseResult child_entity_template = EntityManager::Get()->GetEntitySignature(child);
		Entity child_ent = world.CreateEntity(new_ent);
		script_vm->SetEngineInitializationEntity(child_ent, child);
		world.SetComponent<DynamicPropertiesComponent>(child_ent, DynamicPropertiesComponent(child_entity_template.properties));
		script_vm->CallInitializationFunction(child, "OnConstruct");
		world.SetComponent<InitializationComponent>(child_ent);
	}

	return new_ent;
}

EntityManager::EntityManager()
{

}

const EntityParseResult& EntityManager::GetEntitySignature(const std::string& path)
{
	std::unique_lock<std::mutex> lock(sync_mutex);
	auto ent = m_entity_cache.find(path);
	if (ent != m_entity_cache.end()) {
		return (*ent).second;
	}
	else {
		lock.unlock();
		std::fstream file(FileManager::Get()->GetAssetFilePath(path));
		if (!file.is_open()) {
			throw std::runtime_error("Entity at path: " + path + "doesn't exist");
		}

		std::stringstream stream;
		stream << file.rdbuf();
		std::string script_raw = stream.str();

		EntityParseResult result = EntityParser::ParseEntity(script_raw);
		lock.lock();
		auto out = m_entity_cache.insert_or_assign(path, result);
		return (*(out.first)).second;
	}
}

