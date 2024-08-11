#pragma once
#include <unordered_map>
#include <World/EntityParser.h>
#include <mutex>
#include <string>
#include <vector>
#include <deque>
#include <World/Entity.h>

/** @page Prefabs Prefabs
 * Prefabs or Prefabrications are groups of entities organized in a hierarchy with only the root of this hierarchy, the Prefab root, being represented in the scene hierarchy.
 * They allow for construction of more complex reusable Entities by combing multiple entities together and making them behave as one from the perspective of a Scene.
 * 
 * They are serialized in their own files and can be reused across multiple Scenes. Multiple instances of a single prefab file can exist in a Scene by creating an
 * Entity with a Prefab Component linked to a certian prefab file. This Entity will become a Prefab root, and the engine will populate its prefab children with the entities
 * in a defined prefab file. The scene can override the properties of the Prefab root and DynamicPropertiesComponent along side with scripting can be used to pass down instance specific data down to 
 * Prefab children.
 * 
 * @section Scripting
 * A prefab is also the most basic unit of scripting, allowing for each prefab to have an @ref inline_script "Inline Script" and a @ref construction_script "Construction Script". 
 * A Prefab root with a script must contain a ScriptComponent.
 * 
 * @anchor inline_script
 * Inline script
 * --------
 * The Inline script is executed on runtime either every frame(OnUpdate), or on certain events such as Key presses(OnKeyPressed), 
 Mouse presses(OnMouseButtonPressed), Collisions(OnCollision) or Spawn(OnStart). 
 * 
 * Example:
 * ~~~
function OnUpdate(delta_time)
    speed = GetProperty_FLOAT("Speed")
    if(IsKeyPressed(KeyCode.KEY_W)) then
        translation = GetTranslation()
        translation = translation + rotate_vec3(vec3(0,0,-1*speed), GetRotation())
        SetTranslation(translation)
    end
    norm_scree_pos = GetMousePosition()
    offset = vec2({ x = 0.0, y =0.0 })
    norm_scree_pos = offset + norm_scree_pos;
    norm_scree_pos = norm_scree_pos * vec2({-1,-1});
    norm_scree_pos.y = math.min(math.max(-0.8, norm_scree_pos.y),0.8)
    pos = {
    x = math.sin(norm_scree_pos.x * 3.1415926) * math.cos(norm_scree_pos.y * 3.1415926/2) ,
    y = math.sin(norm_scree_pos.y * 3.1415926 / 2),
    z = math.cos(norm_scree_pos.x * 3.1415926) * math.cos(norm_scree_pos.y * 3.1415926/2)
    }
    rot = quat_lookat(-vec3(pos), vec3({0,1,0}) )
    SetRotation(rot)
end

function OnKeyPressed(e)

end

function OnMouseButtonPressed(e)

end

function OnCollision(event)
    if(event.num_collision_points == 0) then return end
    local distance = event.collision_points[1].y - GetChildWorldTranslation("CollisionBox").y + GetChildScale("CollisionBox").y
    if (math.abs(distance)< 0.05) then
        SetProperty_INT("Collides_bottom",1)
    else
        SetProperty_INT("Collides_bottom",0)
    end
end

function OnStart() 
	PlayAnimation("Skeleton","asset:default_dance_vampire_animations/DanceMoves.anim")
	PlaySound("Audio","asset:test_track_mono.wav")
	print("Start")
end
 * ~~~
 * 
 * 
 * @anchor construction_script
 * Construction script
 * --------
 * The Construction script runs when a Prefab instance gets spawned, and can utilize the data in the customized DynamicPropertiesComponent to alter the Prefab on Construction before
 * any prefab children are deserialized.
 * @note Construction scripts are currently not very useful and may be completely revised in the future
 * Example:
 * ~~~
function OnConstruct()
    pos = GetMousePosition()
    jit.on()
    pos = GetMousePosition()
    SetTranslation({x=pos.x,y=pos.y,z=0.0})
end
 * ~~~
 * 
*/

/**
 * @brief Entry for a deffered contruction of an entity from a file
 * 
 * @deprecated Not used anymore, replaced by ConstructionComponent
*/
struct Construction_Entry {
    Construction_Entry(Entity id, const std::string& path, Entity parent) : id(id), path(path), parent(parent) {}
    Entity id; ///< Empty Entity to initialize 
    Entity parent; ///< The parent of the new entity 
    std::string path; ///< The path to initialize the entity from
};

