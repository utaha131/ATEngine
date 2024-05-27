#ifndef _AT_RENDER_SYSTEM_SHADER_DIFFUSE_INDIRECT_SHADER_H_
#define _AT_RENDER_SYSTEM_SHADER_DIFFUSE_INDIRECT_SHADER_H_
#include "../GPUShader.h"

namespace AT {
	class DiffuseIndirectShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(ComputeGroup)
		BEGIN_CONSTANTS
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4, InverseViewMatrix)
		DEFINE_CONSTANT(DirectX::XMFLOAT2, OutputResolution)
		END_CONSTANTS
		SHADER_PARAMETER(Texture2D, SceneColorTexture)
		SHADER_PARAMETER(Texture2D, BaseColorTexture)
		SHADER_PARAMETER(Texture2D, NormalTexture)
		SHADER_PARAMETER(Texture2D, RoughnessMetalnessTexture)
		SHADER_PARAMETER(Texture2D, SSAOTexture)
		SHADER_PARAMETER(TextureCube, DiffuseIrradianceTexture)
		SHADER_PARAMETER(RWTexture2D, OutputTexture)
		END_SHADER_PARAMETER_GROUP(ComputeGroup)

		BEGIN_SHADER_PARAMETERS(DiffuseIndirectShaderParameters)
		SHADER_PARAMETER_GROUP(ComputeGroup, Compute)
		BEGIN_STATIC_SAMPLER(Sampler)
		.Filter = RHI::Filter::MIN_MAG_MIP_POINT,
		.AddressU = RHI::TextureAddressMode::CLAMP,
		.AddressV = RHI::TextureAddressMode::CLAMP,
		.AddressW = RHI::TextureAddressMode::CLAMP,
		.MipLODBias = 0,
		.MaxAnisotropy = 1,
		.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL,
		.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK,
		.MinLOD = 0.0f,
		.MaxLOD = RHI_FLOAT32_MAX,
		END_STATIC_SAMPLER(Sampler)
		END_SHADER_PARAMETERS(DiffuseIndirectShaderParameters)

		using Parameters = DiffuseIndirectShaderParameters;
		DiffuseIndirectShader(RHI::Shader cs, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_ComputeShader = cs;
			Parameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		RHI::Shader GetComputeShader() const {
			return m_ComputeShader;
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const {
			Parameters* param = static_cast<Parameters*>(parameters);
			static_cast<Parameters*>(parameters)->Compute->constant_buffer->WriteData(static_cast<Parameters*>(parameters)->Compute->constants);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 0, 1, &param->Compute->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 1, 1, &param->Compute->SceneColorTexture.srv);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 2, 1, &param->Compute->BaseColorTexture.srv);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 3, 1, &param->Compute->NormalTexture.srv);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 4, 1, &param->Compute->RoughnessMetalnessTexture.srv);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 5, 1, &param->Compute->SSAOTexture.srv);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 6, 1, &param->Compute->DiffuseIrradianceTexture.srv);
			m_Device->WriteDescriptorTable(param->Compute->m_descriptor_table, 7, 1, &param->Compute->OutputTexture.uav);
			command_list->SetComputeRootDescriptorTable(0, static_cast<Parameters*>(parameters)->Compute->m_descriptor_table);
		}
	private:
		RHI::Shader m_ComputeShader;
	};
}
#endif