#include "DeferredPropertySetModule.h"
#include <World/Components/ScriptComponent.h>
#include <World/EntityManager.h>
#include <World/ScriptModules/GlobalEntityModule.h>
#include <FileManager.h>
#include <World/ScriptModules/MathModule.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/SerializableComponent.h>
#include <Core/Defines.h>

/**
 * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set a property of a different Entity 
 * @tparam T Type of the property value to set
 * @param entity Entity of which to set the property
 * @param name Name of the property to set
 * @param value Value of the property to set
 * @lua
 */
template<typename T>
static void SetEntityProperty(Entity entity, std::string name, T value) {
    auto map = ThreadManager::GetThreadLocalData<Deferred_Set_Map>();
    auto fnd = map->find((uint32_t)entity.id);
    if (fnd != map->end()) {
        fnd->second.push_back(Script_Variant_Key_Value(value, name));
    }
    else {
        ScriptSystemManager::Get()->SetEntityAsDirty(Entity(entity));
        auto new_vec = map->insert(std::make_pair((uint32_t)entity.id, std::vector<Script_Variant_Key_Value>()));
        (new_vec.first)->second.push_back(Script_Variant_Key_Value(value, name));
    }
}

extern "C" {
    
    /**
     * @brief a structure serving as a lua interface, which contains the arguments with which to do a @ref ScriptSystemDeferredCall "deferred call"
     * @lua
     */
    LIBEXP typedef struct entity_call_parameters_L {
        void* parameter_list = nullptr; ///< Type erased pointer, which point to a std::vector< Script_Variant_type > containing all parameters passed to a @ref ScriptSystemDeferredCall "deferred call"
    } entity_call_parameters_L;
    
    /**
     * @brief Creates a new instance of @ref entity_call_parameters_L
     * @return an instance of @ref entity_call_parameters_L
     * @lua
     */
    LIBEXP entity_call_parameters_L CreateCallParameters_Unmanaged() {
        return entity_call_parameters_L{ new std::vector<Script_Variant_type> };
    }

    /**
     * @brief Destroys an instance of @ref entity_call_parameters_L
     * @param params an instance of @ref entity_call_parameters_L
     * @lua
     */
    LIBEXP void Free_entity_call_parameters_L(entity_call_parameters_L params) {
        if (params.parameter_list) {
            delete static_cast<std::vector<Script_Variant_type>*>(params.parameter_list);
        }
    };

    /**
     * @brief Adds a new integer call parameter to an instance of @ref entity_call_parameters_L
     * @param params the instance of @ref entity_call_parameters_L to add the parameter to 
     * @param value the integer value to add to params
     * @lua
     */
    LIBEXP void AddCallParameterInt_L(entity_call_parameters_L params, int value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(value);
    }

    /**
     * @brief Adds a new float call parameter to an instance of @ref entity_call_parameters_L
     * @param params the instance of @ref entity_call_parameters_L to add the parameter to
     * @param value the float value to add to params
     * @lua
     */
    LIBEXP void AddCallParameterFloat_L(entity_call_parameters_L params, float value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(value);
    }

    /**
     * @brief Adds a new double call parameter to an instance of @ref entity_call_parameters_L
     * @param params the instance of @ref entity_call_parameters_L to add the parameter to
     * @param value the double value to add to params
     * @lua
     */
    LIBEXP void AddCallParameterDouble_L(entity_call_parameters_L params, double value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(value);
    }

    /**
     * @brief Adds a new string call parameter to an instance of @ref entity_call_parameters_L
     * @param params the instance of @ref entity_call_parameters_L to add the parameter to
     * @param value the string value to add to params
     * @lua
     */
    LIBEXP void AddCallParameterString_L(entity_call_parameters_L params, const char* value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back((std::string)value);
    }

    /**
     * @brief Adds a new Entity call parameter to an instance of @ref entity_call_parameters_L
     * @param params the instance of @ref entity_call_parameters_L to add the parameter to
     * @param value the Entity to add to params
     * @lua
     */
    LIBEXP void AddCallParameterEntity_L(entity_call_parameters_L params, entity value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(Entity(value.id));
    }

    /**
     * @brief Creates a new Entity from a file
     * @param path the path to the file the entity is created from 
     * @param parent the parent entity of the newly created entity 
     * @return the newly created entity 
     * 
     * @note Unlike CreateSerializableEntity_L, this method creates non=serializable Entities which will not be saved even when the scene is saved, this is useful for entities spawned by scripts.
     * @lua
     */
    LIBEXP entity CreateEntity_L(const char* path, int parent)
    {
        return entity{ EntityManager::Get()->CreateEntity(FileManager::Get()->GetPath(path), Entity(parent)).id };
    }

    /**
     * @brief Creates a new Entity from a file and makes it serializable, so it will be saved as a part of the scene when the scene is saved
     * @param path the path to the file the entity is created from
     * @param parent the parent entity of the newly created entity
     * @return the newly created entity
     * 
     * @note This function is only useful with the editor, since the runtime will generally not save scenes, only load them.
     * @lua
     */
    LIBEXP entity CreateSerializableEntity_L(const char* path, int parent)
    {
        Entity ent = EntityManager::Get()->CreateEntity(FileManager::Get()->GetPath(path), Entity(parent)).id;
        Application::GetWorld().SetComponent<SerializableComponent>(ent);
        return entity{ ent.id };
    }

    /**
     * @brief Creates a new Entity from a file and gives it a name
     * @param name the name to give the new entity.
     * @param path the path to the file the entity is created from
     * @param parent the parent entity of the newly created entity
     * @return the newly created entity
     *
     * @note Unlike CreateSerializableEntity_L, this method creates non=serializable Entities which will not be saved even when the scene is saved, this is useful for entities spawned by scripts.
     * @lua
     */
    LIBEXP entity CreateEntityNamed_L(const char* name ,const char* path, int parent)
    {
        return entity{ EntityManager::Get()->CreateEntity(name, FileManager::Get()->GetPath(path), Entity(parent)).id };
    }

    /**
     * @brief Creates a new Entity from a file gives it a name, and makes it serializable, so it will be saved as a part of the scene when the scene is saved
     * @param name the name to give the new entity.
     * @param path the path to the file the entity is created from
     * @param parent the parent entity of the newly created entity
     * @return the newly created entity
     *
     * @note This function is only useful with the editor, since the runtime will generally not save scenes, only load them.
     * @lua
     */
    LIBEXP entity CreateSerializableEntityNamed_L(const char* name, const char* path, int parent)
    {
        Entity ent = EntityManager::Get()->CreateEntity(name, FileManager::Get()->GetPath(path), Entity(parent)).id;
        Application::GetWorld().SetComponent<SerializableComponent>(ent);
        return entity{ ent.id };
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set an integer property of a different Entity
     * @param ent The entity to set the property of 
     * @param name the name of the property to set
     * @param value the value of the property to set
     * @lua
     */
    LIBEXP void SetEntityProperty_INT_L(entity ent, const char* name, int value) {
        SetEntityProperty(Entity(ent.id),name , value);
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set a float property of a different Entity
     * @param ent The entity to set the property of
     * @param name the name of the property to set
     * @param value the value of the property to set
     * @lua
     */
    LIBEXP void SetEntityProperty_FLOAT_L(entity ent, const char* name, float value) {
        SetEntityProperty(Entity(ent.id), name, value);
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set a vec2 property of a different Entity
     * @param ent The entity to set the property of
     * @param name the name of the property to set
     * @param value the value of the property to set
     * @lua
     */
    LIBEXP void SetEntityProperty_VEC2_L(entity ent, const char* name, vec2 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec2*>(&value));
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set a vec3 property of a different Entity
     * @param ent The entity to set the property of
     * @param name the name of the property to set
     * @param value the value of the property to set
     * @lua
     */
    LIBEXP void SetEntityProperty_VEC3_L(entity ent, const char* name, vec3 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec3*>(&value));
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set a vec4 property of a different Entity
     * @param ent The entity to set the property of
     * @param name the name of the property to set
     * @param value the value of the property to set
     * @lua
     */
    LIBEXP void SetEntityProperty_VEC4_L(entity ent, const char* name, vec4 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec4*>(&value));
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredSet "Deferred Set" request to set a string property of a different Entity
     * @param ent The entity to set the property of
     * @param name the name of the property to set
     * @param value the value of the property to set
     * @lua
     */
    LIBEXP void SetEntityProperty_STRING_L(entity ent, const char* name, const char* value) {
        SetEntityProperty(Entity(ent.id), name, value);
    }


    /**
     * @brief Creates a @ref ScriptSystemDeferredCall "deferred call" to call a function of a different entity.
     * @param ent Entity of which the function should be called
     * @param name name of the function to call
     * 
     * @note this version does not support arguments
     * @lua
     */
    LIBEXP void CallEntityFunction_L(entity ent, const char* name) {
        ScriptSystemManager::Get()->AddDeferredCall(Entity(ent.id), Deferred_Call{ std::string(name), std::vector< Script_Variant_type >() });
    }

    /**
     * @brief Creates a @ref ScriptSystemDeferredCall "deferred call" to call a function of a different entity.
     * @param ent Entity of which the function should be called
     * @param name name of the function to call
     * @param args the arguments to pass to the function, use the lua CreateCallParameters function to create this and AddCallParameter* to set the parameters
     * @lua
     */
    LIBEXP void CallEntityFunctionWithArguments_L(entity ent, const char* name, entity_call_parameters_L args) {
        ScriptSystemManager::Get()->AddDeferredCall(Entity(ent.id), Deferred_Call{ std::string(name), *static_cast<std::vector<Script_Variant_type>*>(args.parameter_list) });
    }
}

void DeferredPropertySetModule::OnRegisterModule(ModuleBindingProperties& props)
{
    GlobalEntityModule().RegisterModule(props);
    MathModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
    entity CreateEntity_L(const char* path, int parent);
    entity CreateSerializableEntity_L(const char* path, int parent);
    entity CreateEntityNamed_L(const char* name ,const char* path, int parent);
    entity CreateSerializableEntityNamed_L(const char* name, const char* path, int parent);
    void SetEntityProperty_INT_L(entity ent, const char* name, int value);
    void SetEntityProperty_FLOAT_L(entity ent, const char* name, float value);
    void SetEntityProperty_VEC2_L(entity ent, const char* name, vec2 value);
    void SetEntityProperty_VEC3_L(entity ent, const char* name, vec3 value);
    void SetEntityProperty_VEC4_L(entity ent, const char* name, vec4 value);
    void SetEntityProperty_STRING_L(entity ent, const char* name, const char* value);
    void CallEntityFunction_L(entity ent, const char* name);

    typedef struct entity_call_parameters_L {
        void* parameter_list; 
    } entity_call_parameters_L;
    entity_call_parameters_L CreateCallParameters_Unmanaged();
    void Free_entity_call_parameters_L(entity_call_parameters_L params);

    void AddCallParameterInt_L(entity_call_parameters_L params, int value);
    void AddCallParameterFloat_L(entity_call_parameters_L params, float value);
    void AddCallParameterDouble_L(entity_call_parameters_L params, double value); 
    void AddCallParameterString_L(entity_call_parameters_L params, const char* value); 
    void AddCallParameterEntity_L(entity_call_parameters_L params, entity value); 
    void CallEntityFunctionWithArguments_L(entity ent, const char* name, entity_call_parameters_L args);

    )");

    props.Add_FFI_aliases({
        {"CreateEntity_L","CreateEntity"},
        {"CreateSerializableEntity_L","CreateSerializableEntity"},
        {"CreateEntityNamed_L","CreateEntityNamed"},
        {"CreateSerializableEntityNamed_L","CreateSerializableEntityNamed"},
        {"SetEntityProperty_INT_L","SetEntityProperty_INT"},
        {"SetEntityProperty_FLOAT_L","SetEntityProperty_FLOAT"},
        {"SetEntityProperty_VEC2_L","SetEntityProperty_VEC2"},
        {"SetEntityProperty_VEC3_L","SetEntityProperty_VEC3"}, 
        {"SetEntityProperty_VEC4_L","SetEntityProperty_VEC4"},
        {"SetEntityProperty_STRING_L","SetEntityProperty_STRING"},
        {"CallEntityFunction_L" ,"CallEntityFunction"},
        {"CreateCallParameters_Unmanaged","CreateCallParameters_Unmanaged"},
        {"Free_entity_call_parameters_L", "Free_entity_call_parameters"},
        {"struct entity_call_parameters_L", "entity_call_parameters"},
        {"AddCallParameterInt_L", "AddCallParameterInt"},
        {"AddCallParameterFloat_L", "AddCallParameterFloat"},
        {"AddCallParameterDouble_L", "AddCallParameterDouble"},
        {"AddCallParameterString_L", "AddCallParameterString"},
        {"AddCallParameterEntity_L", "AddCallParameterEntity"},
        {"CallEntityFunctionWithArguments_L", "CallEntityFunctionWithArguments"}

        });

    props.Add_init_script(R"(
        function CreateCallParameters() 
			return ffi.gc(CreateCallParameters_Unmanaged(), Free_entity_call_parameters)
		end

    )");

}
