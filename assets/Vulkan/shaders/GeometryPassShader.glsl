#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;

out vec2 uv_fragment;
out vec3 pos_fragment;
out vec3 normal_fragment;
out mat3 TBN;

layout(set = 0, binding = 0) uniform mvp{
	mat4 mvp_matrix;
	mat4 view_model_matrix;
};

layout(set = 1, binding = 0) uniform material{
	vec4 Base_Color;
	float roughness_bias;
	float roughness_gain;
};

void main() {
	vec3 normal_transformed = normalize(mat3(transpose(inverse(view_model_matrix))) * normal.xyz).xyz;
	vec3 tangent_transformed = normalize(mat3(transpose(inverse(view_model_matrix))) * tangent.xyz).xyz;
	vec3 bitangent_transformed = cross(normal_transformed, tangent_transformed);

	TBN = mat3(tangent_transformed, bitangent_transformed, normal_transformed);
	
	pos_fragment = (view_model_matrix * vec4(position, 1.0)).xyz;
	gl_Position = mvp_matrix * vec4(position, 1.0);
	uv_fragment = uv;
	normal_fragment = normalize(mat3(transpose(inverse(view_model_matrix))) * normal.xyz).xyz;
}


#end
#Fragment //------------------------------------------------
#version 430

in vec2 uv_fragment;
in vec3 pos_fragment;
in vec3 normal_fragment;
in mat3 TBN;

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec4 normal_out;
layout(location = 2) out float roughness_out;

layout(set = 1, binding = 1) uniform sampler2D Color;
layout(set = 1, binding = 2) uniform sampler2D Normal;
layout(set = 1, binding = 3) uniform sampler2D Roughness;

layout(set = 1, binding = 0) uniform material{
	vec4 Base_Color;
	float roughness_bias;
	float roughness_gain;
};

void main() {
	color_out = vec4(texture(Color,uv_fragment).xyz,1) * Base_Color;
	roughness_out = texture(Roughness, uv_fragment).x * roughness_gain + roughness_bias;
	normal_out = normalize(vec4(TBN * (texture(Normal, uv_fragment).rgb * 2.0 - 1.0),0.0));
}

#end
