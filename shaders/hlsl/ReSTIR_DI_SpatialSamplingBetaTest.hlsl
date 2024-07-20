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

  Reservoir<ReSTIR_DI_Sample> s;
  // s.OutSample = 0;
  // s.WeightSum = 0.0f;
  // s.M = 0.0f;
  // s.W = 0.0f;
  s.OutSample= asuint(gReservoirBuffer[index.xy].x);
  s.W = gReservoirBuffer[index.xy].y;
  s.WeightSum = gReservoirBuffer[index.xy].z;
  s.M = gReservoirBuffer[index.xy].w;
  float M_sum = s.M;
  
  float current_depth = length(CameraPosition.xyz - world_space_position.xyz);
  float3 current_normal = normal;

  float pixel_radius = 30.0f;

  for (uint i = 0; i < 5; ++i) {
      float2 offset = ( SampleDisk(float2(rng.SampleUniform(), rng.SampleUniform()))  ) * pixel_radius;
      uint2 neighbour_index =(uint2)(DispatchRaysIndex().xy + 0.5f + offset);
      
      if (neighbour_index.x >= 1280 || neighbour_index.x < 0 || neighbour_index.y < 0 || neighbour_index.y >= 720) {
        continue;
      }

      float2 neighbour_d = (((neighbour_index.xy) / dims.xy) * 2.0f - 1.0f);
      float4 neighbour_world_space_position = mul(InverseViewProjectionMatrix, float4(neighbour_d.x, -neighbour_d.y, g_Depth.Load(uint3(neighbour_index.xy, 0)).r, 1.0f));
      neighbour_world_space_position.xyz /= neighbour_world_space_position.w;

      float neighbour_depth = length(CameraPosition.xyz - neighbour_world_space_position.xyz);

      if (abs(neighbour_depth - current_depth) > 0.1f * current_depth || dot(current_normal, normalize(g_Normals.Load(uint3(neighbour_index.xy, 0)).xyz)) < 0.9063f) {
        continue;
      }
      Reservoir<ReSTIR_DI_Sample> neighbour_reservoir;
      neighbour_reservoir.OutSample = asuint(gReservoirBuffer[neighbour_index.xy].x);
      neighbour_reservoir.W = gReservoirBuffer[neighbour_index.xy].y;
      neighbour_reservoir.WeightSum = gReservoirBuffer[neighbour_index.xy].z;
      neighbour_reservoir.M = gReservoirBuffer[neighbour_index.xy].w;

      LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[neighbour_reservoir.OutSample]);
      float p_hat = ReSTIR_DI_TargetFunction(light_sample, surface_interaction);
      s.AddSample(rng, neighbour_reservoir.OutSample, p_hat * neighbour_reservoir.W * neighbour_reservoir.M);
      M_sum += neighbour_reservoir.M;
  }

  s.M = M_sum;
  
  LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[s.OutSample]);
  s.W = s.WeightSum / ZERO_GUARD(s.M * ReSTIR_DI_TargetFunction(light_sample, surface_interaction));
  gOutput[index.xy] = float4(asfloat(s.OutSample), s.W, s.WeightSum, s.M);
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