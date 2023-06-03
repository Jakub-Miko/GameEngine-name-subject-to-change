#include "MathModule.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Core/Defines.h>


extern "C" {
	LIBEXP vec3 multiply_matrix_vec3_L(mat3* matrix, vec3* vector) {
		glm::mat3* mat = reinterpret_cast<glm::mat3*>(matrix);
		glm::vec3* vec = reinterpret_cast<glm::vec3*>(vector);
		glm::vec3 result = *mat * *vec;
		return *reinterpret_cast<vec3*>(&result);
	}

	LIBEXP float GetVector2Length_L(vec2 in_vector) {
		glm::vec2 vec = *reinterpret_cast<glm::vec2*>(&in_vector);
		return glm::length(vec);
	}

	LIBEXP float GetVector3Length_L(vec3 in_vector) {
		glm::vec3 vec = *reinterpret_cast<glm::vec3*>(&in_vector);
		return glm::length(vec);
	}

	LIBEXP float GetVector4Length_L(vec4 in_vector) {
		glm::vec4 vec = *reinterpret_cast<glm::vec4*>(&in_vector);
		return glm::length(vec);
	}

	LIBEXP vec4 multiply_matrix_vec4_L(mat4* matrix, vec4* vector) {
		glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix);
		glm::vec4* vec = reinterpret_cast<glm::vec4*>(vector);
		glm::vec4 result = *mat * *vec;
		return *reinterpret_cast<vec4*>(&result);
	}

	LIBEXP quat mat3_to_quat_L(mat3* matrix) {
		glm::mat3* mat = reinterpret_cast<glm::mat3*>(matrix);
		glm::quat quaternion = glm::quat_cast(*mat);
		return *reinterpret_cast<quat*>(&quaternion);
	}

	LIBEXP mat3 multiple_mat3_L(mat3* first, mat3* second) {
		glm::mat3* fr = reinterpret_cast<glm::mat3*>(first);
		glm::mat3* sc = reinterpret_cast<glm::mat3*>(second);
		glm::mat3 result = *fr * *sc;
		return *reinterpret_cast<mat3*>(&result);
	}

	LIBEXP quat quat_lookat_L(vec3* direction, vec3* up_vector) {
		glm::vec3* direction_v = reinterpret_cast<glm::vec3*>(direction);
		glm::vec3* up_vector_v = reinterpret_cast<glm::vec3*>(up_vector);
		glm::quat result = glm::quatLookAt(glm::normalize(*direction_v), *up_vector_v);
		return *reinterpret_cast<quat*>(&result);
	}

	LIBEXP mat4 multiple_mat4_L(mat4* first, mat4* second) {
		glm::mat4* fr = reinterpret_cast<glm::mat4*>(first);
		glm::mat4* sc = reinterpret_cast<glm::mat4*>(second);
		glm::mat4 result = *fr * *sc;
		return *reinterpret_cast<mat4*>(&result);
	}

	LIBEXP vec3 rotate_vec3_L(vec3 vec, quat rotation) {
		glm::quat quaternion = *reinterpret_cast<glm::quat*>(&rotation);
		glm::vec3 vec_to_rotate = *reinterpret_cast<glm::vec3*>(&vec);
		vec_to_rotate = glm::toMat3(quaternion) * vec_to_rotate;
		return *reinterpret_cast<vec3*>(&vec_to_rotate);
	}

	LIBEXP quat quat_multiply_L(quat quat1, quat quat2) {
		glm::quat quaternion1 = *reinterpret_cast<glm::quat*>(&quat1);
		glm::quat quaternion2 = *reinterpret_cast<glm::quat*>(&quat2);
		glm::quat result = quaternion1 * quaternion2;
		return *reinterpret_cast<quat*>(&result);
	}

	LIBEXP vec4 MixVector4_L(vec4 first, vec4 second, float blend) {
		glm::vec4* vec_1 = reinterpret_cast<glm::vec4*>(&first);
		glm::vec4* vec_2 = reinterpret_cast<glm::vec4*>(&second);
		glm::vec4 mix = glm::mix(*vec_1, *vec_2, blend);
		return *reinterpret_cast<vec4*>(&mix);
	}

	LIBEXP vec3 MixVector3_L(vec3 first, vec3 second, float blend) {
		glm::vec3* vec_1 = reinterpret_cast<glm::vec3*>(&first);
		glm::vec3* vec_2 = reinterpret_cast<glm::vec3*>(&second);
		glm::vec3 mix = glm::mix(*vec_1, *vec_2, blend);
		return *reinterpret_cast<vec3*>(&mix);
	}

	LIBEXP vec2 MixVector2_L(vec2 first, vec2 second, float blend) {
		glm::vec2* vec_1 = reinterpret_cast<glm::vec2*>(&first);
		glm::vec2* vec_2 = reinterpret_cast<glm::vec2*>(&second);
		glm::vec2 mix = glm::mix(*vec_1, *vec_2, blend);
		return *reinterpret_cast<vec2*>(&mix);
	}

}

