#pragma once 
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to manipulate the GameStateMachine.
 *
 * For the functionality provided see StateModule.cpp.
 * 
 * @note Since the GameStateMachine is not heavily utilized yet this module doesn't provide much functionality at the present moment.
*/
class StateModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("StateModule");

	/**
	 * @brief Provides the bindings with the functionality of this module.
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};