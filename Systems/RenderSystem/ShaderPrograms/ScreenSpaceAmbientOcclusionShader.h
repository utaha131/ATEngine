#ifndef _AT_RENDER_SYSTEM_SCREEN_SPACE_AMBIENT_OCCLUSION_SHADER_H_
#define _AT_RENDER_SYSTEM_SCREEN_SPACE_AMBIENT_OCCLUSION_SHADER_H_
#include "../GPUShader.h"

namespace AT {
	class ScreenSpaceAmbientOcclusionShader : public GPUShader {
	public:

		BEGIN_SHADER_PARAMETER_GROUP(PassGroup)
			BEGIN_CONSTANTS
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Inverse_Projection_Matrix)
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Projection_Matrix)
			DEFINE_CONSTANT(DirectX::XMFLOAT4A, Kernel[64])
			DEFINE_CONSTANT(DirectX::XMFLOAT2, Size)
			END_CONSTANTS
			SHADER_PARAMETER(Texture2D, g_Depth)
			SHADER_PARAMETER(Texture2D, g_Normal)
			SHADER_PARAMETER(Texture2D, g_Noise)
			END_SHADER_PARAMETER_GROUP(PassGroup)

			BEGIN_SHADER_PARAMETERS(ScreenSpaceAmbientOcclusionParameters)
				SHADER_PARAMETER_GROUP(PassGroup, pass)
				BEGIN_STATIC_SAMPLER(Sampler)
				END_STATIC_SAMPLER(Sampler)
			END_SHADER_PARAMETERS(ScreenSpaceAmbientOcclusionParameters)

			using Parameters = ScreenSpaceAmbientOcclusionParameters;

		ScreenSpaceAmbientOcclusionShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;

			m_PixelShader = ps;

			Parameters parameters;
			parameters.Sampler.Filter = RHI::Filter::MIN_MAG_MIP_LINEAR;
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
			params->pass->constant_buffer->WriteData(params->pass->constants);
			
			m_Device->WriteDescriptorTable(params->pass->m_descriptor_table, 0, 1, &params->pass->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(params->pass->m_descriptor_table, 1, 1, &params->pass->g_Depth.srv);
			m_Device->WriteDescriptorTable(params->pass->m_descriptor_table, 2, 1, &params->pass->g_Normal.srv);
			m_Device->WriteDescriptorTable(params->pass->m_descriptor_table, 3, 1, &params->pass->g_Noise.srv);
			
			command_list->SetGraphicsRootDescriptorTable(0, params->pass->m_descriptor_table);
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