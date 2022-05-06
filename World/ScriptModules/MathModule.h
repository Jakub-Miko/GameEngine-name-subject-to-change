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



class MathModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("MathModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};