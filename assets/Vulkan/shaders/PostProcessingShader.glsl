#RootSignature
{
	"RootSignature": [
		{
			"name" : "Color",
			"type" : "texture_2D"
		},
		{
			"name" : "Depth",
			"type" : "texture_2D"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 uv_fragment;

void main() {
	gl_Position = vec4(position,0.0,1.0);
	uv_fragment = uv;
	uv_fragment.y = 1 - uv_fragment.y;
}


#end
#Fragment //------------------------------------------------
#version 430

in vec2 uv_fragment;

layout(set = 0, binding = 0) uniform sampler2D Color;
layout(set = 0, binding = 1) uniform sampler2D Depth;

out vec4 color_out;

void main() {
	color_out = vec4(texture(Color, uv_fragment).xyz,1);
	gl_FragDepth = texture(Depth, uv_fragment).x;
}

#end
