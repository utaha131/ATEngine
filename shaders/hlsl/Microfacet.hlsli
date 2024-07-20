static const float PI = 3.14159265359f;
static const float3 F0_DIELECTRICS = float3(0.04f, 0.04f, 0.04f);

float3 TangentToWorld(float3 V, float3 N) {

    const float3 up_vector = abs(N.y) < 0.9999f ? float3(0, 1, 0) : float3(0, 0, -1);
    const float3 tangent_x = normalize(cross(up_vector, N));
    const float3 tangent_y = cross(N, tangent_x);

    return normalize(tangent_x * V.x + tangent_y * V.y  + N * V.z);
}


float3 ImportanceSampleGGX(float2 x_i, float roughness, float3 wo, float3 normal) {
    float alpha = roughness * roughness;
    float phi = 2.0f * PI * x_i.x;
    float cos_theta = sqrt( (1.0f - x_i.y) / ( 1.0f + (alpha * alpha - 1.0f) * x_i.y ) );
    float sin_theta = sqrt( 1.0f - cos_theta * cos_theta );

    float3 H;
    H.x = sin_theta * cos(phi);
    H.y = sin_theta * sin(phi);
    H.z = cos_theta;

    float3 world_space_H = TangentToWorld(H, normal);
    return normalize(reflect(-wo, world_space_H));
}

float D(float roughness, float NdotH) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    return alpha2 / ( PI * pow( ( pow(NdotH, 2.0f) * (alpha2 - 1.0f) + 1.0f ), 2.0f ) + 0.0000001f );
}

float G_Shlick(float roughness, float NdotV) {
    float k = pow(roughness + 1.0f, 2.0f) / 8.0f;
    return NdotV / ( NdotV * (1.0f - k) + k + 0.0000001f );
}

float G(float roughness, float NdotV, float NdotL) {
    return G_Shlick(roughness, NdotV) * G_Shlick(roughness, NdotL);
}

float3 Fresnel(float3 F0, float VdotH) {
    return F0 + (1.0f - F0) * pow(2.0f, ( -5.55473 * VdotH - 6.98316 ) * VdotH);
}