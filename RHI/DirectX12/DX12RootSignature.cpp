#include "DX12RootSignature.h"

namespace RHI::DX12 {
	DX12RootSignature::DX12RootSignature(const RHI::RootSignatureDescription& description, ID3D12RootSignature* dx12_root_signature) :
		RHI::IRootSignature(description),
		m_DX12RootSignature(dx12_root_signature)
	{

	}

	DX12RootSignature::~DX12RootSignature() {
		m_DX12RootSignature->Release();
	}
}