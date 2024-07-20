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
RWTexture2D<float4> gReservoirOutput : register(u0, space0);
RWTexture2D<float4> gReservoirOutput1 : register(u1, space0);
RWTexture2D<float4> gReservoirOutput2 : register(u2, space0);
RWTexture2D<float4> gReservoirOutput3 : register(u3, space0);
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

#define SKY_COLOR float3(0.529f, 0.808f, 0.922f) * 0.0f

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

    Reservoir<ReSTIR_GI_Sample> reservoir;
    //reservoir.OutSample;
    reservoir.WeightSum = 0.0f;
    reservoir.M = 0.0f;
    reservoir.W = 0.0f;
  
 
    float3 L = float3(0.0f, 0.0f, 0.0f);
    float3 Throughput = float3(1.0f, 1.0f, 1.0f);

    //if depth
    uint ray_depth = 1; 
    uint max_depth = 2;

    SurfaceInteraction<UE4BRDF> current_surface_interaction = surface_interaction;
    float pdf = 0.0f;
    while (true) {
        if (ray_depth++ == max_depth) {
        break;
        }

        RayPayload ray_payload;
        BxDFSample bxdf_sample = current_surface_interaction.brdf.Sample_F(float2(rng.SampleUniform(), rng.SampleUniform()), current_surface_interaction.wo, current_surface_interaction.Normal);
        float cos_theta = abs(dot(bxdf_sample.wi, current_surface_interaction.Normal));
        Throughput *= bxdf_sample.f * cos_theta / (bxdf_sample.pdf + 0.0000001f);

        pdf = bxdf_sample.pdf + 0.0000001f;

        ray_payload.IsShadowRay = false;
        ray_payload.Visibility = 1.0f;
        ray_payload.HitSurface = false;
        ray_payload.Normal = float3(0.0f, 0.0f, 0.0f);
        
        RayDesc ray_desc;
        ray_desc.Origin = current_surface_interaction.Position + current_surface_interaction.Normal * 0.001f;
        ray_desc.Direction = bxdf_sample.wi;
        ray_desc.TMin = 0.0f;
        ray_desc.TMax = 10000.0f;
        TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray_desc, ray_payload);
            
        if (ray_payload.HitSurface) {
        //Get Surface.
        current_surface_interaction.Position = ray_payload.Position;
        current_surface_interaction.Normal = ray_payload.Normal;
        current_surface_interaction.wo = ray_payload.Wo;
        current_surface_interaction.brdf.BaseColor = ray_payload.BaseColor;
        
        current_surface_interaction.brdf.Roughness = ray_payload.Roughness;
        current_surface_interaction.brdf.Metalness = ray_payload.Metalness;
        
        //Sample Ld.
        uint index = (LIGHT_COUNT - 1) * rng.SampleUniform();
        LightSample light_sample = GetLightSample(current_surface_interaction.Position, Lights[index]);
        light_sample.pdf /= LIGHT_COUNT;

        float3 Ld = SampleDirectLighting(light_sample, current_surface_interaction);
        L += Ld;//Throughput * Ld;
        } else {
        L += SKY_COLOR * 10.0f;//Throughput * SKY_COLOR * 10.0f;
        break;
        }
    }

    ReSTIR_GI_Sample new_sample;
    new_sample.X_s = current_surface_interaction.Position;
    new_sample.Normal_s = normalize(current_surface_interaction.Normal);
    new_sample.L_o = L;
    new_sample.Random = float3(0.0f, 0.0f, 0.0f);
    //float f = surface_interaction.brdf.F(surface_interaction.wo, surface_interaction.Normal, wi);
    reservoir.AddSample(rng, new_sample, new_sample.L_o / ZERO_GUARD(pdf));

    reservoir.W = reservoir.WeightSum / (reservoir.M * length(new_sample.L_o) + 0.0001f);

    //gOutput[index.xy] = float4(asfloat(reservoir.OutSample), reservoir.W, reservoir.WeightSum, reservoir.M);
    /*
    float3 X_v;
    float3 Normal_v;
    float3 X_s;
    float3 Normal_s;
    float3 L_o;
    float3 Random;
    */
    gReservoirOutput[index.xy] = float4(reservoir.OutSample.Random.x, reservoir.W, reservoir.WeightSum, reservoir.M);
    gReservoirOutput1[index.xy] = float4(reservoir.OutSample.Random.y, reservoir.OutSample.X_s);
    gReservoirOutput2[index.xy] = float4(reservoir.OutSample.Random.z, reservoir.OutSample.Normal_s);
    gReservoirOutput3[index.xy] = float4(reservoir.OutSample.L_o, 1.0f);
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
  vertex.BitTangent = BarycentricLerp(vertex0.BitTangent, vertex1.BitTangent, vertex2.BitTangent, barycentric);//normalize(BarycentricLerp(vertex0.BitTangent, vertex1.BitTangent, vertex2.BitTangent, barycentric));
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
  //ray_payload.Visibility = InstanceIndex();
  //return;
  InstanceInfo instance_info = g_InstanceInfo[InstanceIndex()];
  Vertex vertex = GetHitSurface(attributes, instance_info);
  Texture2D<float4> base_color_map = ResourceDescriptorHeap[instance_info.MaterialIndex];
  Texture2D<float4> normal_map = ResourceDescriptorHeap[instance_info.MaterialIndex + 1];
  Texture2D<float4> surface_map = ResourceDescriptorHeap[instance_info.MaterialIndex + 2];
  float3 wo = normalize(-WorldRayDirection());
  float3 position = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();//vertex.Position.xyz;

  uint sample_level = 0;
  float4 base_color = base_color_map.SampleLevel(g_Sampler, vertex.Tex_Coord.xy, sample_level).rgba;
  
  float3 normal = normalize(normal_map.SampleLevel(g_Sampler, vertex.Tex_Coord.xy, sample_level).xyz * 2.0f - 1.0f);
  //Construct TBN Matrix;
  float3x3 TBN = transpose(float3x3(vertex.Tangent, vertex.BitTangent, vertex.Normal));
  normal = normalize(mul(TBN, normal));
  float3 surface = surface_map.SampleLevel(g_Sampler, vertex.Tex_Coord.xy, sample_level).xyz;

  ray_payload.HitSurface = true;
  ray_payload.Position = position;
  ray_payload.Normal = normal;
  ray_payload.Wo = wo;
  ray_payload.BaseColor = base_color.rgba;
  ray_payload.Roughness = surface.g;
  ray_payload.Metalness = surface.b;

  if (InstanceIndex() == 41) {
     ray_payload.HitSurface =  false;
     return;
  }
}

[shader("miss")]
void Miss(inout RayPayload ray_payload : SV_RayPayload) {
    ray_payload.HitSurface = false;
}