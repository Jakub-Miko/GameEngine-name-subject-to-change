#Vertex //--------------------------------------------------
#version 430

in vec4 position;
in vec4 normal;

out vec4 out_normal;

layout(binding = 0) uniform input
{
	mat4 mvp_matrix;
	vec4 sun_direction;
	vec4 color;
};

void main() {
	gl_Position = mvp_matrix * position;
	out_normal = normal;
}


#end
#Fragment //------------------------------------------------
#version 430

layout(binding = 0) uniform input
{
	mat4 mvp_matrix;
	vec4 sun_direction;
	vec4 color;
};

in vec4 out_normal;

out vec4 out_color;

void main() {
	//out_color = abs(out_normal);
	
	out_color = color * (0.45 + max(0,dot(out_normal,sun_direction)));
}

#end