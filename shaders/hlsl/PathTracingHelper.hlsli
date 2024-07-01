#include "Lights.hlsli"

RaytracingAccelerationStructure Scene : register(t0, space0);

struct RayPayload {
    float3 L;
    float3 Throughput;
    bool IsShadowRay;
    float Visibility;
};

template<typename BxDF> 
struct SurfaceInteraction {
    float3 Position;
    float3 Normal;
    float3 wo;
    BxDF brdf;
};

template <typename BxDF> float3 SampleDirectLighting(in LightSample light_sample, in SurfaceInteraction<BxDF> surface_interaction) {//float3 position, float3 normal, float3 wo, float4 base_color, float roughness, float metalness) {
    RayDesc shadow_ray;
    shadow_ray.Origin = float3(surface_interaction.Position + surface_interaction.Normal * 0.001f);
    shadow_ray.Direction = light_sample.wi;
    shadow_ray.TMin = 0.0f;
    shadow_ray.TMax = light_sample.wi_distance;

    RayPayload ray_payload;
    ray_payload.Visibility = 1.0f;
    ray_payload.IsShadowRay = true;
    TraceRay(Scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, shadow_ray, ray_payload);

    float3 f = surface_interaction.brdf.F(surface_interaction.wo, surface_interaction.Normal, light_sample.wi);
    float3 Li = light_sample.L * 10.0f;
    float cos_theta = abs(dot(light_sample.wi, surface_interaction.Normal));
    return f * Li * cos_theta / light_sample.pdf * ray_payload.Visibility;
}