#RootSignature
{
	"RootSignature": [
		{
			"name" : "Color",
			"type" : "texture_2D"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 uv_fragment;

void main() {
	gl_Position = vec4(position,0.0,1.0);
	uv_fragment = uv;
}


#end
#Fragment //------------------------------------------------
#version 410

in vec2 uv_fragment;

uniform sampler2D Color;

out vec4 color_out;

void main() {
	color_out = vec4(texture(Color, uv_fragment).xyz,1);
}

#end
