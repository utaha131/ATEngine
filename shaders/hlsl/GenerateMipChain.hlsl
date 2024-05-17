[[vk::binding(0, 0)]] cbuffer MipSettings : register(b0, space0) {
    bool IsSRGB;
    float2 Output_Resolution;
};

[[vk::binding(1, 0)]] Texture2D<float4> Source_Texture : register(t0);
[[vk::binding(2, 0)]] RWTexture2D<float4> Output_Texture : register(u0);
[[vk::binding(3, 0)]] SamplerState BilinearClamp : register(s0);

float3 ApplySRGBCurve(float3 x)
{
    // This is exactly the sRGB curve
    //return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
     
    // This is cheaper but nearly equivalent
    return (x < 0.0031308) ? 12.92 * x : 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719;//select(x < 0.0031308, 12.92 * x, 1.13005 * sqrt(abs(x - 0.00228)) - 0.13448 * x + 0.005719);
}

[numthreads( 8, 8, 1 )]
void main(uint Group_Index : SV_GroupIndex, uint3 Thread_ID : SV_DispatchThreadID) {
    if (Thread_ID.x < Output_Resolution.x && Thread_ID.y < Output_Resolution.y) {
        float2 uv = ((float2)Thread_ID.xy + 0.5f) * (1.0f / Output_Resolution.xy);
        float4 r = Source_Texture.GatherRed(BilinearClamp, uv);
        float4 g = Source_Texture.GatherGreen(BilinearClamp, uv);
        float4 b = Source_Texture.GatherBlue(BilinearClamp, uv);
        float4 a = Source_Texture.GatherAlpha(BilinearClamp, uv);
        float sum = (a.x + a.y + a.z + a.w);
        float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
        if (sum == 0.0f) {
            color.rgb += float4(r.x, g.x, b.x, a.x);
            color.rgb += float4(r.y, g.y, b.y, a.y);
            color.rgb += float4(r.z, g.z, b.z, a.z);
            color.rgb += float4(r.w, g.w, b.w, a.w);
            color.rgb /= 4.0f;
        } else {
            color.rgb += float4(r.x, g.x, b.x, 0.0f) * a.x;
            color.rgb += float4(r.y, g.y, b.y, 0.0f) * a.y;
            color.rgb += float4(r.z, g.z, b.z, 0.0f) * a.z;
            color.rgb += float4(r.w, g.w, b.w, 0.0f) * a.w;
            color.rgb /= sum;
            color.a = max(a.x, max(a.y, max(a.z, a.w)));
        }
        if (IsSRGB) {
            color.rgb = ApplySRGBCurve(color.rgb);
        }
        Output_Texture[Thread_ID.xy] = color;
    }
}