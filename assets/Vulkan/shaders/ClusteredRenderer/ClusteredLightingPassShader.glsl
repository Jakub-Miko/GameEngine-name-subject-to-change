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

out vec3 light_volume_pos;

void main() {
	gl_Position = vec4(position, 1.0);
	light_volume_pos = vec3(inverse(projection_matrix) * vec4(position.xy, -1.0, 1.0));
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


in vec3 light_volume_pos;

vec3 GetFragmentPosition(vec3 coordinates) {
	vec3 dir = vec3(light_volume_pos.xy / abs(light_volume_pos.z), -1.0);
	float depth = texture(DepthBuffer, coordinates.xy).x;

	float linearized_depth = depth_constant_b / (depth - depth_constant_a);
	return dir * linearized_depth;
}

uint get_depth_slice_index(float depth) {
	float scale = cluster_grid_size.z / (log2(far_plane/near_plane));
	float bias = -scale*log2(near_plane);
	return int(min(max(log2(depth)*scale + bias, 0.0f), cluster_grid_size.z - 1));
}

uint pixel_depth_slice(vec3 coords) {
	float depth = texture(DepthBuffer, coords.xy).x;
	float linearized_depth = depth_constant_b / (depth - depth_constant_a);
	uint slice = get_depth_slice_index(linearized_depth);
	return slice;
}

uint get_cluster_index(vec3 coords) {
	uint slice = pixel_depth_slice(coords);

	coords.y = 1.0f - coords.y;
	uvec3 cluster_coords = uvec3(min(uvec2(coords.xy * vec2(cluster_grid_size.xy)), cluster_grid_size.xy - 1u),
		slice);

	uint index = cluster_coords.x
		+ cluster_coords.y * cluster_grid_size.x
		+ cluster_coords.z * cluster_grid_size.x * cluster_grid_size.y;

	return index;
}

vec3 random(uint x)
{
	//From https://nullprogram.com/blog/2018/07/31/
	x ^= x >> 16;
	x *= 0x7feb352dU;
	x ^= x >> 15;
	x *= 0x846ca68bU;
	x ^= x >> 16;

	vec3 color;
	color.r = float(x & 255) / 255.0;
	color.g = float(x >> 8 & 255) / 255.0;
	color.b = float(x >> 16 & 255) / 255.0;
	return color;
}

float calculate_shadows_point(vec3 view_space_pos, vec3 coords, uint index) {
	vec4 light_space_pos = lights[index].light_matrix * vec4(view_space_pos, 1.0);

	vec3 normal = texture(Normal, coords.xy).xyz;
	vec3 bias = normal * 0.02;

	light_space_pos += vec4(bias,0);
	vec3 light_space_coords = normalize(light_space_pos.xyz);
	float current_depth = length(light_space_pos) / lights[index].light_far_plane;
	vec4 shadow_coords = vec4(light_space_coords.xyz * vec3(1,-1,1), current_depth - 0.0002);
	float shadow_map_depth = texture(PointShadowMaps[lights[index].shadow_index], shadow_coords);

	return shadow_map_depth;
}

#include <shaders/utils/NormalPacking.glsl>
#include <shaders/utils/PointAttenuationFalloff.glsl>

void main() {
	vec3 coords = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 view_space_pos = GetFragmentPosition(coords);
	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	vec3 normal = UnpackNormals(texture(Normal, coords.xy).xy);
	float roughness = texture(Roughness, coords.xy).x;
	vec3 color_accum = vec3(0.0);

	uint index = get_cluster_index(coords);

	ClusterLightAssignment list = cluster_assignments[index];

	#ifdef DEBUG_RADIUS
		int inside = 0;
	#endif

	for(int i = 0; i < list.count; i++) {
		vec3 light_direction;
		uint light_index = light_assignment_indicies[list.start_index + i];
		if (lights[light_index].light_type == 0) {
			light_direction = normalize(vec3(lights[light_index].position_or_direction_and_radius));
		}
		else {
			light_direction = - normalize(vec3(lights[light_index].position_or_direction_and_radius) - view_space_pos);
		}
		float attenuation_factor = smoothstep(-0.02, 0.02,dot(-light_direction, normal));

		if (lights[light_index].light_type == 1) {
			float distance = length(vec3(lights[light_index].position_or_direction_and_radius) - view_space_pos);

			#ifdef DEBUG_RADIUS
				if(distance < lights[light_index].position_or_direction_and_radius.w) {
					inside = 1;
				}
			#endif

			attenuation_factor *= PointAttenuationFalloff(distance, lights[light_index].range);
		}

		float diffuse_contribution = 0.5f * (0.1 + max(0, dot(normal, - light_direction)));
		float specular_contribution = 0.5f * pow(clamp(dot(normal, normalize(- light_direction - normalize(view_space_pos))), 0, 1), 1 +((1 - roughness) * 64));

		float contribution = diffuse_contribution + specular_contribution;

		vec4 Light_Color = lights[light_index].Light_Color;
		float shadow_contrib = 1.0f;
		if(lights[light_index].shadow_index != ~uint(0)) {
			shadow_contrib = calculate_shadows_point(view_space_pos, coords, light_index);
		}
		color_accum += vec3(color.xyz * Light_Color.xyz * attenuation_factor * Light_Color.w * contribution * shadow_contrib);
	}
	color_out = vec4(color_accum, 1.0);

	#ifdef DEBUG_LIGHT_COUNT
	color_out += vec4(0,list.count / 20.0f,0,0);
	#endif

	#ifdef DEBUG_CLUSTERS
	color_out = vec4(random(index),1.0f);
	#endif

	#ifdef DEBUG_DEPTH_SLICES
	color_out = vec4(random(pixel_depth_slice(coords)),1.0f);
	#endif

	#ifdef DEBUG_TILES
	uvec2 cluster_coords = uvec2(min(uvec2(coords.xy * vec2(cluster_grid_size.xy)), cluster_grid_size.xy - 1u));
	color_out = vec4(random(cluster_coords.x + cluster_coords.y * cluster_grid_size.x),1.0f);
	#endif

	#ifdef DEBUG_RADIUS
	if(inside == 1) {
		color_out = vec4(random(index),1.0f);
	} else {
		color = vec4(vec3(0), 1);
	}
	#endif
}

// #end
