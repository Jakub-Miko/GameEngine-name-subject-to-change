#pragma once
#include <LuaEngine.h>

class DeferredPropertySetModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("DeferredPropertySetModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};