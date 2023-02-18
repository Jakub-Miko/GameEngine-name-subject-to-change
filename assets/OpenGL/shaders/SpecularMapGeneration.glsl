#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "in_tex",
			"type" : "texture_2D_cubemap"
		}
	]
}
#end

#Vertex //--------------------------------------------------
#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 uv;


uniform mvp{
	mat4 mvp_matrix[6];
	float roughness;
};


void main() {
	gl_Position = vec4(position, 1.0);
}


#end
#Geometry //--------------------------------------------------
#version 410
layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 pos;

uniform mvp{
	mat4 mvp_matrix[6];
	float roughness;
};


void main() {
	gl_Layer = gl_InvocationID;
	for (int i = 0; i < 3; i++) {
		pos = gl_in[i].gl_Position.xyz;
		gl_Position = mvp_matrix[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}


#end


#Fragment //------------------------------------------------
#version 410

uniform samplerCube in_tex;

layout(location = 0) out vec4 specular;
in vec3 pos;

uniform mvp{
	mat4 mvp_matrix[6];
	float roughness;
};

#define PI 3.14159265

float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}



void main() {
	vec3 N = normalize(pos);
	vec3 R = N;
	vec3 V = R;

	const uint SAMPLE_COUNT = 128u;
	float totalWeight = 0.0;
	vec3 prefilteredColor = vec3(0.0);
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0)
		{
			prefilteredColor += clamp(texture(in_tex, L).rgb,0.0,1.0) * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	specular = vec4(prefilteredColor, 1.0);


}

#end
