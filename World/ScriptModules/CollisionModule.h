#pragma once 
#include <LuaEngine.h>
#include <World/ScriptModules/MathModule.h>
#include <World/ScriptModules/GlobalEntityModule.h>

extern "C" {
	typedef struct CollisionEvent_L {
		entity collider;
		int num_collision_points;
		vec3 collision_points[4];
	};

}

template<>
class LuaEngineObjectDelegate<CollisionEvent_L> {
public:

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

class CollisionModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("CollisionModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};