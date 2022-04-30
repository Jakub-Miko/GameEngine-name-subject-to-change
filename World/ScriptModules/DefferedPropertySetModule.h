#pragma once
#include <LuaEngine.h>

class DefferedPropertySetModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("DefferedPropertySetModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};