/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "GlobalPrefixSumSettings",
            "type" : "material",
            "material_inline": [
                {
                  "name": "input_histogram",
                  "type": "storage_buffer"
                },
                {
                  "name": "output_prefix_sum",
                  "type": "storage_buffer"
                },
                {
                    "name": "validate_buffer",
                    "type": "storage_buffer"
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

#define HISTOGRAM_SIZE 256
#define HISTOGRAM_BITS 8
#define HISTOGRAM_BITS_MASK 0x0FF
#define DIGIT_COUNT 4

struct HistogramEntry {
    uint digits[DIGIT_COUNT];
};

struct PrefixSumEntry {
    uint digits[DIGIT_COUNT];
};

layout(std430, set=0, binding = 0) buffer input_histogram_block
{
    HistogramEntry input_histogram[HISTOGRAM_SIZE];
};

layout(std430, set=0, binding = 1) buffer output_prefix_sum_block
{
    PrefixSumEntry output_prefix_sum[HISTOGRAM_SIZE];
};

layout(std430, set=0, binding = 2) buffer validation_buffer_block
{
    HistogramEntry validation_buffer[HISTOGRAM_SIZE];
};


shared uint reductions[32];

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;
void main() {
    uint value = input_histogram[gl_LocalInvocationIndex].digits[gl_WorkGroupID.x];

    uint subgroup_prefix = subgroupExclusiveAdd(value);
    if(gl_SubgroupInvocationID == gl_SubgroupSize - 1) {
        reductions[gl_SubgroupID] = subgroup_prefix + value;
    }
    groupMemoryBarrier();
    barrier();

    if(gl_LocalInvocationIndex < ceil(256 / gl_SubgroupSize)) {
        reductions[gl_LocalInvocationIndex] = subgroupExclusiveAdd(reductions[gl_LocalInvocationIndex]);
    }

    groupMemoryBarrier();
    barrier();

    subgroup_prefix += reductions[gl_SubgroupID];

    output_prefix_sum[gl_LocalInvocationIndex].digits[gl_WorkGroupID.x] = subgroup_prefix;

}

// #end