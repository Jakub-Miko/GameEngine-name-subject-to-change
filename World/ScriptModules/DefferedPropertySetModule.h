#pragma once
#include "ScriptModule.h"

class DefferedPropertySetModule {
public:
	static void RegisterModule(std::vector<LuaEngine::LuaEngine_Function_Binding>& binding_list);
};