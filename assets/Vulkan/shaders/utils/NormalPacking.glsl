
vec2 ExtractSign(vec2 in_vec) {
    return vec2(in_vec.x > 0 ? 1.0 : -1.0, in_vec.y > 0 ? 1.0 : -1.0);
}

vec2 PackNormals(vec3 normal) {
    normal /= dot(abs(normal), vec3(1.0));
    if(normal.z < 0) {
        normal.xy = (1.0 - abs(normal.yx)) * ExtractSign(normal.xy);
    }
    return normal.xy;
}

vec3 UnpackNormals(vec2 packed) {
    vec3 normal = vec3(packed, 1.0 - dot(abs(packed), vec2(1.0)));
    if(normal.z < 0) {
        normal.xy = (1.0 - abs(normal.yx)) * ExtractSign(normal.xy);
    }
    return normalize(normal);
}