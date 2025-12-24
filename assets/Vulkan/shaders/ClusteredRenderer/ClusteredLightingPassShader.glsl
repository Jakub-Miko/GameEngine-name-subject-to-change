#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
		{
			"name" : "light_buffer",
			"type" : "storage_buffer"
		},
		{
			"name" : "GBufferMaterial",
			"type" : "material",
			"material_path": "api:GBufferMaterialLayout.json"
		}
	]
	
}
#end

#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

struct Light {
	mat4 view_model_matrix;
	vec4 Light_Color;
	vec4 attenuation_constants;
	int light_type;
};

layout(set = 0, binding = 0) uniform conf {
	mat4 projection_matrix;
	vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	int light_count;
};
layout(set = 0, binding = 1) readonly buffer light_buffer {
	Light lights[];
};

out vec3 light_volume_pos;

void main() {
//	if (light_type == 0) {
//		gl_Position = vec4(position, 1.0);
//		light_volume_pos = vec3(inverse(projection_matrix) * vec4(position.xy, -1.0, 1.0));
//		light_pos = vec3(view_model_matrix[3]);
//		light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
//	}
//	else {
//		gl_Position = projection_matrix * view_model_matrix * vec4(position, 1.0);
//		light_volume_pos = vec3(view_model_matrix * vec4(position, 1.0));
//		light_pos = vec3(view_model_matrix[3]);
//		light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
//	}

	gl_Position = vec4(position, 1.0);
	light_volume_pos = vec3(inverse(projection_matrix) * vec4(position.xy, -1.0, 1.0));

}


#end
#Fragment //------------------------------------------------
#version 430

layout(location = 0) out vec4 color_out;

layout(set = 1, binding = 0) uniform sampler2D Color;
layout(set = 1, binding = 1) uniform sampler2D Normal;
layout(set = 1, binding = 2) uniform sampler2D Roughness;
layout(set = 1, binding = 3) uniform sampler2D DepthBuffer;

struct Light {
	mat4 view_model_matrix;
	vec4 Light_Color;
	vec4 attenuation_constants;
	int light_type;
};

layout(set = 0, binding = 0) uniform conf {
	mat4 projection_matrix;
	vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	int light_count;
};

layout(set = 0, binding = 1) readonly buffer light_buffer {
	Light lights[];
};

in vec3 light_volume_pos;

vec3 GetFragmentPosition(vec3 coordinates) {
	vec3 dir = vec3(light_volume_pos.xy / abs(light_volume_pos.z), -1.0);
	float depth = texture(DepthBuffer, coordinates.xy).x;

	float linearized_depth = depth_constant_b / (depth - depth_constant_a);

	return dir * linearized_depth;
}


void main() {
	vec3 coords = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 view_space_pos = GetFragmentPosition(coords);
	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	vec3 normal = texture(Normal, coords.xy).xyz;
	float roughness = texture(Roughness, coords.xy).x;
	vec3 color_accum = vec3(0.0);

	for(int i = 0; i < light_count; i++) {
		vec3 light_direction;
		if (lights[i].light_type == 0) {
			light_direction = normalize(mat3(lights[i].view_model_matrix) * vec3(0.0, 0.0, - 1.0));
		}
		else {
			light_direction = - normalize(vec3(lights[i].view_model_matrix[3]) - view_space_pos);
		}

		float attenuation_factor = 1;

		if (lights[i].light_type == 1) {
			float distance = length(vec3(lights[i].view_model_matrix[3]) - view_space_pos);
			vec3 attenuation_constants = lights[i].attenuation_constants.xyz;
			attenuation_factor = 1.0 / (attenuation_constants.x + (attenuation_constants.y * distance) + attenuation_constants.z * (distance * distance));
		}

		float diffuse_contribution = 0.5f * (0.1 + max(0, dot(normal, - light_direction)));
		float specular_contribution = 0.5f * pow(clamp(dot(normal, (- light_direction + vec3(0, 0, - 1)) /2.0), 0, 1), 1 +((1 - roughness) * 32));

		float contribution = diffuse_contribution + specular_contribution;

		vec4 Light_Color = lights[i].Light_Color;
		color_accum += vec3(color.xyz * Light_Color.xyz * attenuation_factor * Light_Color.w * contribution);
	}

	color_out = vec4(color_accum, 1.0);
}

#end
