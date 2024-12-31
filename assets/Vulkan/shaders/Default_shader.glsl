#Vertex //--------------------------------------------------
#version 410

in vec2 pos;
in vec2 uv;

out vec2 vertex_uv;

uniform Testblock_0
{
	vec4 in_color;
	vec2 scale;
};

void main() {
	gl_Position = vec4(pos * scale, 1.0, 1.0);
	vertex_uv = uv;
}


#end
#Fragment //------------------------------------------------
#version 410

out vec4 color;

in vec2 vertex_uv;

uniform sampler2D TestTexture[1];

uniform Testblock_0 {
	vec4 in_color;
	vec2 scale;	
};

void main() {
	color = texture(TestTexture[0],vertex_uv);
	
}

#end
