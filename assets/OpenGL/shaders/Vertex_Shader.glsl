#version 150

in vec2 pos;

uniform mat4 transform;

void main() {
	gl_Position = transform * vec4(pos,-1.0,1.0);
}