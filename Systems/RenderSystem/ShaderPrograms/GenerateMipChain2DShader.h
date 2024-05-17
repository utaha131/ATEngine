#ifndef _AT_RENDER_SYSTEM_SHADER_PROGRAMS_GENERATE_MIP_CHAIN_SHADER_2D_H_
#define _AT_RENDER_SYSTEM_SHADER_PROGRAMS_GENERATE_MIP_CHAIN_SHADER_2D_H_
#include "../GPUShader.h"

namespace AT {
	class GenerateMipChain2DShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(ComputeGroup)
		BEGIN_CONSTANTS
			DEFINE_CONSTANT(bool, IsSRGB)
			DEFINE_CONSTANT(DirectX::XMFLOAT2, Output_Resolution)
		END_CONSTANTS
		SHADER_PARAMETER(Texture2D, Source_Texture)
		SHADER_PARAMETER(RWTexture2D, Output_Texture)
		END_SHADER_PARAMETER_GROUP(ComputeGroup)

		BEGIN_SHADER_PARAMETERS(GenerateMipsParameters)
			SHADER_PARAMETER_GROUP(ComputeGroup, compute)
			BEGIN_STATIC_SAMPLER(Sampler)
				.Filter = RHI::Filter::MIN_MAG_LINEAR_MIP_POINT,
				.AddressU = RHI::TextureAddressMode::CLAMP,
				.AddressV = RHI::TextureAddressMode::CLAMP,
				.AddressW = RHI::TextureAddressMode::CLAMP,
				.MipLODBias = 0,
				.MaxAnisotropy = 10,
				.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL,
				.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK,
				.MinLOD = 0.0f,
				.MaxLOD = RHI_FLOAT32_MAX,
			END_STATIC_SAMPLER(Sampler)
		END_SHADER_PARAMETERS(GenerateMipsParameters)

		using Parameters = GenerateMipsParameters;
		GenerateMipChain2DShader(RHI::Shader cs, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_ComputeShader = cs;

			GenerateMipsParameters parameters;
			parameters.Sampler.Filter = RHI::Filter::MIN_MAG_LINEAR_MIP_POINT;
			parameters.Sampler.AddressU = RHI::TextureAddressMode::CLAMP;
			parameters.Sampler.AddressV = RHI::TextureAddressMode::CLAMP;
			parameters.Sampler.AddressW = RHI::TextureAddressMode::CLAMP;
			parameters.Sampler.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL;
			parameters.Sampler.MaxAnisotropy = 10;
			parameters.Sampler.MinLOD = 0.0f;
			parameters.Sampler.MaxLOD = RHI_FLOAT32_MAX;
			parameters.Sampler.MipLODBias = 0;
			parameters.Sampler.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		RHI::Shader GetComputeShader() const {
			return m_ComputeShader;
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const {
			Parameters* param = static_cast<Parameters*>(parameters);
			static_cast<Parameters*>(parameters)->compute->constant_buffer->WriteData(static_cast<Parameters*>(parameters)->compute->constants);
			m_Device->WriteDescriptorTable(param->compute->m_descriptor_table, 0, 1, &param->compute->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->compute->m_descriptor_table, 1, 1, &param->compute->Source_Texture.srv);
			m_Device->WriteDescriptorTable(param->compute->m_descriptor_table, 2, 1, &param->compute->Output_Texture.uav);
			command_list->SetComputeRootDescriptorTable(0, static_cast<Parameters*>(parameters)->compute->m_descriptor_table);
		}
	private:
		RHI::Shader m_ComputeShader;
	};
}
#endif