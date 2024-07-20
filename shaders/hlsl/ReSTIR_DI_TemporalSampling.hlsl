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
RWTexture2D<float4> gReservoirBuffer : register(u1, space0);
RWTexture2D<float4> gPreviousReservoirBuffer : register(u2, space0);
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

  RNGSampler rng = InitRNGSampler(DispatchRaysIndex().xy, FrameNumber);

    Reservoir<ReSTIR_DI_Sample> current_reservoir;
    current_reservoir.OutSample = asuint(gReservoirBuffer[index.xy].x);
    current_reservoir.W = gReservoirBuffer[index.xy].y;
    current_reservoir.WeightSum = gReservoirBuffer[index.xy].z;
    current_reservoir.M = gReservoirBuffer[index.xy].w;

    float4 previous_ndc = mul(PreviousViewProjectionMatrix, float4(world_space_position.xyz, 1.0f));
    previous_ndc.xyz /= previous_ndc.w;
    
    float2 uv = (previous_ndc.xy * 0.5f + 0.5f);
    uv.y = 1.0f - uv.y;
    uint2 previous_index =  uv * DispatchRaysDimensions().xy;
  
    if (previous_index.x < 0 || previous_index.x >= 1280 || previous_index.y < 0 || previous_index.y > 720) {
      gOutput[index.xy] = float4(asfloat(current_reservoir.OutSample), current_reservoir.W, current_reservoir.WeightSum, current_reservoir.M);
    } else {
      

      Reservoir<ReSTIR_DI_Sample> previous_reservoir;
      previous_reservoir.OutSample = asuint(gPreviousReservoirBuffer[previous_index.xy].x);
      previous_reservoir.W = gPreviousReservoirBuffer[previous_index.xy].y;
      previous_reservoir.WeightSum = gPreviousReservoirBuffer[previous_index.xy].z;
      previous_reservoir.M = gPreviousReservoirBuffer[previous_index.xy].w;

      #define CLAMP_VALUE 20
      if (previous_reservoir.M > CLAMP_VALUE * current_reservoir.M) {
        previous_reservoir.WeightSum *= CLAMP_VALUE * current_reservoir.M / previous_reservoir.M;
        previous_reservoir.M = CLAMP_VALUE * current_reservoir.M;
      }

      LightSample light_sample1 = GetLightSample(surface_interaction.Position, Lights[current_reservoir.OutSample]);
      float p_hat1 = ReSTIR_DI_TargetFunction(light_sample1, surface_interaction);

      LightSample light_sample2 = GetLightSample(surface_interaction.Position, Lights[previous_reservoir.OutSample]);
      float p_hat2 = ReSTIR_DI_TargetFunction(light_sample2, surface_interaction);

      Reservoir<ReSTIR_DI_Sample> reservoir = CombineReservoirs(rng, current_reservoir, p_hat1, previous_reservoir, p_hat2);

      LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[reservoir.OutSample]);
      reservoir.W = reservoir.WeightSum / ZERO_GUARD(reservoir.M * ReSTIR_DI_TargetFunction(light_sample, surface_interaction));

      gOutput[index.xy] = float4(asfloat(reservoir.OutSample), reservoir.W, reservoir.WeightSum, reservoir.M);
    }

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