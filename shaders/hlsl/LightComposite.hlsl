void main() {
    float3 direct_illumination = float3(0.0f, 0.0f, 0.0f);
    float3 indirect_illumination = float3(0.0f, 0.0f, 0.0f);
    gOutput[index.xy] = float4(direct_illumination + indirect_illumination, 1.0f);
}