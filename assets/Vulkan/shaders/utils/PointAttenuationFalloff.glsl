
float PointAttenuationFalloff(float fragment_distance, float light_radius) {
    return pow(max(0.0,1 - (fragment_distance / light_radius)), 2);
}