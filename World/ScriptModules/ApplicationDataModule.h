#pragma once 
#include <LuaEngine.h>

class ApplicationDataModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("ApplicationDataModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};