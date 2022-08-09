#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
			"type" : "constant_buffer"
		},
			{
			"name" : "mat",
			"type" : "constant_buffer",
			"material_visible" : true
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
					}
				]
			},
			{
				"name" : "mat",
				"layout" : [
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


	],

	"default_material" : {
		"parameters": [
			{
				"name": "sun_direction",
				"type" : "VEC4",
				"value" : {
					"x": 0.2,
					"y" : 0.5,
					"z" : 0.5,
					"w" : 0.0
				}
			},
			{
				"name": "color",
				"type" : "VEC4",
				"value" : {
					"x": 0.5,
					"y" : 0.7,
					"z" : 1.0,
					"w" : 1.0
				}
			},
			{
				"name": "options",
				"type" : "VEC4",
				"value" : {
					"x": 0.0,
					"y" : 0.0,
					"z" : 0.0,
					"w" : 0.0
				}
			},
			{
				"name": "Texture_First",
				"type" : "TEXTURE",
				"value" : "asset:image_texture.tex"
			},
			{
				"name": "Texture_Second",
				"type" : "TEXTURE",
				"value" : ""
			}
		]
	}
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv0;


out vec4 out_normal;
out vec2 uvs;

uniform conf
{
	mat4 mvp_matrix;
	mat4 model;
};

uniform mat
{
	vec4 sun_direction;
	vec4 color;
	vec4 options;
};

void main() {
	gl_Position = mvp_matrix * vec4(position,1);
	out_normal = vec4(normal,0);
	uvs = uv0;
}


#end
#Fragment //------------------------------------------------
#version 410

uniform conf
{
	mat4 mvp_matrix;
	mat4 model;
};

uniform mat
{
	vec4 sun_direction;
	vec4 color;
	vec4 options;
};

in vec4 out_normal;
in vec2 uvs;

out vec4 out_color;

uniform sampler2D Texture_First;
uniform sampler2D Texture_Second;

void main() {
	//out_color = abs(out_normal);
	vec3 Normal = normalize(mat3(transpose(inverse(model))) * out_normal.xyz);

	out_color = vec4(texture(Texture_First,uvs).xyz,1) * 5 * (0.45 + max(0, dot(vec4(Normal, 0.0f), sun_direction))) * texture(Texture_First,vec2(0.5,0.5)) * texture(Texture_Second, vec2(0.5, 0.5));
	if (options.x == 1.0) {
		out_color = color;
	}
}

#end
