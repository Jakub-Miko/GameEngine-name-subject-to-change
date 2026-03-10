/*
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
			"name" : "light_assignment_buffer",
			"type" : "storage_buffer"
		},
		{
			"name" : "cluster_buffer",
			"type" : "storage_buffer"
		},
		{
			"name" : "GBufferMaterial",
			"type" : "material",
			"material_path": "api:GBufferMaterialLayout.json"
		},
		{
			"name" : "point_light_shadow_maps",
			"type" : "resource_store",
			"store_type" : "texture_2D_cubemap"
		},
		{
			"name" : "directional_light_shadow_maps",
			"type" : "resource_store",
			"store_type" : "texture_2D_array"
		}
	]

}
#end
*/
// #Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform conf {
	mat4 projection_matrix;
	vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	int light_count;
};

out vec2 light_volume_pos;

void main() {
	gl_Position = vec4(position, 1.0);
	vec3 light_pos = vec3(inverse(projection_matrix) * vec4(position.xy, -1.0, 1.0));
	light_volume_pos = light_pos.xy / abs(light_pos.z);
}

// #end
// #Fragment //------------------------------------------------
#version 430

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec4 color_out;

layout(set = 1, binding = 0) uniform sampler2D Color;
layout(set = 1, binding = 1) uniform sampler2D Normal;
layout(set = 1, binding = 2) uniform sampler2D Roughness;
layout(set = 1, binding = 3) uniform sampler2D DepthBuffer;

layout(set = 2, binding = 0) uniform samplerCubeShadow PointShadowMaps[];
layout(set = 3, binding = 0) uniform sampler2DArrayShadow DirectionalShadowMaps[];

struct Light {
	mat4 light_matrix;
	vec4 position_or_direction_and_radius;
	vec4 Light_Color;
	float range;
	int light_type;
	uint shadow_index;
	float light_far_plane;
};

layout(set = 0, binding = 0) uniform conf {
	mat4 projection_matrix;
	vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	uvec3 cluster_grid_size;
	int light_count;
	float near_plane;
	float far_plane;
};

layout(set = 0, binding = 1) readonly buffer light_buffer {
	Light lights[];
};

layout(std430, set=0, binding = 2) readonly buffer light_assignment_buffer
{
	uint light_assignment_indicies[];
};

struct ClusterLightAssignment {
	uint start_index;
	uint count;
};

layout(std430, set=0, binding = 3) readonly buffer cluster_buffer
{
	ClusterLightAssignment cluster_assignments[];
};


in vec2 light_volume_pos;

struct GBufferData {
	vec4 color_and_roughness;
	vec3 normals;
	vec3 view_space_pos;
	ClusterLightAssignment cluster_assignment;
};



vec3 GetFragmentPosition(vec2 coordinates, float depth) {
	return vec3(light_volume_pos, -1.0) * depth;
}

uint get_depth_slice_index(float depth) {
	float scale = cluster_grid_size.z / (log2(far_plane/near_plane));
	float bias = -scale*log2(near_plane);
	return int(min(max(log2(depth)*scale + bias, 0.0f), cluster_grid_size.z - 1));
}

uint get_cluster_index(vec2 coords, float depth) {
	uint slice = get_depth_slice_index(depth);

	coords.y = 1.0f - coords.y;
	uvec3 cluster_coords = uvec3(min(uvec2(coords.xy * vec2(cluster_grid_size.xy)), cluster_grid_size.xy - 1u),
		slice);

	uint index = cluster_coords.x
		+ cluster_coords.y * cluster_grid_size.x
		+ cluster_coords.z * cluster_grid_size.x * cluster_grid_size.y;

	return index;
}

float calculate_shadows_point(vec3 view_space_pos, uint index, vec3 normal) {
	vec3 light_space_pos = (lights[index].light_matrix * vec4(view_space_pos, 1.0)).xyz + normal * 0.02;
	float current_depth = length(light_space_pos) / lights[index].light_far_plane;
	vec4 shadow_coords = vec4(normalize(light_space_pos.xyz) * vec3(1,-1,1), current_depth - 0.0002);
	return texture(PointShadowMaps[lights[index].shadow_index], shadow_coords);
}

#include <shaders/utils/NormalPacking.glsl>
#include <shaders/utils/PointAttenuationFalloff.glsl>

vec3 ComputePointLight(uint light_index, vec3 normals, vec3 view_space_pos, vec4 color_and_roughness) {
	float shadow_contrib = 1.0f;
	if(lights[light_index].shadow_index != ~uint(0)) {
		shadow_contrib *= calculate_shadows_point(view_space_pos, light_index, normals);
	}

	float distance = length(vec3(lights[light_index].position_or_direction_and_radius) - view_space_pos);
	shadow_contrib *= PointAttenuationFalloff(distance, lights[light_index].range);

	vec3 light_direction = normalize(vec3(lights[light_index].position_or_direction_and_radius) - view_space_pos);
	shadow_contrib *= smoothstep(-0.02, 0.02,dot(light_direction, normals));
	float light_contrib = 0.5f * (0.1 + max(0, dot(normals,  light_direction))); // diffuse part
	light_contrib += 0.5f * pow(clamp(dot(normals, normalize( light_direction - normalize(view_space_pos))), 0, 1), 1 +((1 - color_and_roughness.w) * 64)); //specular part

	return vec3(light_contrib * shadow_contrib * lights[light_index].Light_Color.xyz * lights[light_index].Light_Color.w * color_and_roughness.xyz);
}

vec3 ComputeDirectionalLight(uint light_index, vec3 normals, vec3 view_space_pos, vec4 color_and_roughness) {
	vec3 light_direction = normalize(vec3(lights[light_index].position_or_direction_and_radius));
	float light_contrib = 0.5f * (0.1 + max(0, dot(normals, - light_direction))); // diffuse part
	light_contrib += 0.5f * pow(clamp(dot(normals, normalize(- light_direction - normalize(view_space_pos))), 0, 1), 1 +((1 - color_and_roughness.w) * 64)); //specular part
	light_contrib *= smoothstep(-0.02, 0.02,dot(-light_direction, normals));
	vec4 Light_Color = lights[light_index].Light_Color;
	return vec3(color_and_roughness.xyz * Light_Color.xyz * Light_Color.w * light_contrib);
}

GBufferData GetGBufferData(vec2 coords) {
	GBufferData data;
	float linearized_depth = depth_constant_b / (texture(DepthBuffer, coords.xy).x - depth_constant_a);
	data.cluster_assignment = cluster_assignments[get_cluster_index(coords, linearized_depth)];
	data.view_space_pos = GetFragmentPosition(coords, linearized_depth);
	data.normals = UnpackNormals(texture(Normal, coords.xy).xy);
	data.color_and_roughness = vec4(texture(Color, coords.xy).xyz, texture(Roughness, coords.xy).x);
	return data;
}

void main() {
	GBufferData gbuffer_data = GetGBufferData(vec2((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y)));

	color_out = vec4(0.0,0.0,0.0,1.0);
	for(int i = 0; i < gbuffer_data.cluster_assignment.count; i++) {
		uint light_index = light_assignment_indicies[gbuffer_data.cluster_assignment.start_index + i];
		if (lights[light_index].light_type == 0) {
			color_out += vec4(ComputeDirectionalLight(light_index, gbuffer_data.normals, gbuffer_data.view_space_pos, gbuffer_data.color_and_roughness),0.0);
		}
		else {
			color_out += vec4(ComputePointLight(light_index, gbuffer_data.normals, gbuffer_data.view_space_pos, gbuffer_data.color_and_roughness),0.0);
		}
	}
}

// #end