/**
 * @brief Contains a template of an entity loaded from a file
 * 
 * Entities loaded from a file are created by replicating an EntityTemplate associated with the loaded file.
*/
struct EntityTemplate {
    Entity template_entity; ///< Entity existing in a separate ECS from the world, which is replicated to create actual entities in the world
    std::string construction_script = ""; ///< Construction script ran when an entity is created from this template @warning Should only be used on Prefab roots
    std::string inline_script = ""; ///< Script containing the runtime behaviour of an Entity @warning Should only be used on Prefab roots
    DynamicPropertiesComponent properties; ///< Dynamic properties containing defaults for custom data specific to Entity instances created from this EntityTemplate
    std::vector<std::string> children; ///< Identifiers used to reference child EntityTemplates, allowing for a hierarchy of Entities to be stored (For prefabs) 
    bool has_inline = false; ///< Whether this entity has an Inline(Runtime) Script
};

/**
 * @brief Singleton responsible for loading Entities and Prefabs from files
*/
class EntityManager {
public:
    EntityManager(const EntityManager& ref) = delete;
    EntityManager(EntityManager&& ref) = delete;
    EntityManager& operator=(const EntityManager& ref) = delete;
    EntityManager& operator=(EntityManager&& ref) = delete;

    /**
     * @brief Singleton initializer
    */
    static void Initialize();
    /**
     * @brief Singleton getter
    */
    static EntityManager* Get();
    /**
     * @brief Signleton destructor
    */
    static void Shutdown();
    
    /**
     * @brief Loads a Prefab from a file. This function is deffered
     * @param path file to create the prefab from 
     * @param parent the parent of the new prefab Root entity
     * @return the new Prefab root entity
    */
    Entity CreateEntity(const std::string& path, Entity parent = Entity());

    /**
     * @brief Loads a Prefab from a file. Unlike @ref EntityManager::CreateEntity(const std::string&, Entity), this function executes immediately.
     * @param path file to create the prefab from
     * @param parent the parent of the new prefab Root entity
     * @return the new Prefab root entity
     * @see EntityManager::CreateEntityInplace_existing_entity
    */
    Entity CreateEntityInplace(const std::string& path, Entity parent = Entity());

    /**
     * @brief Similar to @ref EntityManager::CreateEntityInplace(const std::string&, Entity), but creates the prefab from an Existing Entity
     * @param base_entity Entity to create the prefab from
     * @param path file to create the prefab from
     * @param parent the parent of the new prefab Root entity
     * @return the new Prefab root entity
    */
    Entity CreateEntityInplace(Entity base_entity, const std::string& path, Entity parent = Entity());

    /**
     * @brief Similar to @ref EntityManager::CreateEntity(const std::string& , Entity), but assigns a LabelComponent to the new Entity
     * @param name Name to assign to the LabelComponent
     * @param path file to create the prefab from
     * @param parent the parent of the new prefab Root entity
     * @return the new Prefab root entity
    */
    Entity CreateEntity(const std::string& name, const std::string& path, Entity parent = Entity());

    void AddConstructionScriptToPrefab(const std::string& prefab_name, const std::string& construction_script);
    void RemoveConstructionScriptToPrefab(const std::string& prefab_name);

    void AddInlineScriptToPrefab(const std::string& prefab_name, const std::string& inline_script);
    void RemoveInlineScriptToPrefab(const std::string& prefab_name);

    /**
     * @brief Similar to @ref EntityManager::CreateEntityInplace(const std::string&, Entity), but assigns a LabelComponent to the new Entity
     * @param name Name to assign to the LabelComponent
     * @param path file to create the prefab from
     * @param parent the parent of the new prefab Root entity
     * @return the new Prefab root entity
    */
    Entity CreateEntityInplace(const std::string& name, const std::string& path, Entity parent = Entity());

    /**
     * @brief Similar to @ref EntityManager::CreateEntityInplace(const std::string&, const std::string&, Entity parent), but creates the prefab from an Existing Entity
     * @param name Name to assign to the LabelComponent
     * @param base_entity Entity to create the prefab from
     * @param path file to create the prefab from
     * @param parent the parent of the new prefab Root entity
     * @return the new Prefab root entity
    */
    Entity CreateEntityInplace(const std::string& name, Entity base_entity, const std::string& path, Entity parent = Entity());

    /**
     * @brief Removes an EntityTemplate loaded from a file from the cache, causing the next load to reload it from the file again
     * @param name Identifier of an EntityTemplate loaded from a file(usually the relative path from the config file folder)
     * @bug If EntityManager::GetEntitySignature is used the path will be absolute, which can cause duplicate entries of EntityTemplate
    */
    void ClearPrefabCacheEntry(const std::string& name);

