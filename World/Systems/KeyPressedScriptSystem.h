#pragma once 
#include <World/Components/KeyPressedScriptComponent.h>
#include <World/System.h>
#include <Events/KeyPressEvent.h>
#include <World/Systems/ScriptSystem.h>
#include <World/Components/ScriptComponent.h>

inline void KeyPressedScriptSystem(World& world, KeyPressedEvent* e) {
    auto func_1 = [&world,&e](ComponentCollection compcol, system_view_type<KeyPressedScriptComponent>& comps, entt::registry* reg) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }
        
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            if (reg->all_of<ScriptComponent>(*iter)) {
                auto comp = reg->get<ScriptComponent>(*iter);
                script_vm->SetEngineEntity(Entity((uint32_t)*iter));
                script_vm->CallFunction(comp.script_path, "OnKeyPressed", *e);
            }
        };

    };

    RunSystemSimple<KeyPressedScriptComponent>(world, func_1);
}

template<>
class LuaEngineObjectDelegate<KeyPressedEvent> {
public:
    static void SetObject(LuaEngineProxy proxy, const KeyPressedEvent& value) {
        proxy.SetTableItem((int)value.key_code, "key_code");
        proxy.SetTableItem((int)value.key_mods, "key_mods");
        proxy.SetTableItem((int)value.press_type, "press_type");
    }

    static KeyPressedEvent GetObject(LuaEngineProxy proxy, int index = -1) {
        return KeyPressedEvent((KeyCode)proxy.GetTableField<int>("key_code", index), (KeyPressType)proxy.GetTableField<int>("press_type", index), (KeyModifiers)proxy.GetTableField<int>("key_mods", index));
    }

};