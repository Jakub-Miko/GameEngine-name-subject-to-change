#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;


uniform mvp{
	mat4 mvp_matrix;
};


void main() {
	gl_Position = mvp_matrix * vec4(position, 1.0);
}


#end
#Fragment //------------------------------------------------
#version 410

void main() {

}

#end
