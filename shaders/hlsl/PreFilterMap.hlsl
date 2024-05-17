#include "Cube.hlsli"
static const float PI = 3.14159265359f;

[[vk::binding(0, 0)]] cbuffer cbPerPass : register(b0, space0) {
    float4x4 MV_Matrix;
    float Roughness;
    uint Mip_Slice;
}

[[vk::binding(1, 0)]] TextureCube enviroment_map : register(t0, space0);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0, space0);

float G(float alpha, float dot) {
    float k = alpha / 2.0f;
    float denom = ((dot) * (1.0f - k) + k) + 0.00001f;
    return dot / denom;
}

float G_Smith(float roughness, float NoV, float NoL) {
    float alpha = roughness * roughness;
    return G(alpha, NoV) * G(alpha, NoL);
}

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
	return float2(float(i)/float(N), RadicalInverse_VdC(i));
}

// float3 SpecularIBL( float3 SpecularColor , float Roughness, float3 N, float3 V ) {
//     float3 SpecularLighting = 0;
//     const uint NumSamples = 1024;
//     for( uint i = 0; i < NumSamples; i++ )
//     {
//         float2 Xi = Hammersley( i, NumSamples );

//         float3 H = ImportanceSampleGGX( Xi, Roughness, N );
//         float3 L = 2 * dot( V, H ) * H - V;
//         float NoV = saturate( dot( N, V ) );
//         float NoL = saturate( dot( N, L ) );
//         float NoH = saturate( dot( N, H ) );
//         float VoH = saturate( dot( V, H ) );
//         if( NoL > 0 ) {
//             float3 SampleColor = enviroment_map.SampleLevel( g_Sampler , L, 0 ).rgb;
//             float G = G_Smith( Roughness, NoV, NoL );
//             float Fc = pow( 1 - VoH, 5 );
//             float3 F = (1 - Fc) * SpecularColor + Fc;
//             // Incident light = SampleColor * NoL
//             // Microfacet specular = D*G*F / (4*NoL*NoV)
//             // pdf = D * NoH / (4 * VoH)
//             SpecularLighting += SampleColor * F * G * VoH / (NoH * NoV);
//         }
//     }
//     return SpecularLighting / NumSamples;
// }

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N ) {
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    float3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    float3 UpVector = abs(N.z) < 0.999 ? float3(0,0,1) : float3(1,0,0);
    float3 TangentX = normalize( cross( UpVector, N ) );
    float3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 PrefilterEnvMap( float Roughness, float3 R )
{
    float3 N = R;
    float3 V = R;
    float3 PrefilteredColor = 0;
    const uint NumSamples = 1024;
    uint TotalWeight = 0;
    for( uint i = 0; i < NumSamples; i++ )
    {
        float2 Xi = Hammersley( i, NumSamples );
        float3 H = ImportanceSampleGGX( Xi, Roughness, N );
        float3 L = 2 * dot( V, H ) * H - V;
        float NoL = saturate( dot( N, L ) );

        if( NoL > 0 ) {
            PrefilteredColor += enviroment_map.SampleLevel( g_Sampler , L, 0 ).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

struct PixelIn {
    float4 Position : SV_POSITION;
    float3 UV : TEXCOORD;
};

PixelIn VS(uint VertexID : SV_VertexID) {
    PixelIn output;
    float4 position = Cube[VertexID];
    output.UV = position.xyz;
    position = mul(MV_Matrix, position);
    output.Position = position.xyzw;
    return output;
}

float4 PS(PixelIn input) : SV_Target {
    float3 R = normalize(input.UV);
    float3 N = R;
    float3 V = R;
    float3 PrefilteredColor = 0;
    const uint NumSamples = 64;
    float TotalWeight = 0;
    for( uint i = 0; i < NumSamples; i++ )
    {
        float2 Xi = Hammersley( i, NumSamples );
        float3 H = ImportanceSampleGGX( Xi, Roughness, N );
        float3 L = 2 * dot( V, H ) * H - V;
        float NoL = saturate( dot( N, L ) );

        if( NoL > 0 ) {
            float4 color = enviroment_map.SampleLevel( g_Sampler , L, Mip_Slice ).rgba;
            PrefilteredColor +=  (color.a > 0.01f) ? color.rgb * NoL : 0.0f;
            TotalWeight += NoL;
        }
    }
    return float4(PrefilteredColor / TotalWeight, 1.0f);
    //return (PrefilterEnvMap(Roughness, R), 1.0f);
}