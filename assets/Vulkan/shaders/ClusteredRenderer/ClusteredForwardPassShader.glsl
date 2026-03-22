/*
#RootSignature
{
	"RootSignature": [
		{
			"type" : "push_constants",
			"size" : 64
		},
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
		{
			"name" : "point_light_buffer",
			"type" : "storage_buffer"
		},
		{
			"name" : "directional_light_buffer",
			"type" : "storage_buffer"
		},
		{
			"name" : "skylight_buffer",
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
			"name" : "point_light_shadow_maps",
			"type" : "resource_store",
			"store_type" : "texture_2D_cubemap"
		},
		{
			"name" : "directional_light_shadow_maps",
			"type" : "resource_store",
			"store_type" : "texture_2D_array"
		},
		{
			"name" : "skylight_reflection_maps",
			"type" : "resource_store",
			"store_type" : "texture_2D_cubemap"
		},
		{
			"name" : "DeferredGPassMaterial",
			"type" : "material",
			"material_path": "api:GeometryMaterial.json"
		}
	]

}
#end
*/
// #Vertex //--------------------------------------------------
#version 430


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec2 uv;

out vec2 uv_fragment;
out vec3 pos_fragment;
out mat3 TBN;

layout(set = 0, binding = 0) uniform conf {
	mat4 projection_matrix;
	mat4 inverse_projection;
	mat4 inverse_view_matrix;
	vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	uvec3 cluster_grid_size;
	int point_light_count;
	int directional_light_count;
	int skylight_count;
	float near_plane;
	float far_plane;
};

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
};


void main() {
	vec3 normal_transformed = normalize(mat3(transpose(inverse(mv_matrix))) * normal.xyz).xyz;
	vec3 tangent_transformed = normalize(mat3(transpose(inverse(mv_matrix))) * tangent.xyz).xyz;
	vec3 bitangent_transformed = cross(normal_transformed, tangent_transformed) * tangent.w;

	TBN = mat3(tangent_transformed, bitangent_transformed, normal_transformed);

	pos_fragment = (mv_matrix * vec4(position, 1.0)).xyz;
	gl_Position = projection_matrix * mv_matrix * vec4(position, 1.0);
	uv_fragment = uv;
	uv_fragment.y = 1 - uv_fragment.y;
}


// #end
// #Fragment //------------------------------------------------
#version 430

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec4 color_out;

layout(set = 1, binding = 0) uniform samplerCubeShadow PointShadowMaps[];
layout(set = 2, binding = 0) uniform sampler2DArrayShadow DirectionalShadowMaps[];
layout(set = 3, binding = 0) uniform samplerCube SkylightReflectionMaps[];

layout(set = 4, binding = 1) uniform sampler2D Color;
layout(set = 4, binding = 2) uniform sampler2D Normal;
layout(set = 4, binding = 3) uniform sampler2D Material;

layout(set = 4, binding = 0) uniform material{
	vec4 Base_Color;
	float roughness_bias;
	float roughness_gain;
	float metallic_bias;
	float metallic_gain;
};


#ifndef HARD_CODE_CASCADES
#define HARD_CODE_CASCADES 5
#endif

layout(set = 0, binding = 0) uniform conf {
	mat4 projection_matrix;
	mat4 inverse_projection;
	mat4 inverse_view_matrix;
	vec2 pixel_size;
	float depth_constant_a;
	float depth_constant_b;
	uvec3 cluster_grid_size;
	int point_light_count;
	int directional_light_count;
	int skylight_count;
	float near_plane;
	float far_plane;
};

struct PointLight {
	mat4 light_matrix;
	vec4 position_and_radius;
	vec4 Light_Color;
	float range;
	uint shadow_index;
	float light_far_plane;
};

struct DirectionalLight {
	mat4 light_matrix[HARD_CODE_CASCADES];
	vec4 direction;
	vec4 Light_Color;
	vec2 shadowmap_pixel_size;
	uint shadow_index;
	float light_far_plane;
	float shadow_bias;
};

struct Skylight {
	vec4 Light_Color;
	uint specular_map_index;
	uint diffuse_map_index;
};

layout(set = 0, binding = 1) readonly buffer point_light_buffer {
	PointLight point_lights[];
};

layout(set = 0, binding = 2) readonly buffer directional_light_buffer {
	DirectionalLight directional_lights[];
};

layout(set = 0, binding = 3) readonly buffer skylight_buffer {
	Skylight skylights[];
};

layout(std430, set=0, binding = 4) readonly buffer light_assignment_buffer
{
	uint light_assignment_indicies[];
};

struct ClusterLightAssignment {
	uint start_index;
	uint count;
};

layout(std430, set=0, binding = 5) readonly buffer cluster_buffer
{
	ClusterLightAssignment cluster_assignments[];
};


in vec2 uv_fragment;
in vec3 pos_fragment;
in mat3 TBN;


