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

RWTexture2D<float4> gReservoirOutput : register(u0, space0);
RWTexture2D<float4> gReservoirOutput1 : register(u1, space0);
RWTexture2D<float4> gReservoirOutput2 : register(u2, space0);
RWTexture2D<float4> gReservoirOutput3 : register(u3, space0);

RWTexture2D<float4> gReservoir : register(u4, space0);
RWTexture2D<float4> gReservoir1 : register(u5, space0);
RWTexture2D<float4> gReservoir2 : register(u6, space0);
RWTexture2D<float4> gReservoir3 : register(u7, space0);


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

    Reservoir<ReSTIR_GI_Sample> current_reservoir;
    current_reservoir.OutSample.X_s = gReservoir1[index.xy].yzw;
    current_reservoir.OutSample.Normal_s = gReservoir2[index.xy].yzw;
    current_reservoir.OutSample.L_o = gReservoir3[index.xy].xyz;
    current_reservoir.OutSample.Random = float3(gReservoir[index.xy].x, gReservoir1[index.xy].x, gReservoir2[index.xy].x);
    current_reservoir.WeightSum = gReservoir[index.xy].z;
    current_reservoir.M = gReservoir[index.xy].w;
    current_reservoir.W = gReservoir[index.xy].y;

    float current_depth = length(CameraPosition.xyz - world_space_position.xyz);
    float3 current_normal = normal;

    float pixel_radius = 30.0f;

    float M_sum = current_reservoir.M;

    for (uint i = 0u; i < 5u; ++i) {
      float2 offset = ( SampleDisk(float2(rng.SampleUniform(), rng.SampleUniform()))  ) * pixel_radius;
      uint2 neighbour_index = (uint2)(DispatchRaysIndex().xy + 0.5f + offset);

      if (neighbour_index.x >= 1280 || neighbour_index.x < 0 || neighbour_index.y < 0 || neighbour_index.y >= 720) {
        continue;
      }

      float2 neighbour_d = (((neighbour_index.xy) / dims.xy) * 2.0f - 1.0f);
      float4 neighbour_world_space_position = mul(InverseViewProjectionMatrix, float4(neighbour_d.x, -neighbour_d.y, g_Depth.Load(uint3(neighbour_index.xy, 0)).r, 1.0f));
      neighbour_world_space_position.xyz /= neighbour_world_space_position.w;

        Reservoir<ReSTIR_GI_Sample> neighbour_reservoir;
        neighbour_reservoir.OutSample.X_s = gReservoir1[neighbour_index.xy].yzw;
        neighbour_reservoir.OutSample.Normal_s = gReservoir2[neighbour_index.xy].yzw;
        neighbour_reservoir.OutSample.L_o = gReservoir3[neighbour_index.xy].xyz;
        neighbour_reservoir.OutSample.Random = float3(gReservoir[neighbour_index.xy].x, gReservoir1[neighbour_index.xy].x, gReservoir2[neighbour_index.xy].x);
        neighbour_reservoir.WeightSum = gReservoir[neighbour_index.xy].z;
        neighbour_reservoir.M = gReservoir[neighbour_index.xy].w;
        neighbour_reservoir.W = gReservoir[neighbour_index.xy].y;

        if (neighbour_reservoir.M <= 0.0f) {
            continue;
        }

      float neighbour_depth = length(CameraPosition.xyz - neighbour_world_space_position.xyz);

      if (abs(neighbour_depth - current_depth) > 0.05f * current_depth || dot(current_normal, normalize(g_Normals.Load(uint3(neighbour_index.xy, 0)).xyz)) < 0.9063f) {
        continue;
      }

      float jacobian = CalculateJacobian( world_space_position, neighbour_world_space_position, neighbour_reservoir);
      
      if (!ValidateJacobian(jacobian)) {
        continue;
      }

      float p_hat_prime = neighbour_reservoir.OutSample.L_o / ZERO_GUARD(jacobian);

      // RayPayload ray_payload;
      // ray_payload.IsShadowRay = true;
      // ray_payload.Visibility = 1.0f;
      // RayDesc ray_desc;
      // ray_desc.Origin = neighbour_reservoir.OutSample.X_s;
      // ray_desc.Direction = normalize(surface_interaction.Position - neighbour_reservoir.OutSample.X_s);
      // ray_desc.TMin = 0.0f;
      // ray_desc.TMax = length(surface_interaction.Position - neighbour_reservoir.OutSample.X_s);
      // TraceRay(Scene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray_desc, ray_payload);
      float visibility = TraceVisibility((neighbour_reservoir.OutSample.X_s), normalize(surface_interaction.Position - neighbour_reservoir.OutSample.X_s), length(surface_interaction.Position - neighbour_reservoir.OutSample.X_s));

      p_hat_prime *= visibility;

        if (visibility == 0.0f) {
            continue;
        }
        M_sum += neighbour_reservoir.M;
    
        current_reservoir = CombineReservoirs(rng, current_reservoir, current_reservoir.OutSample.L_o, neighbour_reservoir, p_hat_prime);

    }
    
    current_reservoir.M = M_sum;
    current_reservoir.W = current_reservoir.WeightSum / ZERO_GUARD(current_reservoir.M * length(current_reservoir.OutSample.L_o));

    gReservoirOutput[index.xy] = float4(current_reservoir.OutSample.Random.x, current_reservoir.W, current_reservoir.WeightSum, current_reservoir.M);
    gReservoirOutput1[index.xy] = float4(current_reservoir.OutSample.Random.y, current_reservoir.OutSample.X_s);
    gReservoirOutput2[index.xy] = float4(current_reservoir.OutSample.Random.z, current_reservoir.OutSample.Normal_s);
    gReservoirOutput3[index.xy] = float4(current_reservoir.OutSample.L_o, 1.0f);
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