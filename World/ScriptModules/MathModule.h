#pragma once 
#include <LuaEngine.h>

extern "C" {
	typedef struct vec2 { float x, y; } vec2;
	typedef struct vec3 { float x, y, z; } vec3;
	typedef struct vec4 { float x, y, z, w; } vec4;
	typedef vec3 mat3[3];
	typedef vec4 mat4[4];
}



class MathModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("MathModule");
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};