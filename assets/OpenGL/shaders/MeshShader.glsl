#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
		{
			"name" : "table",
			"type" : "descriptor_table",
			"material_visible" : true,
			"ranges" : [
				{
					"size" : 2,
					"name" : "textures",
					"type" : "texture_2D",
					"individual_names" : [
						"Texture_First", "Texture_Second"
					]
				}

			]
		}

	], 

	"constant_buffer_layouts" : [
			{
				"name" : "conf",
				"layout" : [
					{
						"name" : "mvp_matrix",
						"type" : "MAT4"
					},
					{
						"name" : "model",
						"type" : "MAT4"
					},
					{
						"name" : "sun_direction",
						"type" : "VEC4"
					},
					{
						"name" : "color",
						"type" : "VEC4"
					},
					{
						"name" : "options",
						"type" : "VEC4"
					}
				]
			}


	]
}
#end

#Vertex //--------------------------------------------------
#version 410

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 uv0;


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
	gl_Position = mvp_matrix * vec4(position,1);
	out_normal = vec4(normal,0);
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

uniform sampler2D Texture_First;
uniform sampler2D Texture_Second;

void main() {
	//out_color = abs(out_normal);
	vec3 Normal = normalize(mat3(transpose(inverse(model))) * out_normal.xyz);

	out_color = color * (0.45 + max(0, dot(vec4(Normal, 0.0f), sun_direction))) * texture(Texture_First,vec2(0.5,0.5)) * texture(Texture_Second, vec2(0.5, 0.5));
	if (options.x == 1.0) {
		out_color = color;
	}
}

#end
