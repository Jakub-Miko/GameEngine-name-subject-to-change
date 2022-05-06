#pragma once 
#include <LuaEngine.h>

extern "C" {

	typedef struct entity { uint32_t id; } entity;

}


class GlobalEntityModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("GlobalEntityModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};