
#define PI 3.1415926535897932384626433832795

vec3 FresnelSchlick(vec3 F0, float LdotH) {
    return F0 + (1 - F0)*pow(1-LdotH, 5);
}

float NormalDistributionGGX(float NdotH, float a2) {
    float NdotH_2 = NdotH * NdotH;
    float denom = NdotH_2 * a2 - NdotH_2 + 1;
    return a2 / (denom * denom * PI);
}

float GeometricTermSmithGGX(float NdotL, float NdotV, float a2) {
    float V = NdotL * sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
    float L = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);
    return 0.5 / max(V + L,0.000001);
}

vec3 SpecularBRDF(vec3 F, float NdotV, float NdotH, float NdotL, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    return F * NormalDistributionGGX(NdotH, a2) * GeometricTermSmithGGX(NdotL, NdotV, a2);
}

vec3 DiffuseBRDFLambert(vec3 diffuse) {
    return diffuse / PI;
}

vec3 CookTorranceModel(vec3 L, vec3 V, vec3 N, vec3 albedo, float roughness, float metallic) {
    vec3 H = normalize(L + V);
    float NdotL = max(0,dot(N, L));
    float NdotV = max(0,dot(N, V));
    float NdotH = max(0,dot(N, H));
    float LdotH = max(0,dot(L, H));

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = FresnelSchlick(F0, LdotH);

    vec3 kd = (1 - F) * (1 - metallic);
    vec3 specular = SpecularBRDF(F, NdotV, NdotH, NdotL, roughness);
    vec3 diffuse = kd * DiffuseBRDFLambert(albedo);

    return (specular + diffuse) * NdotL;
}