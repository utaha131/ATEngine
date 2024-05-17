#ifndef _AT_RENDER_SYSTEM_SHADER_PROGRAMS_DEPTH_PREPASS_H_
#define _AT_RENDER_SYSTEM_SHADER_PROGRAMS_DEPTH_PREPASS_H_
#include "../GPUShader.h"

namespace AT {
	class DepthPrepassShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(ObjectGroup)
			BEGIN_CONSTANTS
				DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, MVP_Matrix)
				DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Normal_Matrix)
			END_CONSTANTS
			SHADER_PARAMETER(Texture2D, BaseColor)
		END_SHADER_PARAMETER_GROUP(ObjectGroup)

		BEGIN_SHADER_PARAMETERS(DepthPrepassShaderParameters) 
			SHADER_PARAMETER_GROUP(ObjectGroup, object)
			BEGIN_STATIC_SAMPLER(MaterialSampler)
			END_STATIC_SAMPLER(MaterialSampler)
		END_SHADER_PARAMETERS(DepthPrepassShaderParameters)
		using Parameters = DepthPrepassShaderParameters;
		DepthPrepassShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			
			m_VertexShader = vs;

			m_PixelShader = ps;

			Parameters parameters;
			parameters.MaterialSampler.Filter = RHI::Filter::ANISOTROPIC;
			parameters.MaterialSampler.AddressU = RHI::TextureAddressMode::WRAP;
			parameters.MaterialSampler.AddressV = RHI::TextureAddressMode::WRAP;
			parameters.MaterialSampler.AddressW = RHI::TextureAddressMode::WRAP;
			parameters.MaterialSampler.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL;
			parameters.MaterialSampler.MaxAnisotropy = 10;
			parameters.MaterialSampler.MinLOD = 0.0f;
			parameters.MaterialSampler.MaxLOD = RHI_FLOAT32_MAX;
			parameters.MaterialSampler.MipLODBias = 0;
			parameters.MaterialSampler.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			Parameters* param = static_cast<Parameters*>(parameters);
			param->object->constant_buffer->WriteData(param->object->constants);
			m_Device->WriteDescriptorTable(param->object->m_descriptor_table, 0, 1, &param->object->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->object->m_descriptor_table, 1, 1, &param->object->BaseColor.srv);

			command_list->SetGraphicsRootDescriptorTable(0, param->object->m_descriptor_table);
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