#pragma once 
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to toggle event reception.
 *
 * For the functionality provided see EventModule.cpp.
 *
 * @note This module depends on:
 * - @ref LocalEntityModule
*/
class EventModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("EventModule");
	/**
	 * @brief Provides the bindings with the functionality of this module.
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};