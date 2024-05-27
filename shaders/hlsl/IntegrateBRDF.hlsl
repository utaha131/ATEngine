#include "Quad.hlsli"
#include "IBL.hlsli"

struct PixelIn {
    float4 Position : SV_POSITION;
	float2 UV : TEXCOORD;
};

PixelIn VS(uint VertexID : SV_VertexID) {
    PixelIn output;
    output.Position = float4(Quad[VertexID].xyz, 1.0f);
	output.UV = Quad_UV[VertexID];
    return output;
}

float2 PS(PixelIn input) : SV_Target {
    return IntegrateBRDF(input.UV.x, 1.0f - input.UV.y);
}