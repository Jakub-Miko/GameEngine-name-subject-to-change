#pragma once 
#include "ScriptModule.h"

class IOModule {
public:
	static void RegisterModule(std::vector<LuaEngine::LuaEngine_Function_Binding>& binding_list);
};