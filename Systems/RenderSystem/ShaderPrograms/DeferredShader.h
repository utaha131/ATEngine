#ifndef _AT_RENDER_SYSTEM_DEFERRED_SHADER_H_
#define _AT_RENDER_SYSTEM_DEFERRED_SHADER_H_
#include "../GPUShader.h"
#define MAXIMUM_LIGHT_SOURCES 5
namespace AT {
	
	class DeferredShader : public GPUShader {
	public:
		struct LightInput {
			Light lights[5];
		};
		BEGIN_SHADER_PARAMETER_GROUP(DEFERREDINPUT)
		BEGIN_CONSTANTS
			DEFINE_CONSTANT(LightInput, Lights)
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, InverseProjectionMatrix)
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, InverseViewMatrix)
		END_CONSTANTS
		SHADER_PARAMETER(Texture2D, BaseColorTexture)
		SHADER_PARAMETER(Texture2D, NormalTexture)
		SHADER_PARAMETER(Texture2D, SurfaceTexture)
		SHADER_PARAMETER(Texture2D, DepthTexture)
		
		SHADER_PARAMETER_ARRAY(Texture2D, CSM, 5)
		SHADER_PARAMETER_ARRAY(Texture2D, OSM, 5)
		END_SHADER_PARAMETER_GROUP(DEFERREDINPUT)

		BEGIN_SHADER_PARAMETERS(DeferredParameters)
		SHADER_PARAMETER_GROUP(DEFERREDINPUT, Pass)
		BEGIN_STATIC_SAMPLER(Sampler)
			.Filter = RHI::Filter::MIN_MAG_MIP_POINT,
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
		BEGIN_STATIC_SAMPLER(ShadowSampler)
			.Filter = RHI::Filter::MIN_MAG_LINEAR_MIP_POINT,
			.AddressU = RHI::TextureAddressMode::CLAMP,
			.AddressV = RHI::TextureAddressMode::CLAMP,
			.AddressW = RHI::TextureAddressMode::CLAMP,
			.MipLODBias = 0,
			.MaxAnisotropy = 1,
			.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL,
			.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK,
			.MinLOD = 0.0f,
			.MaxLOD = RHI_FLOAT32_MAX,
		END_STATIC_SAMPLER(ShadowSampler)
		END_SHADER_PARAMETERS(DeferredParameters)

		using Parameters = DeferredParameters;
		DeferredShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice())
		{
			m_VertexShader = vs;

			m_PixelShader = ps;

			DeferredParameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}
		
		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			Parameters* param = static_cast<Parameters*>(parameters);
			param->Pass->constant_buffer->WriteData(param->Pass->constants);
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 0, 1, &param->Pass->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 1, 1, &param->Pass->BaseColorTexture.srv);
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 2, 1, &param->Pass->NormalTexture.srv);
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 3, 1, &param->Pass->SurfaceTexture.srv);
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 4, 1, &param->Pass->DepthTexture.srv);

			RHI::ShaderResourceView csm_srvs[5] = { param->Pass->CSM[0].srv, param->Pass->CSM[1].srv, param->Pass->CSM[2].srv, param->Pass->CSM[3].srv, param->Pass->CSM[4].srv };
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 5, 5, csm_srvs);
			RHI::ShaderResourceView osm_srvs[5] = { param->Pass->OSM[0].srv, param->Pass->OSM[1].srv, param->Pass->OSM[2].srv, param->Pass->OSM[3].srv, param->Pass->OSM[4].srv };
			m_Device->WriteDescriptorTable(param->Pass->m_descriptor_table, 10, 5, osm_srvs);
			command_list->SetGraphicsRootDescriptorTable(0, param->Pass->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const { return m_VertexShader; }
		RHI::Shader GetPixelShader() const { return m_PixelShader; }
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};
}
#endif