
float PointAttenuationFalloff(float fragment_distance, float light_radius) {
    float nom = max(0.0, 1- pow(fragment_distance/light_radius,4));
    return (nom*nom)/(fragment_distance*fragment_distance + 1.0);
}