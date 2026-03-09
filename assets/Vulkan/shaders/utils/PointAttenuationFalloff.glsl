
float PointAttenuationFalloff(float fragment_distance, float light_radius) {
    float x = fragment_distance / light_radius;
    return pow(max(0.0,1 - x), 2);
}