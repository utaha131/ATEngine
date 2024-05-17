#ifndef _RHI_DX12_SHADER_H_
#define _RHI_DX12_SHADER_H_
#include "../IShader.h"
#include "Headers/dx12.h"

namespace RHI::DX12 {
	class DX12Shader : public RHI::IShader {
	public:
		DX12Shader(const RHI::ShaderDescription& description, IDxcBlob* dx12_blob);
		~DX12Shader() override;

		inline D3D12_SHADER_BYTECODE GetByteCode() const {
			D3D12_SHADER_BYTECODE byte_code;
			byte_code.BytecodeLength = m_DX12Blob->GetBufferSize();
			byte_code.pShaderBytecode = m_DX12Blob->GetBufferPointer();
			return byte_code;
		};
	private:
		IDxcBlob* m_DX12Blob;

	};
}
#endif