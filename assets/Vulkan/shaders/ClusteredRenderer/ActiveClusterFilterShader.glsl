/*
#RootSignature
{
	"RootSignature": [
		{
			"name" : "config_buffer",
			"type" : "constant_buffer"
		},
		{
			"name" : "active_clusters",
			"type" : "storage_buffer"
		},
		{
			"name" : "DepthBuffer",
			"type" : "texture_2D"
		}
	]

}
#end
*/
// #Compute //--------------------------------------------------
#version 430

layout(set = 0, binding = 0) uniform config_buffer
{
	uvec3 cluster_dimensions;
	float near_plane;
	uvec2 window_size;
	float depth_constant_a;
	float depth_constant_b;
	float far_plane;
};

layout(set = 0, binding = 1) buffer active_clusters_buffer
{
	uint cluster_count;
	uint active_clusters[];
};

layout(set = 0, binding = 2) uniform sampler2D DepthBuffer;

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

#define BITMASK_SIZE 128
struct TileMask {
	uint mask_element[BITMASK_SIZE >> 5];
};

shared TileMask tile_mask;

uint get_depth_slice_index(float depth) {
	float scale = cluster_dimensions.z / (log2(far_plane/near_plane));
	float bias = -scale*log2(near_plane);
	return int(min(max(log2(depth)*scale + bias, 0.0f), cluster_dimensions.z - 1));
}

void main() {
	if (gl_LocalInvocationIndex < BITMASK_SIZE >> 5) {
		tile_mask.mask_element[gl_LocalInvocationIndex] = 0u;
	}

	memoryBarrierShared();
	barrier();

	uvec2 tile_index = uvec2(gl_WorkGroupID.xy);
	uvec2 tile_size = uvec2(ceil(vec2(window_size) / vec2(cluster_dimensions)));
	uint pixel_count = tile_size.x * tile_size.y;
	for(uint i = gl_LocalInvocationIndex; i < pixel_count ; i += gl_WorkGroupSize.x) {
		uvec2 coords = (tile_size * tile_index) + uvec2(i % tile_size.x, i / tile_size.x);

		if (coords.x >= window_size.x || coords.y >= window_size.y) continue;

		float depth = texelFetch(DepthBuffer, ivec2(coords.x, window_size.y - 1 - coords.y), 0).x;
		float linearized_depth = depth_constant_b / (depth - depth_constant_a);
		uint depth_slice = get_depth_slice_index(linearized_depth);
		if(depth_slice < BITMASK_SIZE) {
			atomicOr(tile_mask.mask_element[depth_slice >> 5], 1u << (depth_slice & 31u));
		}
	}

	memoryBarrierShared();
	barrier();

	if(gl_LocalInvocationIndex == 0) {
		uint count = 0;

		for(int i = 0; i < BITMASK_SIZE >> 5; i++) {
			count += bitCount(tile_mask.mask_element[i]);
		}
		uint index = atomicAdd(cluster_count, count);
		uint element = 0;
		uint y_offset = uint(ceil(log2(cluster_dimensions.x)));
		uint z_offset = y_offset + uint(ceil(log2(cluster_dimensions.y)));
		for(int i = 0; i < BITMASK_SIZE;i++) {
			if((tile_mask.mask_element[i >> 5] & (1u << (i & 31u))) != 0) {
				uint cluster_key = tile_index.x | (tile_index.y << y_offset) | (i << z_offset);
				active_clusters[index + element] = cluster_key;
				element++;
			}
		}
	}

}

// #end