vec3 GetFragmentPosition(vec2 coords, float depth) {
	vec3 light_pos = vec3(inverse_projection * vec4(2.0 * coords.x - 1.0, 1.0 - 2.0*coords.y, -1.0, 1.0));
	light_pos.xy = light_pos.xy / abs(light_pos.z);
	return vec3(light_pos.xy, -1.0) * depth;
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

float calculate_shadows_directional(vec3 view_space_pos, uint index, vec3 normal) {
	float depth = abs(view_space_pos.z);
	depth -= near_plane;
	depth /= far_plane - near_plane;
	depth = sqrt(depth);
	depth *= float(HARD_CODE_CASCADES);
	int cascade = int(floor(depth));

	vec3 offset_view_space_pos = view_space_pos + normal * directional_lights[index].shadow_bias * (cascade+1);
	vec4 light_space_pos = directional_lights[index].light_matrix[cascade] * vec4(offset_view_space_pos,1.0);
	vec3 light_space_coords = light_space_pos.xyz / light_space_pos.w;
	float current_depth = light_space_coords.z;
	light_space_coords = light_space_coords * 0.5 + 0.5;
	float accumulate = 0;

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			vec4 shadow_coords = vec4(light_space_coords.x + x * directional_lights[index].shadowmap_pixel_size.x,
			(1 - light_space_coords.y) + y * directional_lights[index].shadowmap_pixel_size.y, cascade, current_depth);
			accumulate += texture(DirectionalShadowMaps[directional_lights[index].shadow_index], shadow_coords);

		}
	}

	return accumulate / 9;
}


float calculate_shadows_point(vec3 view_space_pos, uint index, vec3 normal) {
	vec3 light_space_pos = (point_lights[index].light_matrix * vec4(view_space_pos, 1.0)).xyz + normal * 0.02;
	float current_depth = length(light_space_pos) / point_lights[index].light_far_plane;
	vec4 shadow_coords = vec4(normalize(light_space_pos.xyz) * vec3(1,-1,1), current_depth - 0.0002);
	return texture(PointShadowMaps[point_lights[index].shadow_index], shadow_coords);
}

#include <shaders/utils/NormalPacking.glsl>
#include <shaders/utils/PointAttenuationFalloff.glsl>
#include <shaders/utils/PBR.glsl>

vec3 ComputeSkylight(uint index, vec3 normal, vec3 view_space_pos, vec3 color, float roughness) {
	vec3 reglected_normal = mat3(inverse_view_matrix) * reflect(normalize(view_space_pos), normal);
	reglected_normal.y = -reglected_normal.y;

	vec3 diffuse_contribution = texture(SkylightReflectionMaps[skylights[index].diffuse_map_index], normalize(reglected_normal)).xyz;
	vec3 specular_contribution = textureLod(SkylightReflectionMaps[skylights[index].specular_map_index], normalize(reglected_normal), roughness*4.0).xyz;

	return vec3(color.xyz * skylights[index].Light_Color.xyz * skylights[index].Light_Color.w * (diffuse_contribution + specular_contribution));
}

vec3 ComputePointLight(uint light_index, vec3 normals, vec3 view_space_pos, vec3 color, float roughness, float metallic) {
	float shadow_contrib = 1.0f;
	if(point_lights[light_index].shadow_index != ~uint(0)) {
		shadow_contrib *= calculate_shadows_point(view_space_pos, light_index, normals);
	}

	vec3 light_direction = vec3(point_lights[light_index].position_and_radius) - view_space_pos;
	float ligth_distance = length(light_direction);
	light_direction /= ligth_distance;
	vec3 light_radiance = point_lights[light_index].Light_Color.xyz * point_lights[light_index].Light_Color.w;
	light_radiance *= PointAttenuationFalloff(ligth_distance, point_lights[light_index].range);
	light_radiance *= shadow_contrib;

	return CookTorranceModel(light_direction, -normalize(view_space_pos), normals,
							 color, roughness, metallic) * light_radiance;
}

vec3 ComputeDirectionalLight(uint light_index, vec3 normals, vec3 view_space_pos, vec3 color, float roughness, float metallic) {
	float shadow_contrib = 1.0f;
	if(directional_lights[light_index].shadow_index != ~uint(0)) {
		shadow_contrib *= calculate_shadows_directional(view_space_pos, light_index, normals);
	}

	vec3 light_direction = normalize(vec3(directional_lights[light_index].direction));
	vec3 light_radiance = directional_lights[light_index].Light_Color.xyz * directional_lights[light_index].Light_Color.w;
	light_radiance *= shadow_contrib;
	vec4 Light_Color = directional_lights[light_index].Light_Color;

	return CookTorranceModel(-light_direction, -normalize(view_space_pos), normals,
							 color, roughness, metallic) * light_radiance;
}

void main() {
	vec2 coords = vec2((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y));
	vec4 view_space_pos = vec4(vec3(1.0), depth_constant_b / (gl_FragCoord.z - depth_constant_a));
	view_space_pos.xyz = GetFragmentPosition(coords, view_space_pos.w);
	vec3 surface_color = texture(Color, uv_fragment).xyz * vec3(Base_Color);
	vec3 material = texture(Material, uv_fragment).xyz;
	material.y = clamp(material.y * roughness_gain + roughness_bias, 0.0, 1.0);
	material.z = clamp(material.z * metallic_gain + metallic_bias, 0.0, 1.0);
	vec3 normal = vec3(TBN * (texture(Normal, uv_fragment).rgb * 2.0 - 1.0));
	ClusterLightAssignment assignment = cluster_assignments[get_cluster_index(coords, view_space_pos.w)];

	vec3 color = vec3(0.0,0.0,0.0);

	for(uint i = 0; i < directional_light_count; i++) {
		color += vec3(ComputeDirectionalLight(i, normal, view_space_pos.xyz,
											  surface_color, material.y, material.z));
	}

	for(uint i = 0; i < skylight_count; i++) {
		color += vec3(ComputeSkylight(i, normal, view_space_pos.xyz,
									  surface_color, material.y));
	}

	uint end = assignment.start_index + assignment.count;
	for(uint i = assignment.start_index; i < end; i++) {
		uint light_index = light_assignment_indicies[i];
		color += vec3(ComputePointLight(light_index, normal, view_space_pos.xyz,
										surface_color, material.y, material.z));
	}
	color_out = vec4(color,1.0);
}

// #end
