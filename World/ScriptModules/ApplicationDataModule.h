#pragma once 
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to access Application global data. 
 * 
 * For the functionality provided see ApplicationDataModule.cpp. 
 * 
 * @note This module depends on:
 * - @ref GlobalEntityModule
 * - @ref MathModule
*/
class ApplicationDataModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("ApplicationDataModule");

	/**
	 * @brief Provides the bindings with the functionality of this module. 
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};