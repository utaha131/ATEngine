#include "Cube.hlsli"

struct PixelIn {
    float4 Position : SV_POSITION;
    float3 UV : TEXCOORD;
};

[[vk::binding(0, 0)]] cbuffer Constants : register(b0, space0) {
    float4x4 MV_Matrix;
};

[[vk::binding(1, 0)]] TextureCube enviroment_map : register(t0, space0);
[[vk::binding(2, 0)]] SamplerState g_Sampler : register(s0, space0);

PixelIn VS(uint VertexID : SV_VertexID) {
    PixelIn output;
    float4 position = Cube[VertexID];
    output.UV = position.xyz;
    position = mul(MV_Matrix, position);
    output.Position = position.xyzw;
    return output;
}


float4 PS(PixelIn input) : SV_Target {
    float PI = 3.14159265359f;

    float3 normal = normalize(input.UV);
    float3 up = float3(0.0f, 1.0f, 0.0f);

    float3 right = normalize(cross(up, normal));
    up =  normalize(cross(normal, right));

    float sample_delta_x = 2.0f * PI / 8.0f;
    float sample_delta_y = 0.5f * PI / 8.0f;
    uint sample_count = 0;

    float3 irradiance = float3(0.0f, 0.0f, 0.0f);

    for (float phi = 0.0f; phi < 2.0f * PI; phi += sample_delta_x) {
        for (float theta = 0.0f; theta < 0.5 * PI; theta += sample_delta_y) {
            
            float3 tangent_sample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sample_vec = normalize(tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal); 

            irradiance += enviroment_map.Sample(g_Sampler, sample_vec).rgb * cos(theta) * sin(theta);
            ++sample_count;
        }
    }

    irradiance = PI * irradiance * (1.0f / float(sample_count));

    return float4(irradiance, 1.0f);
}