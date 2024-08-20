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

/**
 * @brief Provides Lua Scripts with functionality to to access operations on the entity being currently processed.
 *
 * These operations are usually thread-safe without requiring synchronization or deferred calls, which means they are more performant than other operations
 * 
 * For the functionality provided see LocalEntityModule.cpp.
 *
 * @note This module depends on:
 * - @ref GlobalEntityModule
 * - @ref MathModule
*/
class LocalEntityModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("LocalEntityModule");

	/**
	 * @brief Provides the bindings with the functionality of this module. 
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};