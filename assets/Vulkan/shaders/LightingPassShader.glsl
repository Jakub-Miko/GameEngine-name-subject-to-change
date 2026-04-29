/*
#RootSignature
{
	"RootSignature": [
		{
			"type" : "push_constants",
			"size" : 68
		},
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
*/
//#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec3 position;

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
	uint light_id;
};

layout(set = 0, binding = 0) uniform conf {
	mat4 inverse_projection;
	mat4 projection_matrix;
	float depth_constant_a;
	float depth_constant_b;
	vec2 pixel_size;
};

struct LightData {
	vec4 light_color;
	float range;
	uint type;
};

layout(set = 0, binding = 1) readonly buffer light_data {
	LightData lights[];
};

out vec3 light_volume_pos;
out vec3 light_pos;
out vec3 light_direction_in;

void main() {
	LightData light = lights[light_id];
	if (light.type == 0) {
		gl_Position = vec4(position, 1.0);
		light_volume_pos = vec3(inverse_projection * vec4(position.xy, -1.0, 1.0));
		light_pos = vec3(mv_matrix[3]);
		light_direction_in = normalize(mat3(mv_matrix) * vec3(0.0, 0.0, -1.0));
	}
	else {
		gl_Position = projection_matrix * mv_matrix * vec4(position, 1.0);
		light_volume_pos = vec3(mv_matrix * vec4(position, 1.0));
		light_pos = vec3(mv_matrix[3]);
		light_direction_in = normalize(mat3(mv_matrix) * vec3(0.0, 0.0, -1.0));
	}
	
}


//#end
// #Fragment //------------------------------------------------
#version 430

layout(location = 0) out vec4 color_out;

layout(set = 1, binding = 0) uniform sampler2D Color;
layout(set = 1, binding = 1) uniform sampler2D Normal;
layout(set = 1, binding = 2) uniform sampler2D Material;
layout(set = 1, binding = 3) uniform sampler2D DepthBuffer;

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
	uint light_id;
};

layout(set = 0, binding = 0) uniform conf {
	mat4 inverse_projection;
	mat4 projection_matrix;
	float depth_constant_a;
	float depth_constant_b;
	vec2 pixel_size;
};

struct LightData {
	vec4 light_color;
	float range;
	uint type;
};

layout(set = 0, binding = 1) readonly buffer light_data {
	LightData lights[];
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
#include <shaders/utils/PBR.glsl>

void main() {
	LightData light = lights[light_id];
	vec3 coords = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 view_space_pos = GetFragmentPosition(coords);
	vec3 light_direction;
	if (light.type == 0) {
		light_direction = light_direction_in;
	}
	else {
		light_direction = -normalize(light_pos - view_space_pos);
	}


	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	vec3 normal = UnpackNormals(texture(Normal, coords.xy).xy);
	vec4 material = texture(Material, coords.xy);
	float roughness = material.y;
	float metallic = material.z;

	vec3 light_radiance = light.light_color.xyz * light.light_color.w;
	float distance = length(light_pos - view_space_pos);
	light_radiance *= light.type == 1 ? PointAttenuationFalloff(distance, light.range) : 1.0f;

	color_out = vec4(CookTorranceModel(-light_direction, -normalize(view_space_pos), normal,
									   color.xyz, roughness, metallic) * light_radiance, 1.0);
}

//#end
