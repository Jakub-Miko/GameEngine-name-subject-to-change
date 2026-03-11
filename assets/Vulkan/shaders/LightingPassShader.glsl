#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
		{
			"name" : "LightingPassLightMaterial",
			"type" : "material",
			"material_inline": [
					{
						"name" : "Light_Color",
						"type" : "VEC4",
						"value" : {
							"x" : 1.0,
							"y" : 1.0,
							"z" : 1.0,
							"w" : 1.0
						}
					},
					{
						"name" : "pixel_size",
						"type" : "VEC2"
					},
					{
						"name" : "range",
						"type" : "SCALAR"
					},
					{
						"name" : "light_type",
						"type" : "INT",
						"value" : 1
					}
			]
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

layout(location = 0) in vec2 position;

layout(set = 0, binding = 0) uniform conf {
	mat4 mvp_matrix;
	mat4 view_model_matrix;
	mat4 inverse_projection;
	float depth_constant_a;
	float depth_constant_b;
};

layout(set = 1, binding = 0) uniform light_props{
	vec4 Light_Color;
	vec2 pixel_size;
	float range;
	int light_type;
};

out vec3 light_volume_pos;
out vec3 light_pos;
out vec3 light_direction_in;

void main() {
	if (light_type == 0) {
		gl_Position = vec4(position, 1.0);
		light_volume_pos = vec3(inverse_projection * vec4(position.xy, -1.0, 1.0));
		light_pos = vec3(view_model_matrix[3]);
		light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
	}
	else {
		gl_Position = mvp_matrix * vec4(position, 1.0);
		light_volume_pos = vec3(view_model_matrix * vec4(position, 1.0));
		light_pos = vec3(view_model_matrix[3]);
		light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
	}
	
}


#end
#Fragment //------------------------------------------------
#version 430

layout(location = 0) out vec4 color_out;

layout(set = 2, binding = 0) uniform sampler2D Color;
layout(set = 2, binding = 1) uniform sampler2D Normal;
layout(set = 2, binding = 2) uniform sampler2D Roughness;
layout(set = 2, binding = 3) uniform sampler2D DepthBuffer;

layout(set = 0, binding = 0) uniform conf {
	mat4 mvp_matrix;
	mat4 view_model_matrix;
	mat4 inverse_projection;
	float depth_constant_a;
	float depth_constant_b;
};

layout(set = 1, binding = 0) uniform light_props{
	vec4 Light_Color;
	vec2 pixel_size;
	float range;
	int light_type;
};

in vec3 light_volume_pos;
in vec3 light_pos;
in vec3 light_direction_in;

vec3 GetFragmentPosition(vec3 coordinates) {
	vec3 dir = vec3(light_volume_pos.xy / abs(light_volume_pos.z), -1.0);
	float depth = texture(DepthBuffer, coordinates.xy).x;

	float linearized_depth = depth_constant_b / (depth - depth_constant_a);

	return dir * linearized_depth;
}

#include <shaders/utils/NormalPacking.glsl>
#include <shaders/utils/PointAttenuationFalloff.glsl>

void main() {
	vec3 coords = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 view_space_pos = GetFragmentPosition(coords);
	vec3 light_direction;
	if (light_type == 0) {
		light_direction = light_direction_in;
	}
	else {
		light_direction = -normalize(light_pos - view_space_pos);
	}


	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	vec3 normal = UnpackNormals(texture(Normal, coords.xy).xy);
	float roughness = texture(Roughness, coords.xy).x;
	float attenuation_factor = 1;

	if (light_type == 1) {
		float distance = length(light_pos - view_space_pos);
		attenuation_factor *= PointAttenuationFalloff(distance, range);
	}

	float diffuse_contribution = 0.5f * (0.1 + max(0, dot(normal, -light_direction)));
	float specular_contribution = 0.5f * pow(clamp(dot(normal, (-light_direction + vec3(0,0,-1))/2.0 ),0,1), 1+((1-roughness)*32));

	float contribution = diffuse_contribution + specular_contribution;

	color_out = vec4(color.xyz * Light_Color.xyz * attenuation_factor * Light_Color.w * contribution, 1.0);
}

#end
