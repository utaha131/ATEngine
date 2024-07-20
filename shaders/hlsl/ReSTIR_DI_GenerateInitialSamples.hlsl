//#include "BRDF.hlsli"

//#include "Lights.hlsli"
#include "BxDF.hlsli"
#include "ReSTIR.hlsli"
#include "PathTracingHelper.hlsli"

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
  const uint SampleCount  = 32;//min(32, LIGHT_COUNT);
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
  Reservoir<ReSTIR_DI_Sample> reservoir;
  reservoir.OutSample = 0;
  reservoir.WeightSum = 0.0f;
  reservoir.M = 0.0f;
  reservoir.W = 0.0f;
  
  for (uint i = 0; i < SampleCount; ++i) {
      //Sample Light.
      ReSTIR_DI_Sample index = (LIGHT_COUNT - 1) * rng.SampleUniform();
      LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[index]);
      light_sample.pdf /= LIGHT_COUNT; // times uniform pdf.
      float p_hat = ReSTIR_DI_TargetFunction(light_sample, surface_interaction);
      float w = (p_hat) / light_sample.pdf;
      reservoir.AddSample(rng, index, w);
  }

  LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[reservoir.OutSample]);

  reservoir.W = reservoir.WeightSum / ZERO_GUARD(reservoir.M * ReSTIR_DI_TargetFunction(light_sample, surface_interaction));
  float3 L = SampleDirectLighting(light_sample, surface_interaction);
  if (TraceVisibility(surface_interaction.Position + surface_interaction.Normal * 0.0001f, light_sample.wi, light_sample.wi_distance) == 0.0f) {
    reservoir.W = 0.0f;
  }
  gOutput[index.xy] = float4(asfloat(reservoir.OutSample), reservoir.W, reservoir.WeightSum, reservoir.M);
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