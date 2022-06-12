#Vertex //--------------------------------------------------
#version 410

in vec3 position;
in vec3 normal;

out vec4 out_normal;

uniform conf
{
	mat4 mvp_matrix;
	vec4 sun_direction;
	vec4 color;
	vec4 options;
};

void main() {
	gl_Position = mvp_matrix * vec4(position,1);
	out_normal = vec4(normal,0);
}


#end
#Fragment //------------------------------------------------
#version 410

uniform conf
{
	mat4 mvp_matrix;
	vec4 sun_direction;
	vec4 color;
	vec4 options;
};

in vec4 out_normal;

out vec4 out_color;

void main() {
	//out_color = abs(out_normal);


	out_color = color * (0.45 + max(0, dot(out_normal, sun_direction)));
	if (options.x == 1.0) {
		out_color = color;
	}
}

#end
