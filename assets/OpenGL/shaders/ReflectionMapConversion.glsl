#RootSignature
{
	"RootSignature": [
		{
			"name" : "mvp",
			"type" : "constant_buffer"
		},
		{
			"name" : "in_tex",
			"type" : "texture_2D"
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

uniform sampler2D in_tex;

layout(location = 0) out vec4 specular;
layout(location = 1) out vec4 diffuse;
in vec3 pos;

#define PI 3.14159265

vec2 VectorToPolar(vec3 in_vec) {
	vec2 out_pol = vec2(atan(in_vec.z, in_vec.x), asin(in_vec.y));
	out_pol *= vec2(0.1591, 0.3183);
	out_pol += 0.5;
	return out_pol;
}

void main() {
	vec3 normal = normalize(pos).xyz;
	specular = vec4(texture(in_tex, VectorToPolar(normal)).xyz,1.0f);
	vec3 irradiance = vec3(0.0);

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up = normalize(cross(normal, right));

	float sample_size = 0.1;
	float sample_count = 0.0;
	for (float phi = 0.0; phi < 2.0 * PI; phi += sample_size)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sample_size)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += clamp(texture(in_tex, VectorToPolar(sampleVec)).rgb,0,1) * cos(theta) * sin(theta);
			sample_count++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(sample_count));
	diffuse = vec4(irradiance,1.0);
}

#end
