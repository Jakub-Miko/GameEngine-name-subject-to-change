#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		}
	],
	
	"default_material" : {
		"parameters": [
			
		]
	}
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

out vec2 uv_fragment;

uniform mvp{
	mat4 mvp_matrix;
};

void main() {
	gl_Position = mvp_matrix * vec4(position,1.0);
	uv_fragment = uv;
}


#end
#Fragment //------------------------------------------------
#version 410

in vec2 uv_fragment;

out vec4 color_out;

void main() {
	color_out = vec4(1);
}

#end
