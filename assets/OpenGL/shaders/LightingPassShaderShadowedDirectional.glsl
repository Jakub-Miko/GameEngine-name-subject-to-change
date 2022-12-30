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
					"size" : 3,
					"name" : "Textures",
					"type" : "texture_2D",
					"individual_names" : [
						"Color", "Normal", "DepthBuffer"
					]
				}, 
				{
					"size" : 1,
					"name" : "Arrays",
					"type" : "texture_2D_array",
					"individual_names" : [
						"ShadowMapArray"
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
	mat4 inverse_projection;
	mat4 light_matrix_cascades[15];
	float depth_constant_a;
	float depth_constant_b;
	float camera_near_plane;
	float camera_far_plane;
	int cascade_count;
};

uniform light_props{
	vec4 Light_Color;
	vec4 attenuation_constants;
	vec2 pixel_size;
};

out vec3 light_volume_pos;
out vec3 light_pos;
out vec3 light_direction_in;

void main() {
	gl_Position = vec4(position, 1.0);
	light_volume_pos = vec3(inverse_projection * vec4(position.xy,-1.0, 1.0));
	light_pos = vec3(view_model_matrix[3]);
	light_direction_in = normalize(mat3(view_model_matrix) * vec3(0.0, 0.0, -1.0));
	 
}


#end
#Fragment //------------------------------------------------
#version 410

layout(location = 0) out vec4 color_out;

uniform sampler2D Color;
uniform sampler2D Normal;
uniform sampler2D DepthBuffer;
uniform sampler2DArrayShadow ShadowMapArray;


uniform conf {
	mat4 mvp_matrix;
	mat4 view_model_matrix;
	mat4 inverse_projection;
	mat4 light_matrix_cascades[15];
	float depth_constant_a;
	float depth_constant_b;
	float camera_near_plane;
	float camera_far_plane;
	int cascade_count;
};

uniform light_props{
	vec4 Light_Color;
	vec4 attenuation_constants;
	vec2 pixel_size;
};

in vec3 light_volume_pos;
in vec3 light_pos;
in vec3 light_direction_in;


float calculate_shadows(vec3 view_space_pos) {
	float depth = abs(view_space_pos.z);
	depth -= camera_near_plane;
	depth /= camera_far_plane - camera_near_plane;
	depth *= float(cascade_count);
	int cascade = int(floor(depth));

	vec4 light_space_pos = light_matrix_cascades[cascade] * vec4(view_space_pos,1.0);
	vec3 light_space_coords = light_space_pos.xyz / light_space_pos.w;
	light_space_coords = light_space_coords * 0.5 + 0.5;
	float current_depth = light_space_coords.z;
	vec4 shadow_coords = vec4(light_space_coords.xy, cascade, current_depth - 0.0005);
	float shadow_map_depth = texture(ShadowMapArray, shadow_coords);
	return shadow_map_depth;
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
	light_direction = light_direction_in;
	

	vec4 color = vec4(texture(Color, coords.xy).xyz, 1.0);
	vec3 normal = texture(Normal, coords.xy).xyz;
	float attenuation_factor = 1;

	float shadows = calculate_shadows(view_space_pos);
	color_out = vec4(shadows * color.xyz * Light_Color.xyz * attenuation_factor * Light_Color.w * (0.1 + max(0, dot(normal, -light_direction))),1.0);
}

#end
