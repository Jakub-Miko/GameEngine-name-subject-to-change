#Vertex //--------------------------------------------------
#version 150

in vec2 pos;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(pos, 1.0, 1.0);
}


#end
#Fragment //------------------------------------------------
#version 150

out vec4 color;

uniform vec4 in_color;

void main() {
	color = in_color;
}

#end