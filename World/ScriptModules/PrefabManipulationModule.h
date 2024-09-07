#pragma once 
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to manipulate the current @ref Prefabs "prefab" and its members.
 *
 * For the functionality provided see PrefabManipulationModule.cpp.
 *
 * @note This module depends on:
 * - @ref LocalPropertySetModule
 * - @ref LocalEntityModule
*/
class PrefabManipulationModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("PrefabManipulationModule");

	/**
	 * @brief Provides the bindings with the functionality of this module. 
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};