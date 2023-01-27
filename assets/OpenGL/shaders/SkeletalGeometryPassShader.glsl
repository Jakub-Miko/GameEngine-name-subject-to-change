#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},

		{
			"name" : "bones",
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
					"size" : 2,
					"name" : "Textures",
					"type" : "texture_2D",
					"individual_names" : [
						"Color", "Normal"
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
			},
			{
				"name": "Normal",
				"type" : "TEXTURE",
				"value" : "",
				"default_normal" : true
			}
		]
	}
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in uvec4 bone_ids;
layout(location = 1) in vec4 bone_weights;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec2 uv;

out vec2 uv_fragment;
out vec3 pos_fragment;
out vec3 normal_fragment;
out mat3 TBN;

uniform mvp{
	mat4 mvp_matrix;
	mat4 view_model_matrix;
};

uniform bones {
	mat4 bone_matricies[100];
	uint valid;
};

uniform material{
	vec4 Base_Color;
};

void main() {
	vec4 new_pos = vec4(0.0f);
	vec4 new_normal = vec4(0.0f);
	vec4 new_tangent = vec4(0.0f);
	if (valid == 1) {
		for (int i = 0; i < 4; i++) {
			if (bone_ids[i] != -1) {
				new_pos += bone_weights[i] * (bone_matricies[bone_ids[i]] * vec4(position, 1.0f));
				new_normal += bone_weights[i] * (bone_matricies[bone_ids[i]] * vec4(normal, 0.0f));
				new_tangent += bone_weights[i] * (bone_matricies[bone_ids[i]] * vec4(tangent, 0.0f));
			}
		}
	}
	else {
		new_pos = vec4(position, 1.0f);
		new_normal = vec4(normal, 1.0f);
		new_tangent = vec4(tangent, 1.0f);
	}
	new_normal = normalize(new_normal);
	new_tangent = normalize(new_tangent);
	vec3 normal_transformed = normalize(mat3(transpose(inverse(view_model_matrix))) * new_normal.xyz).xyz;
	vec3 tangent_transformed = normalize(mat3(transpose(inverse(view_model_matrix))) * new_tangent.xyz).xyz;
	vec3 bitangent_transformed = cross(normal_transformed, tangent_transformed);

	TBN = mat3(tangent_transformed, bitangent_transformed, normal_transformed);
	
	pos_fragment = (view_model_matrix * vec4(new_pos.xyz,1.0f)).xyz;
	gl_Position = mvp_matrix * vec4(new_pos.xyz, 1.0f);
	uv_fragment = uv;
	normal_fragment = normalize(mat3(transpose(inverse(view_model_matrix))) * normal.xyz).xyz;
}


#end
#Fragment //------------------------------------------------
#version 410

in vec2 uv_fragment;
in vec3 pos_fragment;
in vec3 normal_fragment;
in mat3 TBN;

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec4 normal_out;

uniform sampler2D Color;
uniform sampler2D Normal;

uniform material{
	vec4 Base_Color;
};

void main() {
	color_out = vec4(texture(Color,uv_fragment).xyz,1) * Base_Color;
	normal_out = normalize(vec4(TBN * (texture(Normal, uv_fragment).rgb * 2.0 - 1.0),0.0));
}

#end
