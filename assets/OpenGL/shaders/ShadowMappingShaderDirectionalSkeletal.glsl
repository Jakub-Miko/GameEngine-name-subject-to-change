#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "bones",
			"type" : "constant_buffer"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in uvec4 bone_ids;
layout(location = 1) in vec4 bone_weights;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec2 uv;


uniform mvp{
	mat4 model_matrix;
	mat4 vp_matrix[5];
};

uniform bones{
	mat4 bone_matricies[80];
	uint valid;
};

void main() {
	vec4 new_pos = vec4(0.0f);
	if (valid == 1) {
		for (int i = 0; i < 4; i++) {
			if (bone_ids[i] != -1) {
				new_pos += bone_weights[i] * (bone_matricies[bone_ids[i]] * vec4(position, 1.0f));
			}
		}
	}
	else {
		new_pos = vec4(position, 1.0f);
	}

	gl_Position = model_matrix * vec4(new_pos.xyz, 1.0f);
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
