#include "DefferedPropertySetModule.h"
#include <World/Components/ScriptComponent.h>
#include <World/EntityManager.h>
#include <World/ScriptModules/GlobalEntityModule.h>
#include <FileManager.h>
#include <World/ScriptModules/MathModule.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/SerializableComponent.h>
#include <Core/Defines.h>

template<typename T>
static void SetEntityProperty(Entity entity, std::string name, T value) {
    auto map = ThreadManager::GetThreadLocalData<Deffered_Set_Map>();
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
    
    LIBEXP typedef struct entity_call_parameters_L {
        void* parameter_list = nullptr; //Type erased
    } entity_call_parameters_L;
    
    LIBEXP entity_call_parameters_L CreateCallParameters_Unmanaged() {
        return entity_call_parameters_L{ new std::vector<Script_Variant_type> };
    }

    LIBEXP void Free_entity_call_parameters_L(entity_call_parameters_L params) {
        if (params.parameter_list) {
            delete static_cast<std::vector<Script_Variant_type>*>(params.parameter_list);
        }
    };

    LIBEXP void AddCallParameterInt_L(entity_call_parameters_L params, int value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(value);
    }

    LIBEXP void AddCallParameterFloat_L(entity_call_parameters_L params, float value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(value);
    }
    LIBEXP void AddCallParameterDouble_L(entity_call_parameters_L params, double value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(value);
    }
    LIBEXP void AddCallParameterString_L(entity_call_parameters_L params, const char* value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back((std::string)value);
    }
    LIBEXP void AddCallParameterEntity_L(entity_call_parameters_L params, entity value) {
        static_cast<std::vector<Script_Variant_type>*>(params.parameter_list)->push_back(Entity(value.id));
    }


    LIBEXP entity CreateEntity_L(const char* path, int parent)
    {
        return entity{ EntityManager::Get()->CreateEntity(FileManager::Get()->GetPath(path), Entity(parent)).id };
    }

    LIBEXP entity CreateSerializableEntity_L(const char* path, int parent)
    {
        Entity ent = EntityManager::Get()->CreateEntity(FileManager::Get()->GetPath(path), Entity(parent)).id;
        Application::GetWorld().SetComponent<SerializableComponent>(ent);
        return entity{ ent.id };
    }

    LIBEXP entity CreateEntityNamed_L(const char* name ,const char* path, int parent)
    {
        return entity{ EntityManager::Get()->CreateEntity(name, FileManager::Get()->GetPath(path), Entity(parent)).id };
    }

    LIBEXP entity CreateSerializableEntityNamed_L(const char* name, const char* path, int parent)
    {
        Entity ent = EntityManager::Get()->CreateEntity(name, FileManager::Get()->GetPath(path), Entity(parent)).id;
        Application::GetWorld().SetComponent<SerializableComponent>(ent);
        return entity{ ent.id };
    }

    LIBEXP void SetEntityProperty_INT_L(entity ent, const char* name, int value) {
        SetEntityProperty(Entity(ent.id),name , value);
    }

    LIBEXP void SetEntityProperty_FLOAT_L(entity ent, const char* name, float value) {
        SetEntityProperty(Entity(ent.id), name, value);
    }

    LIBEXP void SetEntityProperty_VEC2_L(entity ent, const char* name, vec2 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec2*>(&value));
    }

    LIBEXP void SetEntityProperty_VEC3_L(entity ent, const char* name, vec3 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec3*>(&value));
    }

    LIBEXP void SetEntityProperty_VEC4_L(entity ent, const char* name, vec4 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec4*>(&value));
    }

    LIBEXP void SetEntityProperty_STRING_L(entity ent, const char* name, const char* value) {
        SetEntityProperty(Entity(ent.id), name, value);
    }

    LIBEXP void CallEntityFunction_L(entity ent, const char* name) {
        ScriptSystemManager::Get()->AddDefferedCall(Entity(ent.id), Deffered_Call{ std::string(name), std::vector< Script_Variant_type >() });
    }

    LIBEXP void CallEntityFunctionWithArguments_L(entity ent, const char* name, entity_call_parameters_L args) {
        ScriptSystemManager::Get()->AddDefferedCall(Entity(ent.id), Deffered_Call{ std::string(name), *static_cast<std::vector<Script_Variant_type>*>(args.parameter_list) });
    }
}

void DefferedPropertySetModule::OnRegisterModule(ModuleBindingProperties& props)
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
