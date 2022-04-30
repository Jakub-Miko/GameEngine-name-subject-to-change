#include "MathModule.h"
#include <glm/glm.hpp>

extern "C" {
	vec3 multiply_matrix_vec3_L(mat3 matrix, vec3* vector) {
		glm::mat3* mat = reinterpret_cast<glm::mat3*>(matrix);
		glm::vec3* vec = reinterpret_cast<glm::vec3*>(vector);
		glm::vec3 result = *mat * *vec;
		return *reinterpret_cast<vec3*>(&result);
	}

	vec4 multiply_matrix_vec4_L(mat4 matrix, vec4* vector) {
		glm::mat4* mat = reinterpret_cast<glm::mat4*>(matrix);
		glm::vec4* vec = reinterpret_cast<glm::vec4*>(vector);
		glm::vec4 result = *mat * *vec;
		return *reinterpret_cast<vec4*>(&result);
	}
}

void MathModule::OnRegisterModule(ModuleBindingProperties& props)
{
	props.Add_FFI_declarations(R"(
	typedef struct vec2 { float x, y; } vec2;
	typedef struct vec3 { float x, y, z; } vec3;
	typedef struct vec4 { float x, y, z, w; } vec4;
	typedef vec3 mat3[3];
	typedef vec4 mat4[4];
	
	vec3 multiply_matrix_vec3_L(mat3 matrix, vec3* vector);
	vec4 multiply_matrix_vec4_L(mat4 matrix, vec4* vector);
	)");

	props.Add_FFI_aliases({
		{"struct vec2", "vec2"},
		{"struct vec3", "vec3"},
		{"struct vec4", "vec4"},
		{"array mat3", "mat3"},
		{"array mat4", "mat4"},
		{"multiply_matrix_vec3_L","multiply_matrix_vec3"},
		{"multiply_matrix_vec4_L","multiply_matrix_vec4"}
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
	return vec4({x = num1.x + num2.x,y = num1.y + num2.y}) 
	end

	function subtract_vec3(num1, num2)
	return vec3({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z}) 
	end

	function subtract_vec4(num1, num2)
	return vec4({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z,w = num1.w - num2.w}) 
	end

	function subtract_vec2(num1, num2)
	return vec4({x = num1.x - num2.x,y = num1.y - num2.y}) 
	end

	function multiply_vec3(num1, num2)
	return vec3({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z}) 
	end

	function multiply_vec4(num1, num2)
	return vec4({x = num1.x * num2.x,y = num1.y * num2.y, z = num1.z * num2.z,w = num1.w * num2.w}) 
	end

	function multiply_vec2(num1, num2)
	return vec4({x = num1.x * num2.x,y = num1.y * num2.y}) 
	end

	function divide_vec3(num1, num2)
	return vec3({x = num1.x - num2.x,y = num1.y - num2.y, z = num1.z - num2.z}) 
	end

	function divide_vec4(num1, num2)
	return vec4({x = num1.x / num2.x,y = num1.y / num2.y, z = num1.z / num2.z,w = num1.w / num2.w}) 
	end

	function divide_vec2(num1, num2)
	return vec4({x = num1.x / num2.x,y = num1.y / num2.y}) 
	end

	mt_vec3 = {
	__add = add_vec3,
	__sub = subtract_vec3,
	__mul = multiply_vec3,
	__div = divide_vec3
	}

	mt_vec4 = {
	__add = add_vec4,
	__sub = subtract_vec4,
	__mul = multiply_vec4,
	__div = divide_vec4
	}

	mt_vec2 = {
	__add = add_vec2,
	__sub = subtract_vec2,
	__mul = multiply_vec2,
	__div = divide_vec2
	}	

	ffi.metatype("vec4", mt_vec4)
	
	ffi.metatype("vec3", mt_vec3)	

	ffi.metatype("vec2", mt_vec2)

	

	)");
}
