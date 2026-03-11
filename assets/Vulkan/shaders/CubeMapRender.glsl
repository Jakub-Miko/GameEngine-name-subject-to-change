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
#version 430

layout(location = 0) in vec2 position;

out vec3 screen_pos;

layout(set = 0, binding = 0) uniform mvp{
	mat4 inverse_view_projection;
	vec4 color_bias;
};


void main() {
	vec4 screenpos = inverse_view_projection * vec4(position.xy, 1.0, 1.0);
	gl_Position = vec4(position.xy, 1.0, 1.0);
	screen_pos = screenpos.xyz;
	screen_pos.y = -screen_pos.y;
}


#end

#Fragment //------------------------------------------------
#version 430

layout(set = 0, binding = 1) uniform samplerCube in_tex;

layout(location = 0) out vec4 color;
in vec3 screen_pos;

layout(set = 0, binding = 0) uniform mvp{
	mat4 inverse_view_projection;
	vec4 color_bias;
};


void main() {
	color = vec4(color_bias.xyz, 1.0)*textureLod(in_tex, normalize(screen_pos), 0.0) * color_bias.w;
}

#end
