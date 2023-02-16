#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "in_tex",
			"type" : "texture_2D"
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
	mat4 mvp_matrix[6];
};


void main() {
	gl_Position = vec4(position, 1.0);
}


#end
#Geometry //--------------------------------------------------
#version 410
layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 pos;

uniform mvp{
	mat4 mvp_matrix[6];
};


void main() {
	gl_Layer = gl_InvocationID;
	for (int i = 0; i < 3; i++) {
		pos = gl_in[i].gl_Position.xyz;
		gl_Position = mvp_matrix[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}


#end


#Fragment //------------------------------------------------
#version 410

uniform sampler2D in_tex;

out vec4 color;
in vec3 pos;

vec2 VectorToPolar(vec3 in_vec) {
	vec2 out_pol = vec2(atan(in_vec.z, in_vec.x), asin(in_vec.y));
	out_pol *= vec2(0.1591, 0.3183);
	out_pol += 0.5;
	return out_pol;
}

void main() {
	color = vec4(texture(in_tex, VectorToPolar(normalize(pos))).xyz,1.0f);
}

#end
