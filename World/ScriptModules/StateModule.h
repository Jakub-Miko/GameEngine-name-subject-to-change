#pragma once 
#include <LuaEngine.h>

class StateModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("StateModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};