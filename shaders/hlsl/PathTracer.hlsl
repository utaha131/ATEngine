#include "Random.hlsli"
#include "BxDF.hlsli"
#include "PathTracingHelper.hlsli"

cbuffer Globals : register(b0, space0) {
  float4x4 InverseViewProjectionMatrix;
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

float3 TangentToWorld(float3 v, float3 N)
{
    const float3 upVector = abs(N.y) < 0.9999 ? float3(0, 1, 0) : float3(0, 0, 1);
    const float3 tangentX = normalize(cross(upVector, N));
    const float3 tangentY = cross(N, tangentX);
    // Tangent to world space
    return tangentX * v.x + tangentY * v.y + N * v.z;
}

template <typename BxDF> float3 IndirectIllumination(inout RNGSampler rng, in SurfaceInteraction<BxDF> surface_interaction) {
    /*
        float3 L;
        float3 Throughput;
        bool IsShadowRay;
        float Visibility;
    */
    RayPayload ray_payload;
    ray_payload.L = float3(0.0f, 0.0f, 0.0f);
    float3 wi;
    BxDFSample bxdf_sample = surface_interaction.brdf.Sample_F(float2(rng.SampleUniform(), rng.SampleUniform()), surface_interaction.wo, surface_interaction.Normal, wi);
    bxdf_sample.wi = TangentToWorld(bxdf_sample.wi, surface_interaction.Normal);
    float cos_theta = abs(dot(bxdf_sample.wi, surface_interaction.Normal));
    ray_payload.Throughput = bxdf_sample.f * cos_theta / bxdf_sample.pdf;
    ray_payload.IsShadowRay = false;
    ray_payload.Visibility = 1.0f;

    RayDesc ray_desc;
    ray_desc.Origin = surface_interaction.Position + surface_interaction.Normal * 0.001f;
    ray_desc.Direction = bxdf_sample.wi;
    ray_desc.TMin = 0.0f;
    ray_desc.TMax = 10000.0f;
    TraceRay(Scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray_desc, ray_payload);

    return ray_payload.L;
}


[shader("raygeneration")]
void RayGen() {
  // uint3 index = DispatchRaysIndex();
  // float2 dims = float2(DispatchRaysDimensions().xy);
  // float2 d = (((index.xy + 0.5f) / dims.xy) * 2.0f - 1.0f);
  // float4 base_color = g_BaseColor.Load(index.xyz).rgba;
  // float3 normal = normalize(g_Normals.Load(index.xyz).xyz * 2.0f - 1.0f);
  // float3 surface = g_Surface.Load(index.xyz).rgb;
  // float depth = g_Depth.Load(index.xyz).r;
  // float4 world_space_position = mul(InverseViewProjectionMatrix, float4(d.x, -d.y, depth, 1.0f));
  // world_space_position.xyz /= world_space_position.w;
  // float3 wo = normalize(CameraPosition.xyz - world_space_position.xyz);

  // RNGSampler rng = InitRNGSampler(DispatchRaysIndex().xy, FrameNumber);

  // SurfaceInteraction<UE4BRDF> surface_interaction;
  // surface_interaction.Position = world_space_position.xyz;
  // surface_interaction.Normal = normal;
  // surface_interaction.wo = wo;
  // surface_interaction.brdf.BaseColor = base_color.rgb * base_color.a * (1.0f - surface.b);
  // surface_interaction.brdf.Roughness = surface.g;
  // surface_interaction.brdf.Metalness = surface.b;

  // uint i = (LIGHT_COUNT - 1) * rng.SampleUniform();
  // LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[i]);
  // light_sample.pdf /= LIGHT_COUNT;

  // float3 DI = SampleDirectLighting<UE4BRDF>(light_sample, surface_interaction);
  // float3 GI = IndirectIllumination<UE4BRDF>(rng, surface_interaction);
  // gOutput[index.xy] = float4(DI + GI, 1.0f);
  
  
  if (FrameNumber < 3) {
    uint3 index = DispatchRaysIndex();
    gOutput[index.xy] = float4(0.0f, 0.0f, 0.0f, 1.0f);
  }

  if (FrameNumber < 1024 * 3) {
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
    
    RNGSampler rng = InitRNGSampler(DispatchRaysIndex().xy, FrameNumber);

    SurfaceInteraction<UE4BRDF> surface_interaction;
    surface_interaction.Position = world_space_position.xyz;
    surface_interaction.Normal = normal;
    surface_interaction.wo = wo;
    surface_interaction.brdf.BaseColor = base_color.rgb * base_color.a * (1.0f - surface.b);
    surface_interaction.brdf.Roughness = surface.g;
    surface_interaction.brdf.Metalness = surface.b;

    uint i = (LIGHT_COUNT - 1) * rng.SampleUniform();
    LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[i]);
    light_sample.pdf /= LIGHT_COUNT;

    float3 DI = SampleDirectLighting<UE4BRDF>(light_sample, surface_interaction);
    float3 GI = IndirectIllumination<UE4BRDF>(rng, surface_interaction);

    gOutput[index.xy].xyz += float3((DI + GI) / 1024.0f);
  }

}

