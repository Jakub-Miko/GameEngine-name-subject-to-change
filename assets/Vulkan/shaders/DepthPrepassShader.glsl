#RootSignature
{
	"RootSignature": [
		{
			"type" : "push_constants",
			"size" : 64
		},
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec3 position;

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
};

layout( set = 0, binding = 0 ) uniform mvp{
	mat4 projection_matrix;
};

void main() {
	gl_Position = projection_matrix * mv_matrix * vec4(position, 1.0);
}


#end
#Fragment //------------------------------------------------
#version 430

void main() {

}

#end
