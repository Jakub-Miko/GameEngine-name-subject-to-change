#pragma once
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to set the properties of the currently processed Entity 
 * 
 * Unlike DeferredPropertySetModule this only allows setting the properties of the currently processed Entity and does not defer the set action. 
 *
 * For the functionality provided see LocalPropertySetModule.cpp.
 *
 * @note This module depends on:
 * - @ref LocalEntityModule
*/
class LocalPropertySetModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("LocalPropertySetModule");

	/**
	 * @brief Provides the bindings with the functionality of this module.
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};