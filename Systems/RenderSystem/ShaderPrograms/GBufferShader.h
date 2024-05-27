#ifndef _AT_RENDER_SYSTEM_GBUFFER_SHADER_H_
#define _AT_RENDER_SYSTEM_GBUFFER_SHADER_H_
#include "../GPUShader.h"
#include "../Systems/RenderSystem/MaterialManager.h"

namespace AT {
	class GBufferShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(ObjectGroup)
		BEGIN_CONSTANTS
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, ModelViewProjectionMatrix)
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, NormalMatrix)
		END_CONSTANTS
		END_SHADER_PARAMETER_GROUP(ObjectGroup)

		BEGIN_SHADER_PARAMETERS(GBufferParameters)
		SHADER_PARAMETER_GROUP(MaterialManager::MaterialGroup, Material)
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
		END_SHADER_PARAMETERS(GBufferParameters)

		using Parameters = GBufferParameters;
		GBufferShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;

			m_PixelShader = ps;

			GBufferParameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			GBufferParameters* param = static_cast<GBufferParameters*>(parameters);
			param->Object->constant_buffer->WriteData(param->Object->constants);
			m_Device->WriteDescriptorTable(param->Object->m_descriptor_table, 0, 1, &param->Object->constant_buffer->GetNative());

			m_Device->WriteDescriptorTable(param->Material->m_descriptor_table, 0, 1, &param->Material->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->Material->m_descriptor_table, 1, 1, &param->Material->BaseColor.srv);
			m_Device->WriteDescriptorTable(param->Material->m_descriptor_table, 2, 1, &param->Material->Normals.srv);
			m_Device->WriteDescriptorTable(param->Material->m_descriptor_table, 3, 1, &param->Material->Surface.srv);

			command_list->SetGraphicsRootDescriptorTable(0, param->Material->m_descriptor_table);
			command_list->SetGraphicsRootDescriptorTable(1, param->Object->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const { return m_VertexShader; }
		RHI::Shader GetPixelShader() const { return m_PixelShader;  }
	protected:
		void GenerateRootSignature(const ShaderParameters parameter) {}
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};
}
#endif