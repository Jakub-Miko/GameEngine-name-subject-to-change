#pragma once
#include <LuaEngine.h>

/**
 * @brief Provides Lua Scripts with functionality to access setting properties of other entities and calling their functions in a thread-safe way 
 *
 * Since all Entity scripts are executed in parallel, it is not thread-safe to call function or set properties across Entity boundaries, since any entity other than the one being processed
 * can be running their own script in parallel and thus a data race would occur when both the Entity itself and another entity calling across Entity boundaries would access the Entity data.
 * 
 * This Module provides a thread safe way of communicating across Entity boundaries, by deferring function calls and property set operations until they can be reorganized to run in a thread-safe manner.
 * 
 * @see 
 * 
 * - @ref ScriptSystemDeferredSet
 * - @ref ScriptSystemDeferredCall
 * 
 * For the functionality provided see DeferredPropertySetModule.cpp.
 *
 * @note This module depends on:
 * - @ref GlobalEntityModule
 * - @ref MathModule
*/
class DeferredPropertySetModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("DeferredPropertySetModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};