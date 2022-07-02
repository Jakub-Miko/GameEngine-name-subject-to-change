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

struct EntityTemplate {
    Entity template_entity;
    std::string construction_script = "";
    std::string inline_script = "";
    DynamicPropertiesComponent properties;
    std::vector<std::string> children;
    bool has_inline = false;
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

    Entity CreateEntity(const std::string& name, const std::string& path, Entity parent = Entity());

    Entity CreateEntityInplace(const std::string& name, const std::string& path, Entity parent = Entity());

    Entity CreateEntityInplace(const std::string& name, Entity base_entity, const std::string& path, Entity parent = Entity());

    void InitializeFromTemplate(Entity target_entity, Entity template_entity);

    void DeserializeEntityPrefab(Entity target_entity, const std::string& path, Entity parent = Entity());

    void DeserializeComponents(Entity target_entity, const std::string& json_string);

    const EntityTemplate& GetEntitySignature(const std::string& path);

private:
    void DeserializeEntityPrefab_impl( const std::string& path, Entity parent = Entity());

    void DeserializeComponentsToTemplate(Entity target_entity, const std::string& json_string);

    EntityTemplate ParseEntityTemplate(const std::string& raw_string);

    EntityManager();
    static EntityManager* instance;

    std::mutex auxilary_registry_mutex;
    entt::registry auxilary_registry;
    std::mutex sync_mutex;
    std::unordered_map<std::string, EntityTemplate> m_entity_cache;
};