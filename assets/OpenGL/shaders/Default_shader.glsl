#Vertex //--------------------------------------------------
#version 150

in vec2 pos;
in vec2 uv;

out vec2 vertex_uv;

uniform Testblock
{
	vec4 in_color;
	vec2 test;
};

void main() {
	gl_Position = vec4(pos*0.5 + test, 1.0, 1.0);
	vertex_uv = uv;
}


#end
#Fragment //------------------------------------------------
#version 150

out vec4 color;

in vec2 vertex_uv;

uniform sampler2D TestTexture;

uniform Testblock{
	vec4 in_color;
	vec2 test;	
};

void main() {
	color = texture(TestTexture,vertex_uv);
}

#end