struct Vertex {
  float3 Position;
  float3 Normal;
  float3 Tangent;
  float3 BitTangent;
  float2 Tex_Coord;
};

float2 BarycentricLerp(in float2 v0, in float2 v1, in float2 v2, in float3 barycentric) {
  return v0 * barycentric.x + v1 * barycentric.y + v2 * barycentric.z;
}

float3 BarycentricLerp(in float3 v0, in float3 v1, in float3 v2, in float3 barycentric) {
  return v0 * barycentric.x + v1 * barycentric.y + v2 * barycentric.z;
}

Vertex BarycentricLerp(in Vertex vertex0, in Vertex vertex1, in Vertex vertex2, in float3 barycentric) {
  Vertex vertex;
  vertex.Position = BarycentricLerp(vertex0.Position, vertex1.Position, vertex2.Position, barycentric);
  vertex.Normal = normalize(BarycentricLerp(vertex0.Normal, vertex1.Normal, vertex2.Normal, barycentric));
  vertex.Tangent = normalize(BarycentricLerp(vertex0.Tangent, vertex1.Tangent, vertex2.Tangent, barycentric));
  vertex.BitTangent = normalize(BarycentricLerp(vertex0.BitTangent, vertex1.BitTangent, vertex2.BitTangent, barycentric));
  vertex.Tex_Coord = BarycentricLerp(vertex0.Tex_Coord, vertex1.Tex_Coord, vertex2.Tex_Coord, barycentric);
  return vertex;
}

Vertex GetHitSurface(in Attributes attributes, in InstanceInfo instance_info) {
  float3 barycentric;
  barycentric.x = 1.0f - attributes.BarycentricCoordinates.x - attributes.BarycentricCoordinates.y;
  barycentric.y = attributes.BarycentricCoordinates.x;
  barycentric.z = attributes.BarycentricCoordinates.y;

  StructuredBuffer<Vertex> vertex_buffer = ResourceDescriptorHeap[instance_info.VertexBufferIndex];
  Buffer<uint> index_buffer = ResourceDescriptorHeap[instance_info.IndexBufferIndex];
  
  uint primitive_index = PrimitiveIndex();
  
  uint start = primitive_index * 3;
  uint index0 = index_buffer[start];
  uint index1 = index_buffer[start + 1];
  uint index2 = index_buffer[start + 2];

  Vertex vertex0 = vertex_buffer[index0];
  Vertex vertex1 = vertex_buffer[index1];
  Vertex vertex2 = vertex_buffer[index2];

  return BarycentricLerp(vertex0, vertex1, vertex2, barycentric);
}

[shader("closesthit")] 
void ClosestHit(inout RayPayload ray_payload, in Attributes attributes)  {
  if (ray_payload.IsShadowRay) {
    ray_payload.Visibility = 0.0f;
    return;
  }

  //Get Hit Surface Info.
  InstanceInfo instance_info = g_InstanceInfo[InstanceIndex()];
  Vertex vertex = GetHitSurface(attributes, instance_info);
  Texture2D<float4> base_color_map = ResourceDescriptorHeap[instance_info.MaterialIndex];
  Texture2D<float4> normal_map = ResourceDescriptorHeap[instance_info.MaterialIndex + 1];
  Texture2D<float4> surface_map = ResourceDescriptorHeap[instance_info.MaterialIndex + 2];
  float3 wo = normalize(-WorldRayDirection());
  float3 position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();//vertex.Position.xyz;
  float4 base_color = base_color_map.SampleLevel(g_Sampler, vertex.Tex_Coord.xy, 0).rgba;
  float3 normal = normalize(normal_map.SampleLevel(g_Sampler, vertex.Tex_Coord.xy, 0).xyz * 2.0f - 1.0f);
  //Construct TBN Matrix;
  float3x3 TBN = transpose(float3x3(vertex.Tangent, vertex.BitTangent, vertex.Normal));
  normal = normalize(mul(TBN, normal));
  float3 surface = normal_map.SampleLevel(g_Sampler, vertex.Tex_Coord.xy, 0).xyz;

  RNGSampler rng = InitRNGSampler(DispatchRaysIndex().xy, FrameNumber);
  SurfaceInteraction<UE4BRDF> surface_interaction;
  surface_interaction.Position = position;
  surface_interaction.Normal = normal;
  surface_interaction.wo = wo;
  surface_interaction.brdf.BaseColor = base_color.rgb * base_color.a * (1.0f - surface.b);
  surface_interaction.brdf.Roughness = surface.g;
  surface_interaction.brdf.Metalness = surface.b;

  uint index = (LIGHT_COUNT - 1) * rng.SampleUniform();
  LightSample light_sample = GetLightSample(surface_interaction.Position, Lights[index]);
  light_sample.pdf /= LIGHT_COUNT;
  ray_payload.L += ray_payload.Throughput * SampleDirectLighting<UE4BRDF>(light_sample, surface_interaction);
}

[shader("miss")]
void Miss(inout RayPayload ray_payload : SV_RayPayload) {
}