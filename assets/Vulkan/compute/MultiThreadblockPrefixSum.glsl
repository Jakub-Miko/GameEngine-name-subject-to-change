/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "MultiThreadblockPrefixSumSettings",
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
                  "name": "state_buffer",
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

#define NUM_OF_THREADBLOCKS 2000
#define NUM_OF_THREADS_IN_THREADBLOCK 512
#define NUM_OF_THREADS NUM_OF_THREADBLOCKS * NUM_OF_THREADS_IN_THREADBLOCK
#define STATE_BUFFER_SIZE 4 * NUM_OF_THREADBLOCKS

layout(set = 0, binding = 0) uniform Setting {
    int buffer_size;
    int block_count;
};

layout(std430, set=0, binding = 1) buffer data_buffer
{
    int numbers[NUM_OF_THREADS];
};

layout(std430, set=0, binding = 2) buffer output_buffer
{
    int output_numbers[NUM_OF_THREADS];
};

layout(std430, set=0, binding = 3) buffer validation_buffer
{
    int validation_numbers[NUM_OF_THREADS];
};

struct StateBlock {
    uint state_flag;
};

layout(std430, set=0, binding = 4) buffer state_buffer
{
    uint work_group_counter;
    StateBlock state_blocks[STATE_BUFFER_SIZE];
};

layout(local_size_x = NUM_OF_THREADS_IN_THREADBLOCK, local_size_y = 1, local_size_z = 1) in;

shared uint reductions[32];
shared uint inclusive_group_prefix;
shared uint group_id;

void main() {
    if(gl_LocalInvocationIndex == NUM_OF_THREADS_IN_THREADBLOCK-1) {
        group_id = atomicAdd(work_group_counter, 1);
    }
    memoryBarrierShared();
    barrier();
    int value = numbers[gl_LocalInvocationIndex + group_id * NUM_OF_THREADS_IN_THREADBLOCK];

    uint subgroup_prefix = subgroupExclusiveAdd(value);
    if(gl_SubgroupInvocationID == gl_SubgroupSize - 1) {
        reductions[gl_SubgroupID] = subgroup_prefix + value;
    }

    memoryBarrierShared();
    barrier();

    if(gl_LocalInvocationIndex < ceil(float(NUM_OF_THREADS_IN_THREADBLOCK) / gl_SubgroupSize)) {
        reductions[gl_LocalInvocationIndex] = subgroupExclusiveAdd(reductions[gl_LocalInvocationIndex]);
    }

    memoryBarrierShared();
    barrier();

    if(gl_LocalInvocationIndex == NUM_OF_THREADS_IN_THREADBLOCK-1) {
        uint exclusive_prefix = reductions[gl_SubgroupID] + subgroup_prefix + value;
        //memoryBarrierBuffer();
        atomicExchange(state_blocks[group_id].state_flag, exclusive_prefix << 2 | 1);
        uint aggregate = 0;
        for(int block = int(group_id - 1); block >= 0; block--) {
            uint flag = 0;
            while(((flag = atomicAdd(state_blocks[block].state_flag, 0)) & 3) == 0);
            aggregate += flag >> 2;
            if((flag & 3) == 2) {
                break;
            }
        }
        atomicExchange(state_blocks[group_id].state_flag, (exclusive_prefix + aggregate) << 2 | 2);
        //memoryBarrierBuffer();
        inclusive_group_prefix = aggregate;
    }

    memoryBarrierShared();
    barrier();

    subgroup_prefix += inclusive_group_prefix + reductions[gl_SubgroupID];

    output_numbers[gl_LocalInvocationIndex + group_id * NUM_OF_THREADS_IN_THREADBLOCK] = int(subgroup_prefix);

}

// #end