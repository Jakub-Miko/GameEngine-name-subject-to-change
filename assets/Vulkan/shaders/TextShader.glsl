#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
		{
			"name" : "font_atlas",
			"type" : "texture_2D"
		}

	]
}
#end

#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv0;

out vec2 uvs;
layout(set = 0, binding = 0) uniform conf{
	mat4 mvp;
	float font_size;
};


void main() {
	gl_Position = mvp * vec4(position * font_size, -1.0f, 1.0f);
	uvs = uv0;
}

#end
#Fragment //------------------------------------------------
#version 430

in vec2 uvs;
out vec4 color;

layout(set = 0, binding = 1) uniform sampler2D font_atlas;


void main() {
	color = vec4(texture(font_atlas, uvs).r);
}

#end
