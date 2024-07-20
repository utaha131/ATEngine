#ifndef _PATH_TRACING_HELPER_HLSL_
#define _PATH_TRACING_HELPER_HLSL_

#include "Lights.hlsli"

RaytracingAccelerationStructure Scene : register(t0, space0);

struct RayPayload {
    // float3 L;
    // float3 Throughput;
    bool IsShadowRay;
    float Visibility;
    //Hit Surface
    bool HitSurface;
    float3 Position;
    float3 Normal;
    float3 Wo;
    float4 BaseColor;
    float Roughness;
    float Metalness;
};

template<typename BxDF> 
struct SurfaceInteraction {
    float3 Position;
    float3 Normal;
    float3 wo;
    BxDF brdf;
};

float TraceVisibility(float3 position, float3 direction, float distance) {
    RayDesc shadow_ray;
    shadow_ray.Origin = position;
    shadow_ray.Direction = direction;
    shadow_ray.TMin = 0.0f;
    shadow_ray.TMax = distance;

    RayPayload ray_payload;
    ray_payload.Visibility = 1.0f;
    ray_payload.IsShadowRay = true;
    TraceRay(Scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadow_ray, ray_payload);
    return ray_payload.Visibility;
}

template <typename BxDF> float3 SampleDirectLighting(in LightSample light_sample, in SurfaceInteraction<BxDF> surface_interaction) {

    float visibility = TraceVisibility(float3(surface_interaction.Position + surface_interaction.Normal * 0.0001f), light_sample.wi, light_sample.wi_distance);

    float3 f = surface_interaction.brdf.F(surface_interaction.wo, surface_interaction.Normal, light_sample.wi);
    float3 Li = light_sample.L * 10.0f;
    float cos_theta = abs(dot(light_sample.wi, surface_interaction.Normal));
    return f * Li * cos_theta / light_sample.pdf * visibility;
}
#endif