void MathModule::OnRegisterModule(ModuleBindingProperties& props)
{
	props.Add_FFI_declarations(R"(
	typedef struct vec2 { float x, y; } vec2;
	typedef struct vec3 { float x, y, z; } vec3;
	typedef struct vec4 { float x, y, z, w; } vec4;
	typedef struct quat { float w, x, y, z; } quat;
	typedef struct mat3 { vec3 data[3]; } mat3;
	typedef struct mat4 { vec4 data[4]; } mat4;
	
	vec3 multiply_matrix_vec3_L(mat3 matrix, vec3* vector);
	vec4 multiply_matrix_vec4_L(mat4 matrix, vec4* vector);
	mat4 multiple_mat4_L(mat4* first, mat4* second);
	mat3 multiple_mat3_L(mat3* first, mat3* second);
	quat quat_lookat_L(vec3* direction, vec3* up_vector);
	quat mat3_to_quat_L(mat3 matrix);
	vec3 rotate_vec3_L(vec3 vec, quat rotation);
	quat quat_multiply_L(quat quat1, quat quat2);
	float GetVector2Length_L(vec2 in_vector);
	float GetVector3Length_L(vec3 in_vector);
	float GetVector4Length_L(vec4 in_vector);
	vec4 MixVector4_L(vec4 first, vec4 second, float blend);
	vec3 MixVector3_L(vec3 first, vec3 second, float blend);
	vec2 MixVector2_L(vec2 first, vec2 second, float blend);
	)");

	props.Add_FFI_aliases({
		{"struct vec2", "vec2"},
		{"struct vec3", "vec3"},
		{"struct vec4", "vec4"},
		{"struct quat", "quat"},
		{"struct mat3", "mat3"},
		{"struct mat4", "mat4"},
		{"multiply_matrix_vec3_L","multiply_matrix_vec3"},
		{"multiply_matrix_vec4_L","multiply_matrix_vec4"},
		{"multiple_mat4_L","multiple_mat4"},
		{"multiple_mat3_L","multiple_mat3"},
		{"quat_lookat_L","quat_lookat"},
		{"mat3_to_quat_L","mat3_to_quat"},
		{"rotate_vec3_L","rotate_vec3"},
		{"quat_multiply_L","quat_multiply"},
		{"GetVector2Length_L","GetVector2Length"},
		{"GetVector3Length_L","GetVector3Length"},
		{"GetVector4Length_L","GetVector4Length"},
		{"MixVector4_L", "MixVector4"},
		{"MixVector3_L", "MixVector3"},
		{"MixVector2_L", "MixVector2"}
		});

	props.Add_init_script(R"(

	function to_vec2(vec)
	return vec2({vec.x, vec.y})
	end
	
	function to_vec3(vec)
	return vec3({vec.x, vec.y,vec.z})
	end
	
	function to_vec4(vec)
	return vec4({vec.x, vec.y,vec.z,vec.w})
	end
	
	function add_vec3(num1, num2)
	return vec3({x = num1.x + num2.x,y = num1.y + num2.y, z = num1.z + num2.z}) 
	end

	function add_vec4(num1, num2)
	return vec4({x = num1.x + num2.x,y = num1.y + num2.y, z = num1.z + num2.z,w = num1.w + num2.w}) 
	end

	function add_vec2(num1, num2)
	return vec2({x = num1.x + num2.x,y = num1.y + num2.y}) 
	end

	function subtract_vec3(num1, num2)
	return vec3({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z}) 
	end

	function subtract_vec4(num1, num2)
	return vec4({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z,w = num1.w - num2.w}) 
	end

	function subtract_vec2(num1, num2)
	return vec2({x = num1.x - num2.x,y = num1.y - num2.y}) 
	end

	function multiply_vec3(num1, num2)
	return vec3({x = num1.x * num2.x,y = num1.y * num2.y, z = num1.z * num2.z}) 
	end

	function multiply_vec4(num1, num2)
	return vec4({x = num1.x * num2.x,y = num1.y * num2.y, z = num1.z * num2.z,w = num1.w * num2.w}) 
	end

	function multiply_vec2(num1, num2)
	return vec2({x = num1.x * num2.x,y = num1.y * num2.y}) 
	end

	function divide_vec3(num1, num2)
	return vec3({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z}) 
	end

	function divide_vec4(num1, num2)
	return vec4({x = num1.x / num2.x,y = num1.y / num2.y, z = num1.z / num2.z,w = num1.w / num2.w}) 
	end

	function divide_vec2(num1, num2)
	return vec2({x = num1.x / num2.x,y = num1.y / num2.y}) 
	end

	function normalizeVector3(num1)
	local length = GetVector3Length(num1)
	if(length == 0.0) then return vec3(0.0,0.0,0.0) end
	return vec3(num1.x/length,num1.y/length,num1.z/length) 
	end

	function normalizeVector4(num1)
	local length = GetVector4Length(num1)
	if(length == 0.0) then return vec4(0.0,0.0,0.0,0.0) end
	return vec4(num1.x/length,num1.y/length,num1.z/length,num1.w/length) 
	end

	function normalizeVector2(num1)
	local length = GetVector2Length(num1)
	if(length == 0.0) then return vec2(0.0,0.0) end
	return vec2(num1.x/length,num1.y/length) 
	end

	

	mt_vec3 = {
	__add = add_vec3,
	__sub = subtract_vec3,
	__mul = multiply_vec3,
	__div = divide_vec3,
	__unm = function(value) return vec3({-value.x,-value.y,-value.z}) end
	}

	mt_vec4 = {
	__add = add_vec4,
	__sub = subtract_vec4,
	__mul = multiply_vec4,
	__div = divide_vec4,
	__unm = function(value) return vec3({-value.x,-value.y,-value.z,-value.w}) end
	}

	mt_vec2 = {
	__add = add_vec2,
	__sub = subtract_vec2,
	__mul = multiply_vec2,
	__div = divide_vec2,
	__unm = function(value) return vec3({-value.x,-value.y}) end
	}	

	mt_mat3 = {
	__index = function(table,key) return table.data[key] end,
	__newindex = function(table,key,value) table.data[key] = value end,
	__mul = multiple_mat3,
	__new = function(ctype, data) return ffi.new("struct mat3",{data}) end
	}

	mt_mat4 = {
	__index = function(table,key) return table.data[key] end,
	__newindex = function(table,key,value) table.data[key] = value end,
	__mul = multiple_mat4,
	__new = function(ctype, data) return ffi.new("struct mat4",{data}) end
	}


	ffi.metatype("vec4", mt_vec4)
	
	ffi.metatype("vec3", mt_vec3)	

	ffi.metatype("vec2", mt_vec2)

	ffi.metatype("mat3", mt_mat3)
	
	ffi.metatype("mat4", mt_mat4)
	

	)");
}
