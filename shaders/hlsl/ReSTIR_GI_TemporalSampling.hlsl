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

RWTexture2D<float4> gPreviousReservoir : register(u8, space0);
RWTexture2D<float4> gPreviousReservoir1 : register(u9, space0);
RWTexture2D<float4> gPreviousReservoir2 : register(u10, space0);
RWTexture2D<float4> gPreviousReservoir3 : register(u11, space0);


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

    float4 previous_ndc = mul(PreviousViewProjectionMatrix, float4(world_space_position.xyz, 1.0f));
    previous_ndc.xyz /= previous_ndc.w;

    float2 uv = (previous_ndc.xy * 0.5f + 0.5f);
    uv.y = 1.0f - uv.y;
    uint2 previous_index =  uv * DispatchRaysDimensions().xy;

    if (previous_index.x < 0 || previous_index.x >= 1280 || previous_index.y < 0 || previous_index.y > 720) {
        //gOutput[index.xy] = float4(asfloat(current_reservoir.OutSample), current_reservoir.W, current_reservoir.WeightSum, current_reservoir.M);
        gReservoirOutput[index.xy] = float4(current_reservoir.OutSample.Random.x, current_reservoir.W, current_reservoir.WeightSum, current_reservoir.M);
        gReservoirOutput1[index.xy] = float4(current_reservoir.OutSample.Random.y, current_reservoir.OutSample.X_s);
        gReservoirOutput2[index.xy] = float4(current_reservoir.OutSample.Random.z, current_reservoir.OutSample.Normal_s);
        gReservoirOutput3[index.xy] = float4(current_reservoir.OutSample.L_o, 1.0f);
    } else {
        Reservoir<ReSTIR_GI_Sample> previous_reservoir;
        previous_reservoir.OutSample.X_s = gPreviousReservoir1[index.xy].yzw;
        previous_reservoir.OutSample.Normal_s = gPreviousReservoir2[index.xy].yzw;
        previous_reservoir.OutSample.L_o = gPreviousReservoir3[index.xy].xyz;
        previous_reservoir.OutSample.Random = float3(gPreviousReservoir[index.xy].x, gPreviousReservoir1[index.xy].x, gPreviousReservoir2[index.xy].x);
        previous_reservoir.WeightSum = gPreviousReservoir[index.xy].z;
        previous_reservoir.M = gPreviousReservoir[index.xy].w;
        previous_reservoir.W = gPreviousReservoir[index.xy].y; 

        if (previous_reservoir.M <= 0.0f) {
            gReservoirOutput[index.xy] = float4(current_reservoir.OutSample.Random.x, current_reservoir.W, current_reservoir.WeightSum, current_reservoir.M);
            gReservoirOutput1[index.xy] = float4(current_reservoir.OutSample.Random.y, current_reservoir.OutSample.X_s);
            gReservoirOutput2[index.xy] = float4(current_reservoir.OutSample.Random.z, current_reservoir.OutSample.Normal_s);
            gReservoirOutput3[index.xy] = float4(current_reservoir.OutSample.L_o, 1.0f);
            return;
        }

    #define CLAMP_VALUE 30
        if (previous_reservoir.M > CLAMP_VALUE * current_reservoir.M) {
            previous_reservoir.WeightSum *= CLAMP_VALUE * current_reservoir.M / previous_reservoir.M;
            previous_reservoir.M = CLAMP_VALUE * current_reservoir.M;
        }

        float p_hat1 = length(current_reservoir.OutSample.L_o);
        float p_hat2 = length(previous_reservoir.OutSample.L_o);

        Reservoir<ReSTIR_GI_Sample> reservoir = CombineReservoirs(rng, current_reservoir, p_hat1, previous_reservoir, p_hat2);
        
        reservoir.W = reservoir.WeightSum / (reservoir.M * length(reservoir.OutSample.L_o) + 0.0001f);

        gReservoirOutput[index.xy] = float4(reservoir.OutSample.Random.x, reservoir.W, reservoir.WeightSum, reservoir.M);
        gReservoirOutput1[index.xy] = float4(reservoir.OutSample.Random.y, reservoir.OutSample.X_s);
        gReservoirOutput2[index.xy] = float4(reservoir.OutSample.Random.z, reservoir.OutSample.Normal_s);
        gReservoirOutput3[index.xy] = float4(reservoir.OutSample.L_o, 1.0f);
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