#pragma once 
#include <LuaEngine.h>
#include <Core/Defines.h>

extern "C" {
	/**
	 * @brief A class representing a 2 component vector in Lua scripts.
	 */
	LIBEXP typedef struct vec2 { float x, y; } vec2;
	/**
	 * @brief A class representing a 3 component vector in Lua scripts.
	 */
	LIBEXP typedef struct vec3 { float x, y, z; } vec3;
	/**
	 * @brief A class representing a 4 component vector in Lua scripts.
	 */
	LIBEXP typedef struct vec4 { float x, y, z, w; } vec4;
	/**
	 * @brief A class representing a quaternion in Lua scripts.
	 */
	LIBEXP typedef struct quat { float w, x, y, z; } quat;
	/**
	 * @brief A class representing a 3x3 matrix in Lua scripts.
	 */
	LIBEXP typedef struct mat3 { vec3 data[3]; } mat3;
	/**
	 * @brief A class representing a 4x4 matrix in Lua scripts.
	 */
	LIBEXP typedef struct mat4 { vec4 data[4]; } mat4;
}

/**
 * @brief A LuaEngineObjectDelegate specialization, which provides an interface between the C++ vec4 type and its Lua counterpart.
*/
template<>
class LuaEngineObjectDelegate<vec4> {
public:

	/**
	 * @brief An interface which converts the C++ structure to a Lua representation.
	 * @param proxy an object which exposes methods to build out the Lua representation of an the specialized structure.
	 * @param value the C++ object to convert
	*/
	static void SetObject(LuaEngineProxy proxy, const vec4& value) {
		proxy.SetTableItem((float)value.x, "x");
		proxy.SetTableItem((float)value.y, "y");
		proxy.SetTableItem((float)value.z, "z");
		proxy.SetTableItem((float)value.w, "w");
	}

	/**
	 * @brief An interface which converts the Lua representation of a structure to a C++ structure.
	 * @param proxy an object which exposes methods read out the Lua structure representation from the Lua state
	 * @param index The index at which the table with the structure is located in the Lua stack.
	 * @return an instance of the C++ structure
	*/
	static vec4 GetObject(LuaEngineProxy proxy, int index = -1) {
		return vec4{ proxy.GetTableField<float>("x", index),proxy.GetTableField<float>("y", index),proxy.GetTableField<float>("z", index),proxy.GetTableField<float>("w", index) };
	}
};

/**
 * @brief A LuaEngineObjectDelegate specialization, which provides an interface between the C++ vec3 type and its Lua counterpart.
*/
template<>
class LuaEngineObjectDelegate<vec3> {
public:

	/**
	 * @brief An interface which converts the C++ structure to a Lua representation.
	 * @param proxy an object which exposes methods to build out the Lua representation of an the specialized structure.
	 * @param value the C++ object to convert
	*/
	static void SetObject(LuaEngineProxy proxy, const vec3& value) {
		proxy.SetTableItem((float)value.x, "x");
		proxy.SetTableItem((float)value.y, "y");
		proxy.SetTableItem((float)value.z, "z");
	}

	/**
	 * @brief An interface which converts the Lua representation of a structure to a C++ structure.
	 * @param proxy an object which exposes methods read out the Lua structure representation from the Lua state
	 * @param index The index at which the table with the structure is located in the Lua stack.
	 * @return an instance of the C++ structure
	*/
	static vec3 GetObject(LuaEngineProxy proxy, int index = -1) {
		return vec3{ proxy.GetTableField<float>("x", index),proxy.GetTableField<float>("y", index),proxy.GetTableField<float>("z", index)};
	}
};

/**
 * @brief A LuaEngineObjectDelegate specialization, which provides an interface between the C++ vec2 type and its Lua counterpart.
*/
template<>
class LuaEngineObjectDelegate<vec2> {
public:

	/** 
	 * @brief An interface which converts the C++ structure to a Lua representation.
	 * @param proxy an object which exposes methods to build out the Lua representation of an the specialized structure.
	 * @param value the C++ object to convert
	*/
	static void SetObject(LuaEngineProxy proxy, const vec2& value) {
		proxy.SetTableItem((float)value.x, "x");
		proxy.SetTableItem((float)value.y, "y");
	}

	/**
	 * @brief An interface which converts the Lua representation of a structure to a C++ structure.
	 * @param proxy an object which exposes methods read out the Lua structure representation from the Lua state
	 * @param index The index at which the table with the structure is located in the Lua stack.
	 * @return an instance of the C++ structure
	*/
	static vec2 GetObject(LuaEngineProxy proxy, int index = -1) {
		return vec2{ proxy.GetTableField<float>("x", index),proxy.GetTableField<float>("y", index)};
	}
};


/**
 * @brief Provides Lua Scripts with functionality to perform mathematical operations and access vector and matrix types.
 * 
 * For the functionality provided see MathModule.cpp.
 * 
 * ## Additional lua functionality
 * 
 * This lua functionality has been implemented purely in lua and was as such not auto documented by doxygen
 * 
 * ### Operators
 * All vector types contain a component wise addition, multiplication, subtraction and division operators
 * Matrices contain indexing operators to return columns as vectors. Individual vectors can have componets accessed through the x, y, z and w members
 * Matricies also provide a multiplication operator implemented by @ref multiple_mat4_L and @ref multiple_mat3_L
 * 
 * 
 * ### to_vec2(vec)
 * Converts vectors with more components to vec2
 * 
 * 
 * ### to_vec3(vec)
 * Converts vectors with more components to vec3
 * 
 * 
 * ### to_vec4(vec)
 * Doesn't have much purpose and just returns a copy of a vec4
 * 
 * 
 * ### normalizeVector2(vec)
 * Normalizes a vec2
 * 
 * 
 * ### normalizeVector3(vec)
 * Normalizes a vec3
 * 
 * 
 * ### normalizeVector4(vec)
 * Normalizes a vec4
 * 
 * 
*/
class MathModule : public ScriptModule {
public:
	SCRIPT_MODULE_NAME("MathModule");

	/**
	 * @brief Provides the bindings with the functionality of this module. 
	 * @param props ModuleBindingProperties to which the bindings and definitions of this module are written.
	*/
	virtual void OnRegisterModule(ModuleBindingProperties& props) override;
};