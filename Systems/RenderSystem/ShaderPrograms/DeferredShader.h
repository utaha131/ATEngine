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
			DEFINE_CONSTANT(LightInput, lights)
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Inverse_Projection_Matrix)
			DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Inverse_View_Matrix)
			DEFINE_CONSTANT(bool, Use_Enviroment_Map)
		END_CONSTANTS
		SHADER_PARAMETER(Texture2D, Base_Color)
		SHADER_PARAMETER(Texture2D, Normals)
		SHADER_PARAMETER(Texture2D, Surface)
		SHADER_PARAMETER(Texture2D, Depth)
		SHADER_PARAMETER(Texture2D, SSAO)
		
		SHADER_PARAMETER_ARRAY(Texture2D, CSM, 5)
		SHADER_PARAMETER_ARRAY(Texture2D, OSM, 5)
		SHADER_PARAMETER(Texture2D, Irradiance)
		SHADER_PARAMETER(TextureCube, prefiltered_map)
		SHADER_PARAMETER(Texture2D, env_brdf)
		END_SHADER_PARAMETER_GROUP(DEFERREDINPUT)

		BEGIN_SHADER_PARAMETERS(DeferredParameters)
		SHADER_PARAMETER_GROUP(DEFERREDINPUT, pass)
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
			.MaxAnisotropy = 10,
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
			{
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
			}
			{
				parameters.ShadowSampler.Filter = RHI::Filter::MIN_MAG_MIP_LINEAR;
				parameters.ShadowSampler.AddressU = RHI::TextureAddressMode::CLAMP;
				parameters.ShadowSampler.AddressV = RHI::TextureAddressMode::CLAMP;
				parameters.ShadowSampler.AddressW = RHI::TextureAddressMode::CLAMP;
				parameters.ShadowSampler.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL;
				parameters.ShadowSampler.MaxAnisotropy = 10;
				parameters.ShadowSampler.MinLOD = 0.0f;
				parameters.ShadowSampler.MaxLOD = RHI_FLOAT32_MAX;
				parameters.ShadowSampler.MipLODBias = 0;
				parameters.ShadowSampler.BorderColor = RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK;
			}
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}
		
		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			Parameters* param = static_cast<Parameters*>(parameters);
			param->pass->constant_buffer->WriteData(param->pass->constants);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 0, 1, &param->pass->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 1, 1, &param->pass->Base_Color.srv);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 2, 1, &param->pass->Normals.srv);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 3, 1, &param->pass->Surface.srv);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 4, 1, &param->pass->Depth.srv);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 5, 1, &param->pass->SSAO.srv);

			RHI::ShaderResourceView csm_srvs[5] = { param->pass->CSM[0].srv, param->pass->CSM[1].srv, param->pass->CSM[2].srv, param->pass->CSM[3].srv, param->pass->CSM[4].srv };
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 6, 5, csm_srvs);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 7, param->pass->CSM[1].srv);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 8, param->pass->CSM[2].srv);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 9, param->pass->CSM[3].srv);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 10, param->pass->CSM[4].srv);
			
			RHI::ShaderResourceView osm_srvs[5] = { param->pass->OSM[0].srv, param->pass->OSM[1].srv, param->pass->OSM[2].srv, param->pass->OSM[3].srv, param->pass->OSM[4].srv };
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 11, 5, osm_srvs);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 12, param->pass->OSM[1].srv);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 13, param->pass->OSM[2].srv);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 14, param->pass->OSM[3].srv);
			//m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 15, param->pass->OSM[4].srv);

			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 16, 1, &param->pass->Irradiance.srv);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 17, 1, &param->pass->prefiltered_map.srv);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 18, 1, &param->pass->env_brdf.srv);
			command_list->SetGraphicsRootDescriptorTable(0, param->pass->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const { return m_VertexShader; }
		RHI::Shader GetPixelShader() const { return m_PixelShader; }
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};
}
#endif