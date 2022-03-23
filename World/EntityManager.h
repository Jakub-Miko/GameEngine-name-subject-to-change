#pragma once
#include <unordered_map>
#include <World/EntityParser.h>
#include <mutex>
#include <string>
#include <vector>
#include <deque>
#include <World/Entity.h>


struct Construction_Entry {
    Construction_Entry(Entity id, const std::string& path, Entity parent) : id(id), path(path), parent(parent) {}
    Entity id;
    Entity parent;
    std::string path;
};

class EntityManager {
public:
    EntityManager(const EntityManager& ref) = delete;
    EntityManager(EntityManager&& ref) = delete;
    EntityManager& operator=(const EntityManager& ref) = delete;
    EntityManager& operator=(EntityManager&& ref) = delete;

    static void Initialize();
    static EntityManager* Get();
    static void Shutdown();

    Entity CreateEntity(const std::string& path, Entity parent = Entity());

    Entity CreateEntityInplace(const std::string& path, Entity parent = Entity());

    Entity CreateEntityInplace(Entity base_entity, const std::string& path, Entity parent = Entity());

    void ClearConstructionQueue();

    std::deque<Construction_Entry>& GetQueue();

    const EntityParseResult& GetEntitySignature(const std::string& path);

private:
    EntityManager();
    static EntityManager* instance;

    std::mutex sync_mutex;
    std::unordered_map<std::string, EntityParseResult> m_entity_cache;

    std::mutex constuction_mutex;
    std::deque<Construction_Entry> construction_queue;
};