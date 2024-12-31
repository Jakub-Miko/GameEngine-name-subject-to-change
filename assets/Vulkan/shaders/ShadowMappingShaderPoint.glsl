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
	mat4 model;
	mat4 mvp_matrix[6];
	vec4 light_pos;
	float far_plane;
};


void main() {
	gl_Position = model * vec4(position, 1.0);
}


#end
#Geometry //--------------------------------------------------
#version 410
layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

out float pos;

uniform mvp{
	mat4 model;
	mat4 mvp_matrix[6];
	vec4 light_pos;
	float far_plane;
};


void main() {
	gl_Layer = gl_InvocationID;
	for (int i = 0; i < 3; i++) {
		pos = length(gl_in[i].gl_Position - light_pos) / far_plane;
		gl_Position = mvp_matrix[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}


#end


#Fragment //------------------------------------------------
#version 410


in float pos;

void main() {
	gl_FragDepth = pos;
}

#end
