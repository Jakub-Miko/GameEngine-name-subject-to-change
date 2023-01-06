#pragma once 
#include <LuaEngine.h>


class RayCastingModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("RayCastingModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};