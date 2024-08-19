#pragma once 
#include <LuaEngine.h>
#include <Core/Defines.h>
extern "C" {

	/**
	 * @brief a class representing an Entity in Lua scripts
	*/
	LIBEXP typedef struct entity { uint32_t id; } entity;

}

/**
 * @brief A LuaEngineObjectDelegate specialization, which provides an interface between the C++ Entity type and its Lua counterpart.
*/
template<>
class LuaEngineObjectDelegate<entity> {
public:

	/** 
	 * @brief An interface which converts the C++ structure to a Lua representation.
	 * @param proxy an object which exposes methods to build out the Lua representation of an the specialized structure.
	 * @param value the C++ object to convert
	*/
	static void SetObject(LuaEngineProxy proxy, const entity& value) {
		proxy.SetTableItem((int)value.id, "id");
	}

	/**
	 * @brief An interface which converts the Lua representation of a structure to a C++ structure.
	 * @param proxy an object which exposes methods read out the Lua structure representation from the Lua state
	 * @param index The index at which the table with the structure is located in the Lua stack. 
	 * @return an instance of the C++ structure
	*/
	static entity GetObject(LuaEngineProxy proxy, int index = -1) {
		return entity{ (unsigned int)proxy.GetTableField<int>("id", index) };
	}
};

/**
 * @brief Provides Lua Scripts with a definition of an Entity type and an IsEntityValid function.
 *
 * For the functionality provided see GlobalEntityModule.cpp.
*/
class GlobalEntityModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("GlobalEntityModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};