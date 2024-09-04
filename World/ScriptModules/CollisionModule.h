#pragma once 
#include <LuaEngine.h>
#include <World/ScriptModules/MathModule.h>
#include <World/ScriptModules/GlobalEntityModule.h>
#include <Core/Defines.h>

extern "C" {
	/**
	 * @brief a class representing a CollisionEvent in Lua scripts
	*/
	LIBEXP typedef struct CollisionEvent_L {
		entity collider;
		int num_collision_points;
		vec3 collision_points[4];
	};

}


/**
 * @brief A LuaEngineObjectDelegate specialization, which provides an interface between the C++ CollisionEvent_L type and its Lua counterpart.
*/
template<>
class LuaEngineObjectDelegate<CollisionEvent_L> {
public:

	/** 
	 * @brief An interface which converts the C++ structure to a Lua representation.
	 * @param proxy an object which exposes methods to build out the Lua representation of an the specialized structure.
	 * @param value the C++ object to convert
	*/
	static void SetObject(LuaEngineProxy proxy, const CollisionEvent_L& value) {
		proxy.SetTable([&value](LuaEngineProxy proxy) {
			proxy.SetTableItem<int>(value.collider.id, "id");
			}, "collider");

		proxy.SetTableItem((int)value.num_collision_points, "num_collision_points");
		proxy.SetTable([&value](LuaEngineProxy proxy) {
			for (int i = 0; i < std::min(value.num_collision_points,4); i++) {
				proxy.SetTableItem(value.collision_points[i], i);
			}
			}
		, "collision_points");
	}

	/**
	 * @brief An interface which converts the Lua representation of a structure to a C++ structure.
	 * @param proxy an object which exposes methods read out the Lua structure representation from the Lua state
	 * @param index The index at which the table with the structure is located in the Lua stack. 
	 * (Since these structures are only used as return values, they are always on top of the stack when being fetched, so this index isn't used.)
	 * @return an instance of the C++ structure
	 * 
	 * @bug If this function gets used to fetch something at index other than -1, it will not function properly. LuaEngineProxy::GetTable doesn't have the index parameter.
	*/
	static CollisionEvent_L GetObject(LuaEngineProxy proxy, int index = -1) {
		CollisionEvent_L col_event;

		proxy.GetTable([&col_event](LuaEngineProxy proxy) {
			col_event.collider = entity{ (uint32_t)proxy.GetTableField<int>("id") };
			}, "collider");

		col_event.num_collision_points = proxy.GetTableField<int>("num_collision_points");
		proxy.GetTable([&col_event](LuaEngineProxy proxy) {
			for (int i = 0; i < std::min(col_event.num_collision_points, 4); i++) {
				col_event.collision_points[i] = proxy.GetTableField<vec3>(i);
			}
			}, "collision_points");

	}
};

/**
 * @brief Provides Lua Scripts with a definition for @ref CollisionEvent_L. 
 * 
 * @note This module depends on:
 * - @ref GlobalEntityModule
 * - @ref MathModule
*/
class CollisionModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("CollisionModule");
	/**
	 * @brief Provides the bindings with the functionality of this module. 
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};