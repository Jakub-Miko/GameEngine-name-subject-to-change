#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "material",
			"type" : "constant_buffer",
			"material_visible" : true
		},
		{
			"name" : "Textures",
			"type" : "descriptor_table",
			"material_visible" : true,
			"ranges" : [
				{
					"size" : 1,
					"name" : "Textures",
					"type" : "texture_2D",
					"individual_names" : [
						"Color"
					]
				}
			]
		}
	],
	
	"constant_buffer_layouts" : [
			{
				"name" : "material",
				"layout" : [
					{
						"name" : "Base_Color",
						"type" : "VEC4"
					}
				]
			}
	],

	"default_material" : {
		"parameters": [
			{
				"name": "Base_Color",
				"type" : "VEC4",
				"value" : {
					"x" : 1.0,
					"y" : 1.0,
					"z" : 1.0,
					"w" : 1.0
				}
			},
			{
				"name": "Color",
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
layout(location = 3) in vec2 uv;

out vec2 uv_fragment;
out vec3 pos_fragment;
out vec3 normal_fragment;

uniform mvp{
	mat4 mvp_matrix;
	mat4 model_matrix;
};

uniform material{
	vec4 Base_Color;
};

void main() {
	pos_fragment = (model_matrix * vec4(position, 1.0)).xyz;
	gl_Position = mvp_matrix * vec4(position, 1.0);
	uv_fragment = uv;
	normal_fragment = normalize(mat3(transpose(inverse(model_matrix))) * normal.xyz).xyz;
}


#end
#Fragment //------------------------------------------------
#version 410

in vec2 uv_fragment;
in vec3 pos_fragment;
in vec3 normal_fragment;

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec4 position_out;
layout(location = 2) out vec4 normal_out;

uniform sampler2D Color;

uniform material{
	vec4 Base_Color;
};

void main() {
	color_out = vec4(texture(Color,uv_fragment).xyz,1) * Base_Color;
	position_out = vec4(pos_fragment.xyz, 1.0);
	normal_out = vec4(normal_fragment,0.0);
}

#end
