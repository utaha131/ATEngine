#ifndef _AT_PFEM_SHADER_H_
#define _AT_PFEM_SHADER_H_
#include "../GPUShader.h"

namespace AT {
	class PreFilteredEnviromentMapShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(PassGroup)
		BEGIN_CONSTANTS
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4, MV_Matrix)
			DEFINE_CONSTANT(float, Roughness)
			DEFINE_CONSTANT(uint32_t, Mip_Slice)
		END_CONSTANTS
		SHADER_PARAMETER(TextureCube, Enviroment_Map)
		END_SHADER_PARAMETER_GROUP(PassGroup)

		BEGIN_SHADER_PARAMETERS(PreFilteredEnviromentMapShaderParameters)
		SHADER_PARAMETER_GROUP(PassGroup, Pass)
		BEGIN_STATIC_SAMPLER(Sampler)
		END_STATIC_SAMPLER(Sampler)
		END_SHADER_PARAMETERS(PreFilteredEnviromentMapShaderParameters)

		using Parameters = PreFilteredEnviromentMapShaderParameters;

		PreFilteredEnviromentMapShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;

			m_PixelShader = ps;

			Parameters parameters;
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
			params->Pass->constant_buffer->WriteData(params->Pass->constants);
			m_Device->WriteDescriptorTable(params->Pass->m_descriptor_table, 0, 1, &params->Pass->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(params->Pass->m_descriptor_table, 1, 1, &params->Pass->Enviroment_Map.srv);
			command_list->SetGraphicsRootDescriptorTable(0, params->Pass->m_descriptor_table);
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