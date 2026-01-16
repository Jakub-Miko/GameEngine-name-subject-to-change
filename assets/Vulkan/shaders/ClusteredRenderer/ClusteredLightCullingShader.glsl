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
        },
        {
			"name" : "active_cluster_buffer",
			"type" : "storage_buffer"
		}
	]
}
#end
*/
// #Compute //--------------------------------------------------
#version 430

#extension GL_EXT_debug_printf : enable

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
    vec4 position_or_direction_and_radius;
    vec4 Light_Color;
    vec4 attenuation_constants;
    int light_type;
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

layout(std430, set=0, binding = 5) readonly buffer active_cluster_buffer
{
    uint active_cluster_count;
    uint active_cluster_indicies[];
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

    float tangent = tan(fov/2.0);

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

bool sphere_aabb_overlap_test(vec4 sphere_pos_and_radius, uvec3 cluster_coords) {
    float near_z = depth_slice(cluster_coords.z, cluster_grid_size.z, near_plane, far_plane);
    float far_z = depth_slice(cluster_coords.z + 1u, cluster_grid_size.z, near_plane, far_plane);
    float tan_fov = tan(fov * 0.5);

    if (sphere_pos_and_radius.z + sphere_pos_and_radius.w < -far_z ||
        sphere_pos_and_radius.z - sphere_pos_and_radius.w > -near_z) return false;

    float new_z, new_radius;
    if (sphere_pos_and_radius.z <= -near_z && sphere_pos_and_radius.z >= -far_z) {
        new_z = sphere_pos_and_radius.z;
        new_radius = sphere_pos_and_radius.w;
    } else {
        float d_near = abs(sphere_pos_and_radius.z + near_z);
        float d_far = abs(sphere_pos_and_radius.z + far_z);
        new_z = (d_near < d_far) ? -near_z : -far_z;
        float dist = min(d_near, d_far);
        new_radius = sqrt(sphere_pos_and_radius.w * sphere_pos_and_radius.w - dist * dist);
    }

    float y_extent = tan_fov * abs(new_z);
    float x_extent = y_extent * aspect_ratio;

    float y_step = 2.0 * y_extent / float(cluster_grid_size.y);
    float x_step = 2.0 * x_extent / float(cluster_grid_size.x);

    float y_min = -y_extent + y_step * float(cluster_coords.y);
    float y_max = y_min + y_step;
    float x_min = -x_extent + x_step * float(cluster_coords.x);
    float x_max = x_min + x_step;

    vec3 n_bot = normalize(vec3(0.0, -new_z, y_min));
    vec3 n_top = normalize(vec3(0.0, new_z, -y_max));

    vec3 center = vec3(sphere_pos_and_radius.xy, new_z);
    float d_bot = dot(n_bot, center);
    float d_top = dot(n_top, center);

    if (d_bot < -new_radius || d_top < -new_radius) return false;

    vec3 new_center;
    if (d_bot >= 0.0 && d_top >= 0.0) {
        // Inside Y bounds
        new_center = center;
    } else {
        float y_dist;
        if (d_bot < 0.0) {
            y_dist = -d_bot;
            new_center = center - n_bot * d_bot;
        } else {
            y_dist = -d_top;
            new_center = center - n_bot * d_bot;
        }
        new_radius = sqrt(new_radius * new_radius - y_dist * y_dist);
    }

    float d_left = dot(normalize(vec3(-new_z, 0.0, x_min)), new_center);
    float d_right = dot(normalize(vec3(new_z, 0.0, -x_max)), new_center);

    return (d_left >= -new_radius && d_right >= -new_radius);
}

void main() {
    uint index = gl_LocalInvocationIndex + gl_WorkGroupID.x * 256;
    if(index >= active_cluster_count) {
        return;
    }

    uint num_of_clusters = cluster_grid_size.x * cluster_grid_size.y * cluster_grid_size.z;

    uint cluster_key = active_cluster_indicies[index];
    uvec3 cluster_coords; // we can optimize this quite heavily by using powers of two
    uint xmask = uint(ceil(log2(cluster_grid_size.x)));
    uint ymask = uint(ceil(log2(cluster_grid_size.y)));
    cluster_coords.x = cluster_key & ~(~0u << xmask);
    cluster_coords.y = (cluster_key >> xmask) & ~(~0u << ymask);
    cluster_coords.z = cluster_key >> (xmask + ymask);

    uint count = 0;
    for(int i = 0; i < light_count; i++) {
        if(sphere_aabb_overlap_test(lights[i].position_or_direction_and_radius, cluster_coords)) {
            count++;
        }
    }

    uint allocated_offset = atomicAdd(allocator_index, count);
    if(allocated_offset + count >= light_assignment_size) {
        atomicExchange(success, 0);
        return;
    }
    uint cluster_index = cluster_coords.x + cluster_coords.y * cluster_grid_size.x + cluster_coords.z * cluster_grid_size.x * cluster_grid_size.y;
    cluster_assignments[cluster_index].count = count;
    cluster_assignments[cluster_index].start_index = allocated_offset;

    uint write_index = 0;
    for(int i = 0; i < light_count && write_index < count; i++) {
        if(sphere_aabb_overlap_test(lights[i].position_or_direction_and_radius, cluster_coords)) {
            light_assignment_indicies[allocated_offset + write_index] = i;
            write_index++;
        }
    }
}

// #end