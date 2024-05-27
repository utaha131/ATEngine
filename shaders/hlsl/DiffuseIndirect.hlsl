[[vk::binding(0, 0)]] cbuffer PostProcessParameters : register(b0, space0) {
    float4x4 InverseViewMatrix;
    float2 OutputResolution;
};

[[vk::binding(1, 0)]] Texture2D g_SceneColor : register(t0, space0);
[[vk::binding(2, 0)]] Texture2D g_BaseColor : register(t1, space0);
[[vk::binding(3, 0)]] Texture2D g_Normal : register(t2, space0);
[[vk::binding(4, 0)]] Texture2D g_RoughnessMetalness : register(t3, space0);
[[vk::binding(5, 0)]] Texture2D g_SSAO : register(t4, space0);
[[vk::binding(6, 0)]] TextureCube g_DiffuseIrradianceMap : register(t5, space0);
[[vk::binding(7, 0)]] RWTexture2D<float4> g_Output : register(u0, space0);
[[vk::binding(8, 0)]] SamplerState g_Sampler : register(s0);

[numthreads( 8, 8, 1 )]
void main(uint Group_Index : SV_GroupIndex, uint3 Thread_ID : SV_DispatchThreadID) {
    if (Thread_ID.x < OutputResolution.x && Thread_ID.y < OutputResolution.y) {
        float2 uv = ((float2)Thread_ID.xy + 0.5f) * (1.0f / OutputResolution);
        float3 diffuse_reflection_color = float3(0.0f, 0.0f, 0.0f);
        float3 N = g_Normal.SampleLevel(g_Sampler, uv.xy, 0).xyz;
        float4 albedo = float4(0.0f, 0.0f, 0.0f, 0.0f);
        if (length(N) > 0) {
            N = normalize(mul(InverseViewMatrix, normalize(N * 2.0f - 1.0f)));
            float3 albedo = g_BaseColor.SampleLevel(g_Sampler, uv.xy, 0).rgb;
            diffuse_reflection_color  = g_DiffuseIrradianceMap.SampleLevel(g_Sampler, N, 0).rgb * albedo * (1.0f - g_RoughnessMetalness.SampleLevel(g_Sampler, uv.xy, 0).b) * g_SSAO.SampleLevel(g_Sampler, uv.xy, 0).r;
        }
        g_Output[Thread_ID.xy] = float4(g_SceneColor.SampleLevel(g_Sampler, uv.xy, 0).rgb + diffuse_reflection_color.rgb, 1.0f);
    }
}