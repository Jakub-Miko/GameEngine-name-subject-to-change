/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "OneThreadblockStreamCompactionSettings",
            "type" : "material",
            "material_inline": [
                {
                  "name": "data_buffer",
                  "type": "storage_buffer"
                },
                {
                  "name": "output_buffer",
                  "type": "storage_buffer"
                },
                {
                  "name": "buffer_size",
                  "type": "INT"
                },
                {
                  "name" : "block_count",
                  "type" : "INT"
                },
                {
                  "name": "key_lookup",
                  "type": "INT"
                }
            ]
        }
	]
}
#end
*/
// #Compute //--------------------------------------------------
#version 430
#extension GL_KHR_shader_subgroup_vote: enable
#extension GL_KHR_shader_subgroup_ballot: enable
#extension GL_KHR_shader_subgroup_basic: enable
#extension GL_KHR_shader_subgroup_arithmetic: enable

layout(set = 0, binding = 0) uniform Setting {
    int buffer_size;
    int block_count;
    int key_lookup;
};

layout(std430, set=0, binding = 1) buffer data_buffer
{
    int numbers[1000];
};

layout(std430, set=0, binding = 2) buffer output_buffer
{
    int output_numbers[1000];
};


layout(local_size_x = 1000, local_size_y = 1, local_size_z = 1) in;

shared int reductions[32];

void main() {
    int value = numbers[gl_LocalInvocationIndex];
    int key = value % 10;

    bool is_key = bool(key == key_lookup);
    int subgroup_prefix = subgroupExclusiveAdd(is_key ? 1 : 0);
    if(gl_SubgroupInvocationID == gl_SubgroupSize - 1) {
        reductions[gl_SubgroupID] = subgroup_prefix + (is_key ? 1 : 0);
    }
    groupMemoryBarrier();
    barrier();

    if(gl_LocalInvocationIndex < ceil(1000.0 / gl_SubgroupSize)) {
        reductions[gl_LocalInvocationIndex] = subgroupExclusiveAdd(reductions[gl_LocalInvocationIndex]);
    }

    groupMemoryBarrier();
    barrier();

    subgroup_prefix += reductions[gl_SubgroupID];

    if(is_key) {
        output_numbers[subgroup_prefix] = value;
    }

}

// #end