#pragma once 
#include <LuaEngine.h>

extern "C" {
	typedef struct vec2 { float x, y; } vec2;
	typedef struct vec3 { float x, y, z; } vec3;
	typedef struct vec4 { float x, y, z, w; } vec4;
	typedef struct quat { float w, x, y, z; } quat;
	typedef struct mat3 { vec3 data[3]; } mat3;
	typedef struct mat4 { vec4 data[4]; } mat4;
}


template<>
class LuaEngineObjectDelegate<vec4> {
public:

	static void SetObject(LuaEngineProxy proxy, const vec4& value) {
		proxy.SetTableItem((float)value.x, "x");
		proxy.SetTableItem((float)value.y, "y");
		proxy.SetTableItem((float)value.z, "z");
		proxy.SetTableItem((float)value.w, "w");
	}

	static vec4 GetObject(LuaEngineProxy proxy, int index = -1) {
		return vec4{ proxy.GetTableField<float>("x", index),proxy.GetTableField<float>("y", index),proxy.GetTableField<float>("z", index),proxy.GetTableField<float>("w", index) };
	}
};


template<>
class LuaEngineObjectDelegate<vec3> {
public:

	static void SetObject(LuaEngineProxy proxy, const vec3& value) {
		proxy.SetTableItem((float)value.x, "x");
		proxy.SetTableItem((float)value.y, "y");
		proxy.SetTableItem((float)value.z, "z");
	}

	static vec3 GetObject(LuaEngineProxy proxy, int index = -1) {
		return vec3{ proxy.GetTableField<float>("x", index),proxy.GetTableField<float>("y", index),proxy.GetTableField<float>("z", index)};
	}
};


template<>
class LuaEngineObjectDelegate<vec2> {
public:

	static void SetObject(LuaEngineProxy proxy, const vec2& value) {
		proxy.SetTableItem((float)value.x, "x");
		proxy.SetTableItem((float)value.y, "y");
	}

	static vec2 GetObject(LuaEngineProxy proxy, int index = -1) {
		return vec2{ proxy.GetTableField<float>("x", index),proxy.GetTableField<float>("y", index)};
	}
};

class MathModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("MathModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};