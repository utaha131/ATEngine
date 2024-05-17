#include "DX12Shader.h"

namespace RHI::DX12 {
	DX12Shader::DX12Shader(const RHI::ShaderDescription& description, IDxcBlob* dx12_blob) :
		RHI::IShader(description),
		m_DX12Blob(dx12_blob)
	{

	}

	DX12Shader::~DX12Shader() {
		m_DX12Blob->Release();
	}
}