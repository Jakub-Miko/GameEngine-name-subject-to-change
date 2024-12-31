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
	mat4 model_matrix;
	mat4 vp_matrix[5];
};


void main() {
	gl_Position = model_matrix * vec4(position, 1.0);
}


#end
#Geometry //--------------------------------------------------
#version 410
layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mvp{
	mat4 model_matrix;
	mat4 vp_matrix[5];
};


void main() {
	gl_Layer = gl_InvocationID;
	for (int i = 0; i < 3; i++) {
		gl_Position = vp_matrix[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}


#end

#Fragment //------------------------------------------------
#version 410

void main() {

}

#end
