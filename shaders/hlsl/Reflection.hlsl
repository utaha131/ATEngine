[[vk::binding(0, 0)]] cbuffer PostProcessParameters : register(b0, space0) {
    float4x4 InverseProjectionMatrix;
    float4x4 InverseViewMatrix;
    float2 OutputResolution;
};

[[vk::binding(1, 0)]] Texture2D g_SceneColorTexture : register(t0, space0);
[[vk::binding(2, 0)]] Texture2D g_BaseColorTexture : register(t1, space0);
[[vk::binding(3, 0)]] Texture2D g_NormalTexture : register(t2, space0);
[[vk::binding(4, 0)]] Texture2D g_SurfaceTexture : register(t3, space0);
[[vk::binding(5, 0)]] Texture2D g_DepthTexture : register(t4, space0);
[[vk::binding(6, 0)]] Texture2D g_SSAOTexture : register(t5, space0);
[[vk::binding(7, 0)]] Texture2D g_SSRTexture : register(t6, space0);
[[vk::binding(8, 0)]] Texture2D g_BRDFLutTexture : register(t7, space0);
[[vk::binding(9, 0)]] TextureCube g_PrefilteredEnviromentMapTexture : register(t8, space0);
[[vk::binding(10, 0)]] RWTexture2D<float4> g_OutputTexture : register(u0, space0);
[[vk::binding(11, 0)]] SamplerState g_Sampler : register(s0);

static const float3 F0_DIELECTRICS = float3(0.04f, 0.04f, 0.04f);

[numthreads( 8, 8, 1 )]
void main(uint Group_Index : SV_GroupIndex, uint3 Thread_ID : SV_DispatchThreadID) {
    if (Thread_ID.x < OutputResolution.x && Thread_ID.y < OutputResolution.y) {
        float2 uv = ((float2)Thread_ID.xy + 0.5f) * (1.0f / OutputResolution);
        float depth = g_DepthTexture.SampleLevel(g_Sampler, uv.xy, 0).r;
        float3 coords = float3(float2(uv.x, 1.0f - uv.y) * 2.0f - 1.0f, depth);
        float4 view_space_position = mul(InverseProjectionMatrix, float4(coords, 1.0f));
        view_space_position.xyz /= view_space_position.w;
        float4 base_color = g_BaseColorTexture.SampleLevel(g_Sampler, uv.xy, 0);
        float3 surface = g_SurfaceTexture.SampleLevel(g_Sampler, uv.xy, 0).rgb;
        float3 specular_color = float3(lerp(F0_DIELECTRICS, base_color.rgb, surface.b).rgb);
        float3 ssr_color = g_SSRTexture.SampleLevel(g_Sampler, uv.xy, 0).rgb * specular_color;

        float3 N = g_NormalTexture.SampleLevel(g_Sampler, uv.xy, 0).xyz;
        float3 specular_reflection_color = float3(0.0f, 0.0f, 0.0f);
       
        if (length(N) > 0) {
            N = normalize(mul(InverseViewMatrix, normalize(N * 2.0f - 1.0f)));
            float3 V = normalize(mul(InverseViewMatrix, normalize(view_space_position.xyz)));
            float3 R = normalize(reflect(V, N));
            float2 brdf = g_BRDFLutTexture.SampleLevel(g_Sampler, float2(saturate(dot(N, V)), 1.0f - surface.g), 0).rg;
            specular_reflection_color = g_PrefilteredEnviromentMapTexture.SampleLevel(g_Sampler, R,  surface.g * 7.0f).rgb * (specular_color * brdf.x + brdf.y);
        }
        
        g_OutputTexture[Thread_ID.xy] = float4(
            g_SceneColorTexture.SampleLevel(g_Sampler, uv.xy, 0).rgb + (ssr_color + specular_reflection_color) * g_SSAOTexture.SampleLevel(g_Sampler, uv.xy, 0).r,
            1.0f
        );
    }
}