#pragma once 
#include <LuaEngine.h>

extern "C" {

}



class PrefabManipulationModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("PrefabManipulationModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};