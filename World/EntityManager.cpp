#include "EntityManager.h"
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <FileManager.h>

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

