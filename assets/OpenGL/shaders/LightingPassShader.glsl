#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "light_props",
			"type" : "constant_buffer",
			"material_visible" : true
		},
		{
			"name" : "G_Buffer",
			"type" : "descriptor_table",
			"material_visible" : true,
			"ranges" : [
				{
					"size" : 3,
					"name" : "Textures",
					"type" : "texture_2D",
					"individual_names" : [
						"Color", "World_Position", "Normal"
					]
				}
			]
		}
	],
	
	"constant_buffer_layouts" : [
			{
				"name" : "light_props",
				"layout" : [
					{
						"name" : "Light_Color",
						"type" : "VEC4"
					},
					{
						"name" : "attenuation",
						"type" : "VEC4"
					},
					{
						"name" : "pixel_size",
						"type" : "VEC2"
					},
					{
						"name" : "light_type",
						"type" : "INT"
					}
				]
			}
	],

	"default_material" : {
		"parameters": [
			{
				"name": "Light_Color",
				"type" : "VEC4",
				"value" : {
					"x" : 1.0,
					"y" : 1.0,
					"z" : 1.0,
					"w" : 1.0
				}
			},
			{
				"name": "attenuation",
				"type" : "VEC4",
				"value" : {
					"x" : 1.0,
					"y" : 0.1,
					"z" : 0.01,
					"w" : 0.0
				}
			},
			{
				"name": "light_type",
				"type" : "INT",
				"value" : 1
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

uniform mvp{
	mat4 mvp_matrix;
	mat4 model_matrix;
};

uniform light_props{
	vec4 Light_Color;
	vec4 attenuation_constants;
	vec2 pixel_size;
	int light_type;
};

void main() {
	gl_Position = mvp_matrix * vec4(position, 1.0);
}


#end
#Fragment //------------------------------------------------
#version 410

layout(location = 0) out vec4 color_out;

uniform sampler2D Color;
uniform sampler2D World_Position;
uniform sampler2D Normal;

uniform mvp{
	mat4 mvp_matrix;
	mat4 model_matrix;
};

uniform light_props{
	vec4 Light_Color;
	vec4 attenuation_constants;
	vec2 pixel_size;
	int light_type;
};

void main() {
	vec3 pos = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 world_space_pos = texture(World_Position, pos.xy).xyz;
	vec3 light_direction;
	vec3 light_pos = (model_matrix * vec4(0, 0, 0, 1)).xyz;
	if (light_type == 0) {
		light_direction = normalize(mat3(model_matrix) * vec3(0.0, 0.0, 1.0));
	}
	else {
		light_direction = normalize(light_pos - world_space_pos);
	}


	vec4 color = vec4(texture(Color, pos.xy).xyz, 1.0);
	vec3 normal = texture(Normal, pos.xy).xyz;
	float attenuation_factor = 1;

	if (light_type == 1) {
		float distance = length(light_pos - world_space_pos);
		attenuation_factor = 1.0 / (attenuation_constants.x + (attenuation_constants.y * distance) + attenuation_constants.z * (distance * distance));
	}

	color_out =  vec4(color.xyz * Light_Color.xyz * attenuation_factor * Light_Color.w * (0.1 + max(0, dot(normal, light_direction))),1.0);
}

#end
