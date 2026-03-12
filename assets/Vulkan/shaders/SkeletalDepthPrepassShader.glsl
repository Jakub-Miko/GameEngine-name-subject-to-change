#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"type" : "push_constants",
			"size" : 64
		},
		{
			"name" : "bones",
			"type" : "constant_buffer"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in uvec4 bone_ids;
layout(location = 1) in vec4 bone_weights;
layout(location = 2) in vec3 position;

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
};

layout( set = 0, binding = 0 ) uniform mvp{
	mat4 projection_matrix;
};

layout(set = 0, binding = 1) uniform bones {
	mat4 bone_matricies[100];
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
	gl_Position = projection_matrix * mv_matrix * vec4(new_pos.xyz, 1.0f);
}


#end
#Fragment //------------------------------------------------
#version 430

void main() {

}

#end
