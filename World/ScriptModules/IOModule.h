#pragma once 
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to poll input from the keyboard and mouse.
 *
 * For the functionality provided see IOModule.cpp.
 *
 * @note This module depends on:
 * - @ref MathModule
*/
class IOModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("IOModule");

	/**
	 * @brief Provides the bindings with the functionality of this module.
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};