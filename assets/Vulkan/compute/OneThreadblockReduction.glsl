/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "OneThreadblockReductionSettings",
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
                  "name": "validation_buffer",
                  "type": "storage_buffer"
                },
                {
                  "name": "buffer_size",
                  "type": "INT"
                },
                {
                  "name" : "block_count",
                  "type" : "INT"
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

#define HISTOGRAM_SIZE 11
#define HISTOGRAM_BITS 4
#define SUBGROUP_SIZE 32

layout(set = 0, binding = 0) uniform Setting {
    int buffer_size;
    int block_count;
};

layout(std430, set=0, binding = 1) buffer data_buffer
{
    int numbers[1000];
};

layout(std430, set=0, binding = 2) buffer output_buffer
{
    int output_numbers[1000];
};

layout(std430, set=0, binding = 3) buffer validation_buffer
{
    int validation_numbers[1000];
};

layout(local_size_x = 1000, local_size_y = 1, local_size_z = 1) in;

shared int histogram[HISTOGRAM_SIZE];

uvec2 GetSubgroupCountsAndRanks(int key) {

    int count = 0;
    int rank = 0;
    #if SUBGROUP_SIZE == 32

    uint thread_mask = ~uint(0);
    for(int i = 0; i < HISTOGRAM_BITS; i++) {
        bool bit = ((1 << i) & key) == 0 ? false : true;
        uint digit_mask = subgroupBallot(bit).x;
        thread_mask &= (bit ? 0 : ~uint(0)) ^ digit_mask;
    }
    count = bitCount(thread_mask);
    rank = bitCount(thread_mask & gl_SubgroupLtMask.x);

    #elif SUBGROUP_SIZE == 64

    uvec2 thread_mask = uvec2(~uint(0), ~uint(0));
    for(int i = 0; i < HISTOGRAM_BITS; i++) {
        bool bit = ((1 << i) & key) == 0 ? false : true;
        uint digit_mask = subgroupBallot(bit).xy;
        thread_mask &= (bit ? 0 : uvec2(~uint(0), ~uint(0))) ^ digit_mask;
    }
    count = bitCount(thread_mask);
    rank = bitCount(thread_mask & gl_SubgroupLtMask.xy);

    #endif

    return uvec2(count, rank);
}

void main() {
    int key = numbers[gl_LocalInvocationIndex];

    uvec2 count_and_rank = GetSubgroupCountsAndRanks(key);
    if(count_and_rank.y == 0) {
        atomicAdd(histogram[key], int(count_and_rank.x));
    }

    memoryBarrier();
    barrier();

    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE) {
        output_numbers[gl_LocalInvocationIndex] = histogram[gl_LocalInvocationIndex];
    }

}

// #end