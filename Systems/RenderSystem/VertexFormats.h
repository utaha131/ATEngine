#ifndef _AT_VERTEX_FORMATS_H_
#define _AT_VERTEX_FORMATS_H_
#include <DirectXMath.h>

namespace AT::VertexFormat {
	struct Vertex {
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangent;
		DirectX::XMFLOAT3 BitTangent;
		DirectX::XMFLOAT2 Tex_Coord;
	};

	RHI::InputLayout input_layout = {
		{
			{ "POSITION", RHI::Format::R32G32B32_FLOAT, 0},
			{ "NORMAL", RHI::Format::R32G32B32_FLOAT, 12 },
			{ "TANGENT", RHI::Format::R32G32B32_FLOAT, 24 },
			{ "BITANGENT", RHI::Format::R32G32B32_FLOAT, 36 },
			{ "TEXCOORD", RHI::Format::R32G32_FLOAT, 48 },
		},
		sizeof(AT::VertexFormat::Vertex)
	};
	
	typedef uint16_t Index;
}
#endif