    /**
     * @brief Replicates the template entities compoents on an actual entity.
     * @param target_entity Entity which is a part of the World, on which compoenents will be replicated
     * @param template_entity a Pseudo entity which is not a part of the World but a part of a template ECS registry, used as a template for the components (member of EntityTemplate)
     * @param exclude_ids ECS registry ids to exclude replicating certain types of components
     * @warning Should be used only internally or if necessary, improper use may cause undefined behaviour
    */
    void InitializeFromTemplate(Entity target_entity, Entity template_entity, const std::vector<std::string>& exclude_ids = std::vector<std::string>());

    /**
     * @brief Used to Initliaze an Entity as a Prefab deserialized from a file
     * @param target_entity Entity to initialize, cannot be a part of the scene yet, unless it has been deserialized after a SceneLoad and already has a PrefabComponent
     * @param path path to the Prefab to use
     * @param parent Parent of the new entity 
     * @bug If an entity that is already a part of a SceneGraph if supplied, undefined behaviour may occur.
    */
    void DeserializeEntityPrefab(Entity target_entity, const std::string& path, Entity parent = Entity());

    /**
     * @brief Recreates components from a jso object on an Entity existing in a World
     * @param target_entity entity to assign new components
     * @param json_string json object containing serialized component data
    */
    void DeserializeComponents(Entity target_entity, const std::string& json_string);

    /**
     * @brief Get an EntityTemplate from a file
     * @param path path to a prefab of a serialized entity
     * @return an EntityTemplate loaded from the file
    */
    const EntityTemplate& GetEntitySignature(const std::string& path);

    /**
     * @brief Generates a json object from Components belonging to an Entity
     * @param entity Entity components of which to serialize
     * @return a serialized json object containing the component data of an Entity
    */
    std::string SerializeComponentsToJson(Entity entity);

private:
    friend class World;

    /**
     * @brief Clears all EntityTemplates from the cache, causing all prefabs and entities created from a file after this call to reload from their files and refill the cache
    */
    void ClearPrefabCache();

    /**
     * @brief Similar to EntityManager::GetEntitySignature but optionally loads an entity from a file subsection, allowing more enitities and entire prefabs to be store in one file
     * @param path path of the file containing the entity definition
     * @param file_buffer string containing the file contents of the currently open file to resolve local references
     * @return an EntityTemplate loaded from the file
     * 
     * @warning This is used only internally when loading Prefabs
    */
    const EntityTemplate& GetEntitySignatureLocal(const std::string& path, const std::string& file_buffer);

    /**
     * @brief A recursive function used to Load prefabs and their entire Prefab children trees
     * @param path Path to the file subsection containing the currently processed prefab child
     * @param original_path file path of the Prefab Root component
     * @param local_file_buffer content of the file at original_path to use when a local subsection is used(Subsection withing the Root file)
     * @param parent Parent of the Prefab child in the Prefab hierarchy
     * @param prefab_parent Prefab root
    */
    void DeserializeEntityPrefab_impl( const std::string& path, const std::string& original_path ,const std::string& local_file_buffer, Entity parent = Entity(), Entity prefab_parent = Entity());

    /**
     * @brief Same as EntityManager::DeserializeComponents but initializes the components of a template Entity existing in an ECS seperate from the World
     * @param target_entity entity to assign new components
     * @param json_string json object containing serialized component data
    */
    void DeserializeComponentsToTemplate(Entity target_entity, const std::string& json_string);

    /**
     * @brief Use EntityParser to parse the Entity data in a file and generate an EntityTemplate
     * @param raw_string string containing the serialized Entity data
     * @param path path of the file subsection to use for reading the Lua Script it contains
     * @return an EntityTemplate loaded from the raw_string
    */
    EntityTemplate ParseEntityTemplate(const std::string& raw_string, const std::string& path);

    /**
     * @brief Default constructor, used only by the Singleton initializer
    */
    EntityManager();
    static EntityManager* instance;

    std::mutex auxilary_registry_mutex; ///< Mutex for EntityManager::auxilary_registry
    entt::registry auxilary_registry; ///< an ECS Registry containing the EntityTemplate data to be replicated when spawning an Entity loaded from a file
    std::mutex sync_mutex; ///< Mutex for EntityManager::m_entity_cache
    std::unordered_map<std::string, EntityTemplate> m_entity_cache; ///< all EntityTemplates associated with the filepath they have been loaded from
};