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
		},
		{
			"name" : "Overlay",
			"type" : "texture_2D"
		},
		{
			"type" : "push_constants",
			"size" : 4
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
layout(set = 0, binding = 2) uniform sampler2D Overlay;

layout(push_constant) uniform PushConstants {
		float exposure;
};

out vec4 color_out;

vec3 ToneMap(vec3 original) {
	return original / (1.0 + original);
}

void main() {
	vec4 overlay = texture(Overlay, uv_fragment);
	vec4 tonemap = vec4(ToneMap(texture(Color, uv_fragment).xyz * exposure),1);
	color_out = vec4(mix(tonemap.rgb, overlay.rgb, overlay.a),1.0f);
	gl_FragDepth = texture(Depth, uv_fragment).x;
}

#end
