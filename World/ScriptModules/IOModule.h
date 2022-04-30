#pragma once 
#include <LuaEngine.h>

class IOModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("IOModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};