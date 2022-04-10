#include "DefferedPropertySetModule.h"
#include <World/Components/ScriptComponent.h>
#include <World/EntityManager.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <World/Components/SerializableComponent.h>

template<typename T>
static void SetEntityProperty(int entity, std::string name, T value) {
    auto map = ThreadManager::GetThreadLocalData<Deffered_Set_Map>();
    auto fnd = map->find((uint32_t)entity);
    if (fnd != map->end()) {
        fnd->second.push_back(Script_Variant_Key_Value(value, name));
    }
    else {
        ScriptSystemManager::Get()->SetEntityAsDirty(Entity(entity));
        auto new_vec = map->insert(std::make_pair((uint32_t)entity, std::vector<Script_Variant_Key_Value>()));
        (new_vec.first)->second.push_back(Script_Variant_Key_Value(value, name));
    }
}

static int CreateEntity(std::string path, int parent)
{
    return EntityManager::Get()->CreateEntity(path, Entity(parent)).id;
}


static int CreateSerializableEntity(std::string path, int parent)
{
    Entity ent = EntityManager::Get()->CreateEntity(path, Entity(parent)).id;
    Application::GetWorld().SetComponent<SerializableComponent>(ent);
    return ent.id;
}



void DefferedPropertySetModule::RegisterModule(std::vector<LuaEngine::LuaEngine_Function_Binding>& binding_list)
{
    binding_list.insert(binding_list.end(), {
        //This is where function bindings go
        LUA_FUNCTION("SetEntityProperty_INT" ,SetEntityProperty<int>),                 
        LUA_FUNCTION("SetEntityProperty_FLOAT" ,SetEntityProperty<float>),             
        LUA_FUNCTION("SetEntityProperty_VEC2" ,SetEntityProperty<glm::vec2>),          
        LUA_FUNCTION("SetEntityProperty_VEC3" ,SetEntityProperty<glm::vec3>),          
        LUA_FUNCTION("SetEntityProperty_VEC4" ,SetEntityProperty<glm::vec4>),          
        LUA_FUNCTION("SetEntityProperty_STRING" ,SetEntityProperty<std::string>),      
        LUA_FUNCTION("CreateEntity", CreateEntity),
        LUA_FUNCTION("CreateSerializableEntity", CreateSerializableEntity)

        });
}
