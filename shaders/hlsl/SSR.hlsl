[[vk::binding(0, 0)]] cbuffer SSRSettings : register(b0, space0) {
    float4x4 Projection_Matrix;
    float4x4 View_Matrix;
    float4x4 Inverse_View_Projection_Matrix;
    float3 Camera_Position;
    float2 Output_Resolution;
};

struct Ray {
    float3 Origin;
    float3 Direction;
}

Texture2D Depth_Texture : register(t0);
Texture2D Normal_Texture : register(t0);

Ray RayCast(float2 uv) {
    float4 world_space_position = mul(Inverse_View_Projection_Matrix, float4(uv.xy, 0.0f, 1.0f));
    world_space_position.xyzw /= world_space_position.w;
    Ray ray;
    ray.Origin = Camera_Position;
    ray.Direction = normalize(world_space_position.xyz - Camera_Position);
    
    return ray;
}

[numthreads( 8, 8, 1 )]
void main(uint Group_Index : SV_GroupIndex, uint3 Thread_ID : SV_DispatchThreadID) {
    if (Thread_ID.x < Output_Resolution.x && Thread_ID.y < Output_Resolution.y) {
        float2 uv = ((float2)Thread_ID.xy + 0.5f) * (1.0f / Output_Resolution);

    }
}