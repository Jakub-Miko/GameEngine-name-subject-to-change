#pragma once
#include <LuaEngine.h>

class LocalPropertySetModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("LocalPropertySetModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};