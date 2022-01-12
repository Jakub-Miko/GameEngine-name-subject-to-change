#Vertex //--------------------------------------------------
#version 150

in vec2 pos;

uniform Testblock
{
	vec4 in_color;
	vec2 test;
};

void main() {
	gl_Position = vec4(pos + test, 1.0, 1.0);
}


#end
#Fragment //------------------------------------------------
#version 150

out vec4 color;

uniform Testblock{
	vec4 in_color;
	vec2 test;	
};

void main() {
	color = in_color;
}

#end