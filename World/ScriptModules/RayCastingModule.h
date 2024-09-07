#pragma once 
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to Perform RayCasting.
 *
 * For the functionality provided see RayCastingModule.cpp.
 *
 * @note This module depends on:
 * - @ref LocalEntityModule
 * - @ref MathModule
 * 
 * ## Additional lua functionality
 *
 * This lua functionality has been implemented purely in lua and was as such not auto documented by doxygen
 *
 * ### Operators
 * 
 * The lua RayCastingResultArray and RayCastingResultPhysicsArray return iterators via a call operator.
 * 
 * 
*/
class RayCastingModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("RayCastingModule");

	/**
	 * @brief Provides the bindings with the functionality of this module.
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};