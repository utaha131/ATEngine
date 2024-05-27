//Basic Screen-Space Reflection Implementation.
#define MAX_STEPS 32
#define THRESHOLD 0.5f
#define SSR_MAX_ROUGHNESS 0.22
[[vk::binding(0, 0)]] cbuffer SSRSettings : register(b0, space0) {
    float4x4 ProjectionMatrix;
    float4x4 InverseProjectionMatrix;
    float2 OutputResolution;
};

[[vk::binding(1, 0)]] Texture2D g_SceneColor : register(t0);
[[vk::binding(2, 0)]] Texture2D g_DepthTexture : register(t1);
[[vk::binding(3, 0)]] Texture2D g_NormalTexture : register(t2);
[[vk::binding(4, 0)]]Texture2D g_SurfaceTexture : register(t3);
[[vk::binding(5, 0)]] RWTexture2D<float4> g_Output : register(u0);
[[vk::binding(6, 0)]] SamplerState g_Sampler : register(s0);

float3 PositionToScreenSpace(float3 position) {
    float4 projected = mul(ProjectionMatrix, float4(position, 1.0f));
    projected.xyz /= projected.w;
    projected.xy = projected.xy * 0.5f + 0.5f;
    projected.y = 1.0f - projected.y;
    return projected;
}

float4 BinarySearch(float3 direction, inout float3 position) {
    float sample_depth;
    for (uint i = 0; i < MAX_STEPS; ++i) {
        float3 uv = PositionToScreenSpace(position.xyz);
    
        sample_depth = g_DepthTexture.SampleLevel(g_Sampler, uv.xy, 0).r;
        if (uv.z >= sample_depth) {
            position += direction;
        }
        direction *= 0.5f;
        position -= direction;
    }
    float3 uv = PositionToScreenSpace(position.xyz);
    sample_depth = g_DepthTexture.SampleLevel(g_Sampler, uv.xy, 0).r;
    return float4(uv.xy, sample_depth, uv.z);
}

[numthreads( 8, 8, 1 )]
void main(uint Group_Index : SV_GroupIndex, uint3 Thread_ID : SV_DispatchThreadID) {
    if (Thread_ID.x < OutputResolution.x && Thread_ID.y < OutputResolution.y) {
        float2 texel_size = (1.0f / OutputResolution);
        float2 uv = ((float2)Thread_ID.xy + 0.5f) * texel_size;

        if (g_SurfaceTexture.SampleLevel(g_Sampler, uv.xy, 0).g > SSR_MAX_ROUGHNESS) {
            g_Output[Thread_ID.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
            return;
        }

        float2 coords = float2(uv.x, 1.0f - uv.y) * 2.0f - 1.0f;
        float depth = g_DepthTexture.SampleLevel(g_Sampler, uv, 0).r;
        float4 view_space_position = mul(InverseProjectionMatrix, float4(coords.xy, depth, 1.0f));
        view_space_position.xyz /= view_space_position.w;
        float3 normal = normalize(g_NormalTexture.SampleLevel(g_Sampler, uv, 0).xyz * 2.0f - 1.0f);
        float3 view_space_direction = normalize(reflect(view_space_position.xyz, normal));

        //TODO: convert to Screen Space.

        float3 start = view_space_position;
        float3 direction = view_space_direction;
        
        //Ray March Sequence.
        float sample_depth;
        float4 out_coords = float4(0.0f, 0.0f, 0.0f, 0.0f);

        for (uint i = 0; i < MAX_STEPS; ++i) {
            start += direction;

            float3 uv_position = PositionToScreenSpace(start.xyz);

            sample_depth = g_DepthTexture.SampleLevel(g_Sampler, uv_position.xy, 0).r;
            if (uv_position.z < sample_depth) {
                out_coords = BinarySearch(direction.xyz, start.xyz);
                g_Output[Thread_ID.xy] = g_SceneColor.SampleLevel(g_Sampler, out_coords.xy, 0).rgba;
                return;
            }
        }  
        g_Output[Thread_ID.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}