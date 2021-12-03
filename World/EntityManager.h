#pragma once
#include <unordered_map>
#include <World/EntityParser.h>
#include <mutex>
#include <string>

class EntityManager {
public:
    EntityManager(const EntityManager& ref) = delete;
    EntityManager(EntityManager&& ref) = delete;
    EntityManager& operator=(const EntityManager& ref) = delete;
    EntityManager& operator=(EntityManager&& ref) = delete;

    static void Initialize();
    static EntityManager* Get();
    static void Shutdown();

    const EntityParseResult& GetEntitySignature(const std::string& path);

private:
    EntityManager();
    static EntityManager* instance;

    std::mutex sync_mutex;
    std::unordered_map<std::string, EntityParseResult> m_entity_cache;

};