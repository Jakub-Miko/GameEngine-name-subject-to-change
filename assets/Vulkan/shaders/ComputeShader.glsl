/*
#RootSignature
{
	"RootSignature": [
        {
            "name" : "ComputeSetting",
            "type" : "material",
            "material_path": "api:layouts/ComputeMatLayout.json"
        }
	]
}
#end
*/
// #Compute //--------------------------------------------------
#version 430

layout(set = 0, binding = 0) uniform Setting {
    int buffer_size;
    int block_count;
};

layout(std430, set=0, binding = 1) buffer data_buffer
{
    int numbers[1000];
};

layout(local_size_x = 1000, local_size_y = 1, local_size_z = 1) in;

void main() {
    numbers[0] = 42;
}

// #end