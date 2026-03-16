#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},

		{
			"type" : "push_constants",
			"size" : 68
		},

		{
			"name" : "bones",
			"type" : "constant_buffer"
		},

		{
			"name" : "DeferredGPassMaterial",
			"type" : "material",
			"material_path": "api:GeometryMaterial.json"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 430

layout(location = 0) in uvec4 bone_ids;
layout(location = 1) in vec4 bone_weights;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 5) in vec2 uv;

out vec2 uv_fragment;
out vec3 pos_fragment;
out vec3 normal_fragment;
out mat3 TBN;

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
	uint entity_id;
};

layout( set = 0, binding = 0 ) uniform mvp{
	mat4 projection_matrix;
};

layout(set = 0, binding = 1) uniform bones {
	mat4 bone_matricies[100];
	uint valid;
};

layout(set = 1, binding = 0) uniform material{
	vec4 Base_Color;
	float roughness_bias;
	float roughness_gain;
	float metallic_bias;
	float metallic_gain;
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
				new_tangent += bone_weights[i] * (bone_matricies[bone_ids[i]] * vec4(tangent));
			}
		}
	}
	else {
		new_pos = vec4(position, 1.0f);
		new_normal = vec4(normal, 1.0f);
		new_tangent = vec4(tangent);
	}
	new_normal = normalize(new_normal);
	new_tangent = normalize(new_tangent);
	vec3 normal_transformed = normalize(mat3(transpose(inverse(mv_matrix))) * new_normal.xyz).xyz;
	vec3 tangent_transformed = normalize(mat3(transpose(inverse(mv_matrix))) * new_tangent.xyz).xyz;
	vec3 bitangent_transformed = cross(normal_transformed, tangent_transformed) * tangent.w;

	TBN = mat3(tangent_transformed, bitangent_transformed, normal_transformed);
	
	pos_fragment = (mv_matrix * vec4(new_pos.xyz,1.0f)).xyz;
	gl_Position = projection_matrix * mv_matrix * vec4(new_pos.xyz, 1.0f);
	uv_fragment = uv;
	uv_fragment.y = 1 - uv_fragment.y;
	normal_fragment = normalize(mat3(transpose(inverse(mv_matrix))) * normal.xyz).xyz;
}


#end
#Fragment //------------------------------------------------
#version 430

in vec2 uv_fragment;
in vec3 pos_fragment;
in vec3 normal_fragment;
in mat3 TBN;

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec2 normal_out;
layout(location = 2) out vec4 roughness_out;
layout(location = 3) out uint ids_out;

layout(set = 1, binding = 1) uniform sampler2D Color;
layout(set = 1, binding = 2) uniform sampler2D Normal;
layout(set = 1, binding = 3) uniform sampler2D Roughness;

layout(set = 1, binding = 0) uniform material{
	vec4 Base_Color;
	float roughness_bias;
	float roughness_gain;
	float metallic_bias;
	float metallic_gain;
};

layout(push_constant) uniform model_view {
	mat4 mv_matrix;
	uint entity_id;
};

#include <shaders/utils/NormalPacking.glsl>

void main() {
	ids_out = entity_id;
	color_out = vec4(texture(Color,uv_fragment).xyz,1) * Base_Color;
	vec4 pbr = vec4(texture(Roughness, uv_fragment).xyz, 1.0);
pbr.y = clamp(pbr.y * roughness_gain + roughness_bias, 0.0, 1.0);
pbr.z = clamp(pbr.z * metallic_gain + metallic_bias, 0.0, 1.0);
	roughness_out = pbr;
	normal_out = PackNormals(normalize(vec3(TBN * (texture(Normal, uv_fragment).rgb * 2.0 - 1.0))));
}

#end
