#include "DX12Texture.h"

namespace RHI::DX12 {
	DX12Texture::DX12Texture(const RHI::TextureDescription& description, ID3D12Resource* dx12_resource) :
		RHI::ITexture(description),
		m_DX12Resource(dx12_resource)
	{

	}

	DX12Texture::~DX12Texture() {
		m_DX12Resource->Release();
	}

}