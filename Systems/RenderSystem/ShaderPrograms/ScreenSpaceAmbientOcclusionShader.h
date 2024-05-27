#ifndef _AT_RENDER_SYSTEM_SCREEN_SPACE_AMBIENT_OCCLUSION_SHADER_H_
#define _AT_RENDER_SYSTEM_SCREEN_SPACE_AMBIENT_OCCLUSION_SHADER_H_
#include "../GPUShader.h"

namespace AT {
	class ScreenSpaceAmbientOcclusionShader : public GPUShader {
	public:

		BEGIN_SHADER_PARAMETER_GROUP(PassGroup)
			BEGIN_CONSTANTS
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, InverseProjectionMatrix)
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, ProjectionMatrix)
			DEFINE_CONSTANT(DirectX::XMFLOAT4A, Kernel[64])
			DEFINE_CONSTANT(DirectX::XMFLOAT2, Size)
			END_CONSTANTS
			SHADER_PARAMETER(Texture2D, g_DepthTexture)
			SHADER_PARAMETER(Texture2D, g_NormalTexture)
			SHADER_PARAMETER(Texture2D, g_NoiseTexture)
			END_SHADER_PARAMETER_GROUP(PassGroup)

			BEGIN_SHADER_PARAMETERS(ScreenSpaceAmbientOcclusionParameters)
				SHADER_PARAMETER_GROUP(PassGroup, Pass)
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
			END_SHADER_PARAMETERS(ScreenSpaceAmbientOcclusionParameters)

			using Parameters = ScreenSpaceAmbientOcclusionParameters;

		ScreenSpaceAmbientOcclusionShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;

			m_PixelShader = ps;

			Parameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			Parameters* params = static_cast<Parameters*>(parameters);
			params->Pass->constant_buffer->WriteData(params->Pass->constants);
			
			m_Device->WriteDescriptorTable(params->Pass->m_descriptor_table, 0, 1, &params->Pass->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(params->Pass->m_descriptor_table, 1, 1, &params->Pass->g_DepthTexture.srv);
			m_Device->WriteDescriptorTable(params->Pass->m_descriptor_table, 2, 1, &params->Pass->g_NormalTexture.srv);
			m_Device->WriteDescriptorTable(params->Pass->m_descriptor_table, 3, 1, &params->Pass->g_NoiseTexture.srv);
			
			command_list->SetGraphicsRootDescriptorTable(0, params->Pass->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const { return m_VertexShader; }
		RHI::Shader GetPixelShader() const { return m_PixelShader; }
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};

	typedef ScreenSpaceAmbientOcclusionShader SSAOShader;
}
#endif