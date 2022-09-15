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
						"Color", "Normal", "DepthBuffer", "ShadowMap"
					]
				}, 
				{
					"size" : 1,
					"name" : "Cubemaps",
					"type" : "texture_2D_cubemap",
					"individual_names" : [
						"ShadowCubeMap"
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
						"name": "light_far_plane",
						"type" : "FLOAT"
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
				"name": "light_far_plane",
				"type" : "SCALAR",
				"value" : 1.0
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

uniform conf{
	mat4 mvp_matrix;
	mat4 view_model_matrix;
	mat4 light_matrix;
	mat4 inverse_projection;
	float depth_constant_a;
	float depth_constant_b;
};

uniform light_props{
	vec4 Light_Color;
	vec4 attenuation_constants;
	vec2 pixel_size;
	float light_far_plane;
	int light_type;
};

out vec3 light_volume_pos;
out vec3 light_pos;
out vec3 light_direction_in;

void main() {
	if (light_type == 0) {
		gl_Position = vec4(position, 1.0);
		light_volume_pos = vec3(inverse_projection * vec4(position.xy,-1.0, 1.0));
		light_pos = vec3(view_model_matrix[3]);
		light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
	}
	else {
		gl_Position = mvp_matrix * vec4(position, 1.0);
		light_volume_pos = vec3(view_model_matrix * vec4(position, 1.0));
		light_pos = vec3(view_model_matrix[3]);
		light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
	}
	 
}


#end
#Fragment //------------------------------------------------
#version 410

layout(location = 0) out vec4 color_out;

uniform sampler2D Color;
uniform sampler2D Normal;
uniform sampler2D DepthBuffer;
uniform sampler2D ShadowMap;
uniform samplerCube ShadowCubeMap;

uniform conf {
	mat4 mvp_matrix;
	mat4 view_model_matrix;
	mat4 light_matrix;
	mat4 inverse_projection;
	float depth_constant_a;
	float depth_constant_b;
};

uniform light_props{
	vec4 Light_Color;
	vec4 attenuation_constants;
	vec2 pixel_size;
	float light_far_plane;
	int light_type;
};

in vec3 light_volume_pos;
in vec3 light_pos;
in vec3 light_direction_in;


float calculate_shadows(vec3 view_space_pos) {
	if (light_type == 0) {
		vec4 light_space_pos = light_matrix * vec4(view_space_pos,1.0);
		vec3 light_space_coords = light_space_pos.xyz / light_space_pos.w;
		light_space_coords = light_space_coords * 0.5 + 0.5;
		float shadow_map_depth = texture(ShadowMap, light_space_coords.xy).x;
		float current_depth = light_space_coords.z;
		if (current_depth - 0.0005 < shadow_map_depth || shadow_map_depth > 0.99)
		{
			return 1.0;
		}
		else {
			return 0.0;
		}

	}
	else if (light_type == 1) {
		vec4 light_space_pos = light_matrix * vec4(view_space_pos, 1.0);
		vec3 light_space_coords = normalize(light_space_pos.xyz);
		float shadow_map = texture(ShadowCubeMap, light_space_coords).x;
		float shadow_map_depth = shadow_map * light_far_plane;
		float current_depth = length(light_space_pos);
		if (current_depth - 0.3 < shadow_map_depth || shadow_map > 0.99)
		{
			return 1.0;
		}
		else {
			return 0.0;
		}
	}

	return 1.0;
}

vec3 GetFragmentPosition(vec3 coordinates) {
	vec3 dir = vec3(light_volume_pos.xy / abs(light_volume_pos.z),-1.0);
	float depth = texture(DepthBuffer, coordinates.xy).x;

	float linearized_depth = depth_constant_b / (depth - depth_constant_a);

	return dir * linearized_depth;
}

void main() {
	vec3 coords = vec3((gl_FragCoord.x * pixel_size.x), (gl_FragCoord.y * pixel_size.y), 0.0);
	vec3 view_space_pos = GetFragmentPosition(coords);
	vec3 light_direction;
	if (light_type == 0) {
		light_direction = light_direction_in;
	}
	else {
		light_direction = -normalize(light_pos - view_space_pos);
	}


	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	vec3 normal = texture(Normal, coords.xy).xyz;
	float attenuation_factor = 1;

	if (light_type == 1) {
		float distance = length(light_pos - view_space_pos);
		attenuation_factor = 1.0 / (attenuation_constants.x + (attenuation_constants.y * distance) + attenuation_constants.z * (distance * distance));
	}
	float shadows = calculate_shadows(view_space_pos);
	color_out = vec4(shadows * color.xyz * Light_Color.xyz * attenuation_factor * Light_Color.w * (0.1 + max(0, dot(normal, -light_direction))),1.0);
}

#end
