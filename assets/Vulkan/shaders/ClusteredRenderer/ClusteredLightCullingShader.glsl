/*
#RootSignature
{
	"RootSignature": [
        {
            "name": "config_buffer",
            "type": "constant_buffer"
        },
        {
            "name": "light_buffer",
            "type": "storage_buffer"
        },
        {
            "name": "light_assignment_buffer",
            "type": "storage_buffer"
        },
        {
            "name": "cluster_buffer",
            "type": "storage_buffer"
        },
        {
            "name": "allocator_buffer",
            "type": "storage_buffer"
        }
	]
}
#end
*/
// #Compute //--------------------------------------------------
#version 430

layout(set = 0, binding = 0) uniform config_buffer
{
    mat4 projection_matrix;
    mat4 view_matrix;
    uvec3 cluster_grid_size;
    uint light_count;
    uint light_assignment_size;
    float near_plane;
    float far_plane;
    float fov;
    float aspect_ratio;
};

struct Light {
    vec4 position_and_radius;
};

layout(std430, set=0, binding = 1) readonly buffer light_buffer
{
    Light lights[];
};

layout(std430, set=0, binding = 2) buffer light_assignment_buffer
{
    uint light_assignment_indicies[];
};

struct ClusterLightAssignment {
    uint start_index;
    uint count;
};

layout(std430, set=0, binding = 3) buffer cluster_buffer
{
    ClusterLightAssignment cluster_assignments[];
};

layout(std430, set=0, binding = 4) buffer allocator_buffer
{
    uint allocator_index;
    int success;
};

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

float depth_slice(uint index, uint number_of_slices, float near_plane, float far_plane) {
    return near_plane * pow(far_plane / near_plane, float(index)/float(number_of_slices)); // can be optimized
}

vec3 get_cluster_point(uvec2 xy_indicies, float depth, float tangent) {
    float x_halfspan = tangent * depth * aspect_ratio;
    float x_step = 2 * x_halfspan / cluster_grid_size.x;
    float x_offset = - x_halfspan + x_step * xy_indicies.x;

    float y_halfspan = tangent * depth;
    float y_step = 2 * y_halfspan / cluster_grid_size.y;
    float y_offset = - y_halfspan + y_step * xy_indicies.y;

    return vec3(x_offset, y_offset, -depth);
}

struct BoundingBox {
    vec3 center;
    vec3 half_extents;
};

BoundingBox GetAABB(uvec3 cluster_coords, uvec3 cluster_grid_size) {
    float near_slice = depth_slice(cluster_coords.z, cluster_grid_size.z, near_plane, far_plane);
    float far_slice = depth_slice(cluster_coords.z + 1, cluster_grid_size.z, near_plane, far_plane);

    float tangent = tan(fov/2);

    vec3 points[4] = {
        get_cluster_point(uvec2(cluster_coords.x, cluster_coords.y), near_slice, tangent),
        get_cluster_point(uvec2(cluster_coords.x + 1, cluster_coords.y + 1), near_slice, tangent),
        get_cluster_point(uvec2(cluster_coords.x, cluster_coords.y), far_slice, tangent),
        get_cluster_point(uvec2(cluster_coords.x + 1, cluster_coords.y + 1), far_slice, tangent)
    };

    vec3 minimum = points[0].xyz;
    vec3 maximum = points[0].xyz;
    for(int i = 1; i < 4; i++) {
        minimum = min(minimum, points[i]);
        maximum = max(maximum, points[i]);
    }

    return BoundingBox((maximum + minimum) / 2, (maximum - minimum) / 2);
}

bool sphere_aabb_overlap_test(vec4 sphere_pos_and_radius, BoundingBox box) {
    vec3 center_line = sphere_pos_and_radius.xyz - box.center;
    vec3 bounded_center_line = clamp(center_line, -box.half_extents, box.half_extents);
    float len = length(center_line) - length(bounded_center_line);
    return len < sphere_pos_and_radius.w;
}

void main() {
    uint index = gl_LocalInvocationIndex + gl_WorkGroupID.x * 256;
    uint num_of_clusters = cluster_grid_size.x * cluster_grid_size.y * cluster_grid_size.z;
    if(index >= num_of_clusters) return;

    uvec3 cluster_coords; // we can optimize this quite heavily by using powers of two
    cluster_coords.x = index % cluster_grid_size.x;
    cluster_coords.y = (index / cluster_grid_size.x) % cluster_grid_size.y;
    cluster_coords.z = (index / (cluster_grid_size.y * cluster_grid_size.x));

    BoundingBox bounding_box = GetAABB(cluster_coords, cluster_grid_size);

    uint count = 0;
    for(int i = 0; i < light_count; i++) {
        if(sphere_aabb_overlap_test(lights[i].position_and_radius, bounding_box)) {
            count++;
        }
    }

    uint allocated_offset = atomicAdd(allocator_index, count);
    if(allocated_offset + count >= light_assignment_size) {
        atomicExchange(success, 0);
        return;
    }
    cluster_assignments[index].count = count;
    cluster_assignments[index].start_index = allocated_offset;

    uint write_index = 0;
    for(int i = 0; i < light_count && write_index < count; i++) {
        if(sphere_aabb_overlap_test(lights[i].position_and_radius, bounding_box)) {
            light_assignment_indicies[allocated_offset + write_index] = i;
            write_index++;
        }
    }
}

// #end