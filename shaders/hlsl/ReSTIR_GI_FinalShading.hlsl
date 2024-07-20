#include "BxDF.hlsli"
#include "ReSTIR.hlsli"
#include "PathTracingHelper.hlsli"
//#include "Sampling.hlsli"

cbuffer Globals : register(b0, space0) {
  float4x4 InverseViewProjectionMatrix;
  float4x4 PreviousViewProjectionMatrix;
  float4 CameraPosition;
  uint FrameNumber;
};
Texture2D<float4> g_BaseColor : register(t1, space0);
Texture2D<float4> g_Normals : register(t2, space0);
Texture2D<float4> g_Surface : register(t3, space0);
Texture2D<float4> g_Depth : register(t4, space0);
RWTexture2D<float4> gOutput : register(u0, space0);

RWTexture2D<float4> gReservoir : register(u1, space0);
RWTexture2D<float4> gReservoir1 : register(u2, space0);
RWTexture2D<float4> gReservoir2 : register(u3, space0);
RWTexture2D<float4> gReservoir3 : register(u4, space0);

RWTexture2D<float4> gSaveReservoir : register(u5, space0);
RWTexture2D<float4> gSaveReservoir1 : register(u6, space0);
RWTexture2D<float4> gSaveReservoir2 : register(u7, space0);
RWTexture2D<float4> gSaveReservoir3 : register(u8, space0);

SamplerState g_Sampler : register(s0, space0);

struct InstanceInfo {
  uint32_t VertexBufferIndex;
	uint32_t IndexBufferIndex;
	uint32_t MaterialIndex;
};

StructuredBuffer<InstanceInfo> g_InstanceInfo : register(t5, space0);

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes {
  float2 BarycentricCoordinates;
};

[shader("raygeneration")]
void RayGen() {
    uint3 index = DispatchRaysIndex();
    float2 dims = float2(DispatchRaysDimensions().xy);
    float2 d = (((index.xy + 0.5f) / dims.xy) * 2.0f - 1.0f);
    float4 base_color = g_BaseColor.Load(index.xyz).rgba;
    float3 normal = normalize(g_Normals.Load(index.xyz).xyz * 2.0f - 1.0f);
    float3 surface = g_Surface.Load(index.xyz).rgb;
    float depth = g_Depth.Load(index.xyz).r;

    float4 world_space_position = mul(InverseViewProjectionMatrix, float4(d.x, -d.y, depth, 1.0f));
    world_space_position.xyz /= world_space_position.w;
    float3 wo = normalize(CameraPosition.xyz - world_space_position.xyz);

    SurfaceInteraction<UE4BRDF> surface_interaction;
    surface_interaction.Position = world_space_position.xyz;
    surface_interaction.Normal = normal;
    surface_interaction.wo = wo;
    surface_interaction.brdf.BaseColor = base_color.rgba;
    surface_interaction.brdf.Roughness = surface.g;
    surface_interaction.brdf.Metalness = surface.b;

    float3 wi = normalize(gReservoir1[index.xy].yzw - surface_interaction.Position);
    gOutput[index.xy] = float4(surface_interaction.brdf.F(surface_interaction.wo, surface_interaction.Normal, wi) * gReservoir3[index.xy].rgb * gReservoir[index.xy].y, 1.0f);
    gSaveReservoir[index.xy] = gReservoir[index.xy];
    gSaveReservoir1[index.xy] = gReservoir1[index.xy];
    gSaveReservoir2[index.xy] = gReservoir2[index.xy];
    gSaveReservoir3[index.xy] = gReservoir3[index.xy];
}

[shader("closesthit")] 
void ClosestHit(inout RayPayload ray_payload, in Attributes attributes)  {
  if (ray_payload.IsShadowRay) {
    ray_payload.Visibility = 0.0f;
    return;
  }
}

[shader("miss")]
void Miss(inout RayPayload ray_payload : SV_RayPayload) {
}