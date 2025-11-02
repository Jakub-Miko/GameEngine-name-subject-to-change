/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "DigitBinningSettings",
            "type" : "material",
            "material_inline": [
                {
                    "name": "input_prefix_sum",
                    "type": "storage_buffer"
                },
                {
                    "name": "input_buffer",
                    "type": "storage_buffer"
                },
                {
                    "name": "output_buffer",
                    "type": "storage_buffer"
                },
                {
                    "name": "digit_binning_data",
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
                  "name": "shift",
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

#define HISTOGRAM_SIZE 256
#define HISTOGRAM_BITS 8
#define HISTOGRAM_BITS_MASK 0x0FF
#define DIGIT_COUNT 4
#define THREADS_PER_THREADBLOCK 256
#define ITEMS_PER_THREAD 16
#define ITEMS_PER_THREADBLOCK ITEMS_PER_THREAD * THREADS_PER_THREADBLOCK
#define SUBGROUP_SIZE 32
#define SUBGOURPS_IN_THREADBLOCK ((THREADS_PER_THREADBLOCK + SUBGROUP_SIZE - 1) / SUBGROUP_SIZE)

struct PrefixSumEntry {
    uint digits[DIGIT_COUNT];
};

struct HistogramEntry {
    uint digits[DIGIT_COUNT];
};

struct ThreadBlockInfo {
    uint thread_block_reductions[256];
};

layout(set = 0, binding = 0) uniform Setting {
    int input_size;
    int block_count;
    int shift;
};

layout(std430, set=0, binding = 1) buffer input_prefix_sum_block
{
    PrefixSumEntry input_prefix_sum[HISTOGRAM_SIZE];
};

layout(std430, set=0, binding = 2) buffer input_buffer_block
{
    uint input_buffer[];
};

layout(std430, set=0, binding = 3) buffer output_buffer_block
{
    uint output_buffer[];
};

layout(std430, set=0, binding = 4) buffer digit_binning_data_block
{
    uint thread_block_number_counter[DIGIT_COUNT];
    ThreadBlockInfo thread_block_info[];
};

struct SubgroupHistogram {
    uint entry[HISTOGRAM_SIZE];
};

struct ThreadLocalData {
    SubgroupHistogram subgroup_histograms[SUBGOURPS_IN_THREADBLOCK];
};

shared ThreadLocalData thread_local_data;
shared uint block_global_offsets[HISTOGRAM_SIZE];
shared uint local_sort[ITEMS_PER_THREADBLOCK];
shared uint thread_block_number;

uint get_digit(uint key) {
    return (key >> shift) & ((1 << HISTOGRAM_BITS)-1);
}


// every iteration the final prefix flag swaps values, so the next iteration doesnt see it as valid.
uint prefix_flag() {
    return ((shift >> 3) & 1) == 0 ? 2 : 3;
}

layout(local_size_x = THREADS_PER_THREADBLOCK, local_size_y = 1, local_size_z = 1) in;
void main() {
    uint block_index;
    uint keys[ITEMS_PER_THREAD];
    uint offsets[ITEMS_PER_THREAD];

    // Assign dynamic ID to each threadblock to ensure the order in which they began execution

    if(gl_LocalInvocationIndex == 0) {
        thread_block_number = atomicAdd(thread_block_number_counter[shift >> 3],1);
    }
    memoryBarrierShared();
    barrier();

    block_index = thread_block_number;

    //Load keys for every thread
    if(block_index < block_count - 1) {
        uint index = gl_SubgroupInvocationID + gl_SubgroupID * SUBGROUP_SIZE * ITEMS_PER_THREAD + block_index * ITEMS_PER_THREADBLOCK;
        for(int i = 0; i < ITEMS_PER_THREAD; i++) {
            keys[i] = input_buffer[index];
            index += SUBGROUP_SIZE;
        }
    }
    if(block_index == block_count - 1) {
        uint index = gl_SubgroupInvocationID + gl_SubgroupID * SUBGROUP_SIZE * ITEMS_PER_THREAD + block_index * ITEMS_PER_THREADBLOCK;
        for(int i = 0; i < ITEMS_PER_THREAD; i++) {
            keys[i] = index < input_size ? input_buffer[index] : 0xffffffff;
            index += SUBGROUP_SIZE;
        }
    }

    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE) {
        for (uint i = 0; i < SUBGOURPS_IN_THREADBLOCK; i++) {
            thread_local_data.subgroup_histograms[i].entry[gl_LocalInvocationIndex] = 0;
        }
        block_global_offsets[gl_LocalInvocationIndex] = input_prefix_sum[gl_LocalInvocationIndex].digits[shift >> 3];
    }

    // Perform per subgroup histogram and offset gathering and store into shared memory
    for(int i = 0; i < ITEMS_PER_THREAD; i++) {
        //Subgroup ranking (Wave multi-split)
        uint flags = 0xffffffff;
        for(int digit_bit = 0; digit_bit < HISTOGRAM_BITS; digit_bit++) {
            bool bit_value = (uint(keys[i]) >> (shift + digit_bit) & 1) == 0 ? false : true;
            uint mask = subgroupBallot(bit_value).x;
            flags &= (bit_value ? 0x0 : 0xffffffff) ^ mask;
        }
        uint rank = bitCount(flags & gl_SubgroupLtMask.x);

        //add offset within subgroup
        offsets[i] = thread_local_data.subgroup_histograms[gl_SubgroupID].entry[get_digit(keys[i])] + rank;
        memoryBarrierShared();
        barrier(); // TODO: this might not be neccesary, verify
        if(rank == 0) {
            thread_local_data.subgroup_histograms[gl_SubgroupID].entry[get_digit(keys[i])] += bitCount(flags);
        }
        memoryBarrierShared();
        barrier(); // TODO: this might not be neccesary, verify
    }

    //Perform prefix sum over the per subgroup reductions to get subgroup offsets
    uint reduction = 0;
    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE) {
        //iterate over all subgroup local data in the threadblock and perform prefix some for each histogram index.
        reduction = thread_local_data.subgroup_histograms[0].entry[gl_LocalInvocationIndex];
        for(uint i = 1; i < SUBGOURPS_IN_THREADBLOCK; i++) {
            reduction += thread_local_data.subgroup_histograms[i].entry[gl_LocalInvocationIndex];
            thread_local_data.subgroup_histograms[i].entry[gl_LocalInvocationIndex] = reduction - thread_local_data.subgroup_histograms[i].entry[gl_LocalInvocationIndex];
        }

        //have all but the last threadgroup publish per-histogram reductions of the threadgroup to global memory for decoupled lookback
        if(block_index < block_count - 1) {
            atomicExchange(thread_block_info[block_index].thread_block_reductions[gl_LocalInvocationIndex], 1 | (reduction << 2));
        }

        // Calculate INCLUSIVE(note we are adding not setting) prefix sum over the reductions pre subgroup,
        // this will be needed to sort data in shared memory,
        reduction += subgroupExclusiveAdd(reduction);
    }
    memoryBarrierShared();
    barrier();


    //Use the local subgroup histogram 0 (since in exclusive prefix its always 0 and free to use) to store the prefix
    //sum over the threadgroup global digit histograms.
    //The first subgroup local prefix was already calculated in the reduction variable, now we upload them into memory
    //and merge them.
    // NOTE: the weird bit operation rotates indicies withing warps, so the last index becomes first
    // this means that the reduction of the subgroup(last element of inclusive sum) is stored as the first key, and
    // the remaining keys are identical to EXCLUSIVE SUM.
    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE) {
        thread_local_data.subgroup_histograms[0]
                .entry[((gl_SubgroupInvocationID + 1) & (SUBGROUP_SIZE-1)) + (gl_LocalInvocationIndex & ~(SUBGROUP_SIZE - 1))] = reduction;
    }
    memoryBarrierShared();
    barrier();

    // perform prefix sum over subgroup local reduction.
    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE / SUBGROUP_SIZE) {
        thread_local_data.subgroup_histograms[0].entry[gl_LocalInvocationIndex * SUBGROUP_SIZE] =
            subgroupExclusiveAdd(thread_local_data.subgroup_histograms[0].entry[gl_LocalInvocationIndex * SUBGROUP_SIZE]);
    }
    memoryBarrierShared();
    barrier();

    // For all items except the the first ones, add the subgroup reductions to the subgroup local prefix sum to get,
    // threadgroup local prefix sum
    // Node whe shift back the indicies by one again so the first thread end up reading and broadcasting the reduction to
    // add to all other threads in the subgroup
    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE && gl_SubgroupInvocationID != 0) {
        thread_local_data.subgroup_histograms[0].entry[gl_LocalInvocationIndex] +=
            subgroupBroadcast(thread_local_data.subgroup_histograms[0].entry[gl_LocalInvocationIndex -1],1);
    }
    memoryBarrierShared();
    barrier();


    //For all threads, compute final threadlocal offsets to scatter the keys into (for local sorting)
    if(gl_LocalInvocationIndex >= SUBGROUP_SIZE) {
        for(int i = 0; i < ITEMS_PER_THREAD; i++) {
            const uint digit = get_digit(keys[i]);
            offsets[i] += thread_local_data.subgroup_histograms[gl_SubgroupID].entry[digit] +
                    thread_local_data.subgroup_histograms[0].entry[digit];
        }
    } else {
        for(int i = 0; i < ITEMS_PER_THREAD; i++) {
            const uint digit = get_digit(keys[i]);
            offsets[i] += thread_local_data.subgroup_histograms[0].entry[digit];
        }
    }

    //This is contains the offsets where individual digits will begin in the sorted threadblock
    uint exclusive_hist_reduction;
    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE) {
        exclusive_hist_reduction = thread_local_data.subgroup_histograms[0].entry[gl_LocalInvocationIndex];
    }

    memoryBarrierShared();
    barrier();

    //sort locally per threadblock
    for(uint i = 0; i < ITEMS_PER_THREAD; i++) {
        local_sort[offsets[i]] = keys[i];
    }

    //decoupled lookback
    if(gl_LocalInvocationIndex < HISTOGRAM_SIZE) {
        uint reduction = 0;
        int block = int(block_index - 1);
        while(block >= 0) {
            uint block_digit_data = atomicAdd( thread_block_info[block].thread_block_reductions[gl_LocalInvocationIndex], 0);
            if((block_digit_data & 3) == 1) {
                reduction += block_digit_data >> 2;
                block--;
            } else if( (block_digit_data & 3) == prefix_flag()) {
                reduction += block_digit_data >> 2;
                // we use flag 1(REDUCTION) instead of two besause we are adding, and before it was already 1,
                // this means after adding the flag is going to be 2(PREFIX) as intended.
                break;
            }
        }
        if(block_index < block_count - 1) {
            atomicAdd(thread_block_info[block_index].thread_block_reductions[gl_LocalInvocationIndex], (prefix_flag() - 1) | (reduction << 2));
        }
        block_global_offsets[gl_LocalInvocationIndex] += reduction - exclusive_hist_reduction; // we subtract the start of the digit in the local sort
    }
    memoryBarrierShared();
    barrier();

    //final sort
    if(block_index < block_count - 1) {
        for(uint i = gl_LocalInvocationIndex; i < ITEMS_PER_THREADBLOCK; i += THREADS_PER_THREADBLOCK) {
            output_buffer[block_global_offsets[get_digit(local_sort[i])] + i] = local_sort[i];
        }
    }

    if(block_index == block_count - 1) {
        uint last_index = input_size - block_index * ITEMS_PER_THREADBLOCK;
        for(uint i = gl_LocalInvocationIndex; i < last_index; i += THREADS_PER_THREADBLOCK) {
            output_buffer[block_global_offsets[get_digit(local_sort[i])] + i] = local_sort[i];
        }
    }

}

// #end