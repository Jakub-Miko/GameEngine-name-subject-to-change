#RootSignature
{
	"RootSignature": [
		{
			"name" : "buffer",
			"type" : "storage_buffer"
		}
	]
}
#end

#Compute //--------------------------------------------------
#version 430

layout(std430, binding = 0) buffer Storage
{
    int numbers[1000];
};

layout(local_size_x = 10, local_size_y = 10, local_size_z = 10) in;

void main() {
    numbers[1] = 43;
}

#end