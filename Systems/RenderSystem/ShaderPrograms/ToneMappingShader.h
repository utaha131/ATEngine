#ifndef _AT_RENDER_SYSTEM_TONE_MAPPING_SHADER_H_
#define _AT_RENDER_SYSTEM_TONE_MAPPING_SHADER_H_
#include "../GPUShader.h"

namespace AT {
	class ToneMappingShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(PassGroup)
		SHADER_PARAMETER(Texture2D, Frame)
		END_SHADER_PARAMETER_GROUP(PassGroup)

		BEGIN_SHADER_PARAMETERS(ToneMappingShaderParameters)
		SHADER_PARAMETER_GROUP(PassGroup, pass)
		BEGIN_STATIC_SAMPLER(Sampler)
		END_STATIC_SAMPLER(Sampler)
		END_SHADER_PARAMETERS(ToneMappingShaderParameters)

		using Parameters = ToneMappingShaderParameters;

		ToneMappingShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;
			m_PixelShader = ps;

			ToneMappingShaderParameters parameters;
			parameters.Sampler.Filter = RHI::Filter::MIN_MAG_MIP_POINT;
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

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			Parameters* params = static_cast<Parameters*>(parameters);
			m_Device->WriteDescriptorTable(params->pass->m_descriptor_table, 0, 1, &params->pass->Frame.srv);
			command_list->SetGraphicsRootDescriptorTable(0, params->pass->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const { return m_VertexShader; }
		RHI::Shader GetPixelShader() const { return m_PixelShader; }
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};
}
#endif