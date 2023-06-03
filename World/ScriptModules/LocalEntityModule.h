#pragma once 
#include <LuaEngine.h>
#include <ThreadManager.h>
#include <World/Systems/ScriptSystemManagement.h>
#include "GlobalEntityModule.h"
#include "MathModule.h"
#include <Core/Defines.h>

extern "C" {
	LIBEXP entity GetCurrentEntity_L();
}

class LocalEntityModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("LocalEntityModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};