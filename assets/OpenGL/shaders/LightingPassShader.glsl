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
						"name" : "pixel_size",
						"type" : "VEC2"
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
	vec2 pixel_size;
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
	vec2 pixel_size;
};

void main() {
	vec3 pos = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 light_direction = normalize(mat3(model_matrix) * vec3(0.0, 0.0, 1.0));

	vec4 color = vec4(texture(Color, pos.xy).xyz, 1.0);
	vec3 normal = texture(Normal, pos.xy).xyz;

	color_out =  vec4(color.xyz * Light_Color.xyz * Light_Color.w * (0.45 + max(0, dot(normal, light_direction))),1.0);

}

#end
