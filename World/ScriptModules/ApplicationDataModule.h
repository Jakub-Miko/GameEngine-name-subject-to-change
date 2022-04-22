#pragma once 
#include "ScriptModule.h"

class ApplicationDataModule {
public:
	static void RegisterModule(std::vector<LuaEngine::LuaEngine_Function_Binding>& binding_list);
};