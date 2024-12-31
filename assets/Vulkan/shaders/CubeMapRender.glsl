#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "in_tex",
			"type" : "texture_2D_cubemap"
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

out vec3 screen_pos;

uniform mvp{
	mat4 inverse_view_projection;
	vec4 color_bias;
};


void main() {
	vec4 screenpos = inverse_view_projection * vec4(position.xy, 1.0, 1.0);
	gl_Position = vec4(position.xy, 1.0, 1.0);
	screen_pos = screenpos.xyz;
}


#end

#Fragment //------------------------------------------------
#version 410

uniform samplerCube in_tex;

layout(location = 0) out vec4 color;
in vec3 screen_pos;

uniform mvp{
	mat4 inverse_view_projection;
	vec4 color_bias;
};


void main() {
	color = vec4(color_bias.xyz, 1.0)*textureLod(in_tex, normalize(screen_pos), 0.0) * color_bias.w;
}

#end
