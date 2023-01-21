#pragma once 
#include <LuaEngine.h>

extern "C" {

	typedef struct entity { uint32_t id; } entity;

}

template<>
class LuaEngineObjectDelegate<entity> {
public:

	static void SetObject(LuaEngineProxy proxy, const entity& value) {
		proxy.SetTableItem((int)value.id, "id");
	}

	static entity GetObject(LuaEngineProxy proxy, int index = -1) {
		return entity{ (unsigned int)proxy.GetTableField<int>("id", index) };
	}
};

class GlobalEntityModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("GlobalEntityModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};