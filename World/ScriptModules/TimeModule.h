#pragma once 
#include <LuaEngine.h>


/**
 * @brief Provides Lua Scripts with functionality to access time based functionality.
 *
 * For the functionality provided see TimeModule.cpp.
*/
class TimeModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("TimeModule");

	/**
	 * @brief Provides the bindings with the functionality of this module. 
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};