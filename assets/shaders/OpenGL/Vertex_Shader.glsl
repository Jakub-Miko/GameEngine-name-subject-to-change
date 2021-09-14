#version 150

in vec2 pos;

uniform vec2 position;
uniform vec2 size;

void main() {
	gl_Position = vec4(pos * size + position,0.0f, 1.0f);
}