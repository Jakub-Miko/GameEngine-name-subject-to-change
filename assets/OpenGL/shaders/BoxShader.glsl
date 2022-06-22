#Vertex //--------------------------------------------------
#version 410

in vec4 position;
in vec4 normal;

out vec4 out_normal;

uniform conf 
{
	mat4 mvp_matrix;
	mat4 model;
	vec4 sun_direction;
	vec4 color;
	vec4 options;
};

void main() {
	gl_Position = mvp_matrix * position;
	out_normal = normal;
}


#end
#Fragment //------------------------------------------------
#version 410

uniform conf 
{
	mat4 mvp_matrix;
	mat4 model;
	vec4 sun_direction;
	vec4 color;
	vec4 options;
};

in vec4 out_normal;

out vec4 out_color;

void main() {
	//out_color = abs(out_normal);
	
	vec3 Normal = normalize(mat3(transpose(inverse(model))) * out_normal.xyz);

	out_color = color * (0.45 + max(0,dot(vec4(Normal,0.0f),sun_direction)));
	if (options.x == 1.0) {
		out_color = color;
	}
}

#end
