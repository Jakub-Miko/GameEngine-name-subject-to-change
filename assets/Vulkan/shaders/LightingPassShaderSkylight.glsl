#RootSignature
{
	"RootSignature": [
		{
			"name" : "conf",
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
					"size" : 4,
					"name" : "Textures",
					"type" : "texture_2D",
					"individual_names" : [
						"Color", "Normal","Roughness", "DepthBuffer"
					]
				}
			]
		},
		{
			"name" : "Diffuse",
			"type" : "texture_2D_cubemap"
		},
		{
			"name" : "Specular",
			"type" : "texture_2D_cubemap"
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

uniform conf{
	mat4 inverse_view;
	mat4 inverse_projection;
	float depth_constant_a;
	float depth_constant_b;
};

uniform light_props{
	vec4 Light_Color;
	vec2 pixel_size;
};

out vec3 light_volume_pos;

void main() {
	gl_Position = vec4(position, 1.0);
	light_volume_pos = vec3(inverse_projection * vec4(position.xy,-1.0, 1.0));
	 
}


#end
#Fragment //------------------------------------------------
#version 410

layout(location = 0) out vec4 color_out;

uniform sampler2D Color;
uniform sampler2D Normal;
uniform sampler2D Roughness;
uniform sampler2D DepthBuffer;
uniform samplerCube Diffuse;
uniform samplerCube Specular;

uniform conf{
	mat4 inverse_view;
	mat4 inverse_projection;
	float depth_constant_a;
	float depth_constant_b;
};

uniform light_props{
	vec4 Light_Color;
	vec2 pixel_size;
};

in vec3 light_volume_pos;

vec3 GetFragmentPosition(vec3 coordinates) {
	vec3 dir = vec3(light_volume_pos.xy / abs(light_volume_pos.z),-1.0);
	float depth = texture(DepthBuffer, coordinates.xy).x;

	float linearized_depth = depth_constant_b / (depth - depth_constant_a);

	return dir * linearized_depth;
}

void main() {
	vec3 coords = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 view_space_pos = GetFragmentPosition(coords);


	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	float roughness = texture(Roughness, coords.xy).x;
	vec3 normal = mat3(inverse_view) * reflect(normalize(vec3(light_volume_pos.xy / abs(light_volume_pos.z), -1.0)) , texture(Normal, coords.xy).xyz);

	vec3 diffuse_contribution = texture(Diffuse, normalize(normal)).xyz;
	vec3 specular_contribution = textureLod(Specular, normalize(normal), roughness*4.0).xyz;

	color_out = vec4(color.xyz * Light_Color.xyz * Light_Color.w * (diffuse_contribution + specular_contribution),1.0);
}

#end
