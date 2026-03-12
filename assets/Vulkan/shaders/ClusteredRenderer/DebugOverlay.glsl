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
            "name" : "light_accum_buffer",
			"type" : "texture_2D"
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
// #Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(set = 0, binding = 0) uniform conf {
    mat4 projection_matrix;
    vec2 pixel_size;
    float depth_constant_a;
    float depth_constant_b;
    uvec3 cluster_grid_size;
    int light_count;
    float near_plane;
    float far_plane;
    float opacity;
};

out vec2 uv_fragment;
out vec2 interpolated_view_space_pos;

void main() {
    gl_Position = vec4(position,0.0,1.0);
    uv_fragment = uv;
    uv_fragment.y = 1 - uv_fragment.y;

    vec3 view_space_pos = vec3(inverse(projection_matrix) * vec4(position.xy, -1.0, 1.0));
    interpolated_view_space_pos = view_space_pos.xy / abs(view_space_pos.z);
}

// #end
// #Fragment //------------------------------------------------
#version 430

layout(location = 0) out vec4 color_out;

in vec2 uv_fragment;
in vec2 interpolated_view_space_pos;

layout(set = 1, binding = 0) uniform sampler2D Color;
layout(set = 1, binding = 1) uniform sampler2D Normal;
layout(set = 1, binding = 2) uniform sampler2D Roughness;
layout(set = 1, binding = 3) uniform sampler2D DepthBuffer;

layout(set = 0, binding = 4) uniform sampler2D LightAccumulationBuffer;

layout(set = 0, binding = 0) uniform conf {
    mat4 projection_matrix;
    vec2 pixel_size;
    float depth_constant_a;
    float depth_constant_b;
    uvec3 cluster_grid_size;
    int light_count;
    float near_plane;
    float far_plane;
    float opacity;
    uint mode;
};

struct Light {
    mat4 light_matrix;
    vec4 position_or_direction_and_radius;
    vec4 Light_Color;
    float range;
    int light_type;
    uint shadow_index;
    float light_far_plane;
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

struct GBufferData {
    vec4 color_and_roughness;
    vec3 normals;
    vec3 view_space_pos;
    ClusterLightAssignment cluster_assignment;
    uint cluster_index;
};


vec3 GetFragmentPosition(float depth) {
    return vec3(interpolated_view_space_pos, -1.0) * depth;
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

#include <shaders/utils/NormalPacking.glsl>

GBufferData GetGBufferData(vec2 coords) {
    GBufferData data;
    float linearized_depth = depth_constant_b / (texture(DepthBuffer, coords.xy).x - depth_constant_a);
    data.cluster_index = get_cluster_index(coords, linearized_depth);
    data.cluster_assignment = cluster_assignments[data.cluster_index];
    data.view_space_pos = GetFragmentPosition(linearized_depth);
    data.normals = UnpackNormals(texture(Normal, coords.xy).xy);
    data.color_and_roughness = vec4(texture(Color, coords.xy).xyz, texture(Roughness, coords.xy).x);
    return data;
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


#define CLUSTER_GRID 0
#define TILES 1
#define LIGHT_COUNT 2
#define RADIUS 3
#define DEPTH_SLICE 4


void main() {
    vec2 coords = vec2((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y));
    GBufferData gbuffer_data = GetGBufferData(coords);
    uint inside = 0;


    color_out = vec4(vec3(0.0), 1.0);
    if(mode == RADIUS) {
        for (int i = 0; i < gbuffer_data.cluster_assignment.count; i++) {
            uint light_index = light_assignment_indicies[gbuffer_data.cluster_assignment.start_index + i];
            if (lights[light_index].light_type == 1) {
                float distance = length(vec3(lights[light_index].position_or_direction_and_radius) - gbuffer_data.view_space_pos);
                if (distance < lights[light_index].position_or_direction_and_radius.w) {
                    inside = 1;
                }
            }
        }
        if(inside == 1) {
            color_out = vec4(random(gbuffer_data.cluster_index),1.0f);
        } else {
            color_out = vec4(vec3(0), 1);
        }
    }

    if(mode == LIGHT_COUNT) {
        color_out += vec4(0, gbuffer_data.cluster_assignment.count / 255.0f, 0, 0);
    }

    if(mode == CLUSTER_GRID) {
        color_out = vec4(random(gbuffer_data.cluster_index), 1.0f);
    }

    if(mode == DEPTH_SLICE) {
        float linearized_depth = depth_constant_b / (texture(DepthBuffer, coords.xy).x - depth_constant_a);
        color_out = vec4(random(get_depth_slice_index(linearized_depth)), 1.0f);
    }

    if(mode == TILES) {
        uvec2 cluster_coords = uvec2(min(uvec2(coords.xy * vec2(cluster_grid_size.xy)), cluster_grid_size.xy - 1u));
        color_out = vec4(random(cluster_coords.x + cluster_coords.y * cluster_grid_size.x), 1.0f);
    }


    color_out.a = opacity;
}

// #end
