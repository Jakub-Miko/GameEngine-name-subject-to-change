#pragma once 
#include <LuaEngine.h>

class EventModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("EventModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};