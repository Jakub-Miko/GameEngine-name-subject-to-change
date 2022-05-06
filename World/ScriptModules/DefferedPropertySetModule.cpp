#include "DefferedPropertySetModule.h"
#include <World/Components/ScriptComponent.h>
#include <World/EntityManager.h>
#include <World/ScriptModules/GlobalEntityModule.h>
#include <World/ScriptModules/MathModule.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/SerializableComponent.h>

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
    entity CreateEntity_L(const char* path, int parent)
    {
        return entity{ EntityManager::Get()->CreateEntity(path, Entity(parent)).id };
    }

    entity CreateSerializableEntity_L(const char* path, int parent)
    {
        Entity ent = EntityManager::Get()->CreateEntity(path, Entity(parent)).id;
        Application::GetWorld().SetComponent<SerializableComponent>(ent);
        return entity{ ent.id };
    }

    void SetEntityProperty_INT_L(entity ent, const char* name, int value) {
        SetEntityProperty(Entity(ent.id),name , value);
    }

    void SetEntityProperty_FLOAT_L(entity ent, const char* name, float value) {
        SetEntityProperty(Entity(ent.id), name, value);
    }

    void SetEntityProperty_VEC2_L(entity ent, const char* name, vec2 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec2*>(&value));
    }

    void SetEntityProperty_VEC3_L(entity ent, const char* name, vec3 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec3*>(&value));
    }

    void SetEntityProperty_VEC4_L(entity ent, const char* name, vec4 value) {
        SetEntityProperty(Entity(ent.id), name, *reinterpret_cast<glm::vec4*>(&value));
    }

    void SetEntityProperty_STRING_L(entity ent, const char* name, const char* value) {
        SetEntityProperty(Entity(ent.id), name, value);
    }

}

void DefferedPropertySetModule::OnRegisterModule(ModuleBindingProperties& props)
{
    GlobalEntityModule().RegisterModule(props);
    MathModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
    entity CreateEntity_L(const char* path, int parent);
    entity CreateSerializableEntity_L(const char* path, int parent);
    void SetEntityProperty_INT_L(entity ent, const char* name, int value);
    void SetEntityProperty_FLOAT_L(entity ent, const char* name, float value);
    void SetEntityProperty_VEC2_L(entity ent, const char* name, vec2 value);
    void SetEntityProperty_VEC3_L(entity ent, const char* name, vec3 value);
    void SetEntityProperty_VEC4_L(entity ent, const char* name, vec4 value);
    void SetEntityProperty_STRING_L(entity ent, const char* name, const char* value);
    )");

    props.Add_FFI_aliases({
        {"CreateEntity_L","CreateEntity"},
        {"CreateSerializableEntity_L","CreateSerializableEntity"},
        {"SetEntityProperty_INT_L","SetEntityProperty_INT"},
        {"SetEntityProperty_FLOAT_L","SetEntityProperty_FLOAT"},
        {"SetEntityProperty_VEC2_L","SetEntityProperty_VEC2"},
        {"SetEntityProperty_VEC3_L","SetEntityProperty_VEC3"}, 
        {"SetEntityProperty_VEC4_L","SetEntityProperty_VEC4"},
        {"SetEntityProperty_STRING_L","SetEntityProperty_STRING"},
        });

}
