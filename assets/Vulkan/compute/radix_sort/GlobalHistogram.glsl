/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "GlobalHistogramSettings",
            "type" : "material",
            "material_inline": [
                {
                  "name": "input_buffer",
                  "type": "storage_buffer"
                },
                {
                  "name": "histogram_buffer",
                  "type": "storage_buffer"
                },
                {
                  "name": "input_size",
                  "type": "INT"
                },
                {
                  "name": "block_count",
                  "type": "INT"
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
#define THREADS_PER_THREADBLOCK 256
#define ITEMS_PER_THREAD 16
#define ITEMS_PER_THREADBLOCK ITEMS_PER_THREAD * THREADS_PER_THREADBLOCK
#define SUBGROUP_SIZE 32

layout(set = 0, binding = 0) uniform Setting {
    int input_size;
    int block_count;
};

layout(std430, set=0, binding = 1) buffer input_buffer_block
{
    int input_buffer[];
};

struct HistogramEntry {
    uint digits[DIGIT_COUNT];
};


layout(std430, set=0, binding = 2) buffer histogram_buffer_block
{
    HistogramEntry histogram_buffer[HISTOGRAM_SIZE];
};

layout(std430, set=0, binding = 3) buffer validation_buffer_block
{
    HistogramEntry validation_buffer[HISTOGRAM_SIZE];
};

layout(local_size_x = THREADS_PER_THREADBLOCK, local_size_y = 1, local_size_z = 1) in;


shared HistogramEntry histogram[HISTOGRAM_SIZE];

uvec2 GetSubgroupCountsAndRanks(int key) {

    int count = 0;
    int rank = 0;
    #if SUBGROUP_SIZE == 32

    uint thread_mask = subgroupBallot(true).x;
    for(int i = 0; i < HISTOGRAM_BITS; i++) {
        bool bit = ((1 << i) & key) == 0 ? false : true;
        uint digit_mask = subgroupBallot(bit).x;
        thread_mask &= (bit ? 0 : ~uint(0)) ^ digit_mask;
    }
    count = bitCount(thread_mask);
    rank = bitCount(thread_mask & gl_SubgroupLtMask.x);

    #elif SUBGROUP_SIZE == 64

    uvec2 thread_mask = subgroupBallot(true).xy;
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
    for(uint i = gl_LocalInvocationIndex; i < HISTOGRAM_SIZE*DIGIT_COUNT; i += THREADS_PER_THREADBLOCK) {
        #pragma unroll
        for(int digit = 0; digit < DIGIT_COUNT; digit++) {
            histogram[i].digits[digit] = 0;
        }
    }

    uint end = gl_WorkGroupID.x == (block_count - 1) ? input_size : (gl_WorkGroupID.x + 1) * ITEMS_PER_THREADBLOCK;

    for(uint index = gl_LocalInvocationIndex + gl_WorkGroupID.x * ITEMS_PER_THREADBLOCK; index < end; index += THREADS_PER_THREADBLOCK) {
        int key = input_buffer[index];
        #pragma unroll
        for(int digit = 0; digit < DIGIT_COUNT; digit++) {
            uint extracted_digit = (uint(key) >> (digit*HISTOGRAM_BITS)) & HISTOGRAM_BITS_MASK;
            uvec2 count_and_rank = GetSubgroupCountsAndRanks(int(extracted_digit));
            if(count_and_rank.y == 0) {
                atomicAdd(histogram[extracted_digit].digits[digit], count_and_rank.x);
            }
        }
    }

    memoryBarrierShared();
    barrier();

    for(uint i = gl_LocalInvocationIndex; i < HISTOGRAM_SIZE; i += THREADS_PER_THREADBLOCK) {
        #pragma unroll
        for(int digit = 0; digit < DIGIT_COUNT; digit++) {
            atomicAdd(histogram_buffer[i].digits[digit], histogram[i].digits[digit]);
        }
    }

}

// #end