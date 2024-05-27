#ifndef _AT_RENDER_SYSTEM_SHADER_PROGRAMS_DEPTH_PREPASS_H_
#define _AT_RENDER_SYSTEM_SHADER_PROGRAMS_DEPTH_PREPASS_H_
#include "../GPUShader.h"

namespace AT {
	class DepthPrepassShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(ObjectGroup)
			BEGIN_CONSTANTS
				DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, ModelViewProjectionMatrix)
				DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, NormalMatrix)
			END_CONSTANTS
			SHADER_PARAMETER(Texture2D, BaseColorMap)
		END_SHADER_PARAMETER_GROUP(ObjectGroup)

		BEGIN_SHADER_PARAMETERS(DepthPrepassShaderParameters) 
			SHADER_PARAMETER_GROUP(ObjectGroup, Object)
			BEGIN_STATIC_SAMPLER(MaterialSampler)
				.Filter = RHI::Filter::ANISOTROPIC,
				.AddressU = RHI::TextureAddressMode::WRAP,
				.AddressV = RHI::TextureAddressMode::WRAP,
				.AddressW = RHI::TextureAddressMode::WRAP,
				.MipLODBias = 0,
				.MaxAnisotropy = 10,
				.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL,
				.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK,
				.MinLOD = 0.0f,
				.MaxLOD = RHI_FLOAT32_MAX,
			END_STATIC_SAMPLER(MaterialSampler)
		END_SHADER_PARAMETERS(DepthPrepassShaderParameters)
		using Parameters = DepthPrepassShaderParameters;
		DepthPrepassShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			
			m_VertexShader = vs;
			m_PixelShader = ps;

			Parameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			Parameters* param = static_cast<Parameters*>(parameters);
			param->Object->constant_buffer->WriteData(param->Object->constants);
			m_Device->WriteDescriptorTable(param->Object->m_descriptor_table, 0, 1, &param->Object->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->Object->m_descriptor_table, 1, 1, &param->Object->BaseColorMap.srv);

			command_list->SetGraphicsRootDescriptorTable(0, param->Object->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const {
			return m_VertexShader;
		}

		RHI::Shader GetPixelShader() const {
			return m_PixelShader;
		}
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};
}
#endif