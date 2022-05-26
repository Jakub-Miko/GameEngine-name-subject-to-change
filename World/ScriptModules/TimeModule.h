#pragma once 
#include <LuaEngine.h>

class TimeModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("TimeModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};