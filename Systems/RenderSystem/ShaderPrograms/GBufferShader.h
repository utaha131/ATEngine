#ifndef _AT_RENDER_SYSTEM_GBUFFER_SHADER_H_
#define _AT_RENDER_SYSTEM_GBUFFER_SHADER_H_
#include "../GPUShader.h"
#include "../Systems/RenderSystem/MaterialManager.h"

namespace AT {
	class GBufferShader : public GPUShader {
	public:
		/*BEGIN_SHADER_PARAMETER_GROUP(MaterialGroup)
		BEGIN_CONSTANTS()
			DEFINE_CONSTANT(DirectX::XMFLOAT3, Base_Color_Factor)
		END_CONSTANTS()
		SHADER_PARAMETER(Texture2D, BaseColor)
		SHADER_PARAMETER(Texture2D, Normals)
		SHADER_PARAMETER(Texture2D, Surface)
		END_SHADER_PARAMETER_GROUP(MaterialGroup)*/

		BEGIN_SHADER_PARAMETER_GROUP(ObjectGroup)
		BEGIN_CONSTANTS
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, MVP_Matrix)
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Normal_Matrix)
		END_CONSTANTS
		END_SHADER_PARAMETER_GROUP(ObjectGroup)

		BEGIN_SHADER_PARAMETERS(GBufferParameters)
		SHADER_PARAMETER_GROUP(MaterialManager::MaterialGroup, material)
		SHADER_PARAMETER_GROUP(ObjectGroup, object)
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

		/*
			RHI_FILTER Filter;
		RHI_TEXTURE_ADDRESS_MODE Address_U;
		RHI_TEXTURE_ADDRESS_MODE Address_V;
		RHI_TEXTURE_ADDRESS_MODE Address_W;
		RHI_FLOAT32 Mip_LOD_Bias;
		RHI_UINT32 Max_Anisotropy;
		RHI_COMPARISON_FUNCTION Comparison_Function;
		RHI_FLOAT32 Border_Color[4];
		RHI_FLOAT32 Min_LOD;
		RHI_FLOAT32 Max_LOD;
		RHI_UINT32 Shader_Register;
		RHI_UINT32 Register_Space;
		*/

		GBufferShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;

			m_PixelShader = ps;

			GBufferParameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			GBufferParameters* param = static_cast<GBufferParameters*>(parameters);
			param->object->constant_buffer->WriteData(param->object->constants);
			m_Device->WriteDescriptorTable(param->object->m_descriptor_table, 0, 1, &param->object->constant_buffer->GetNative());

			m_Device->WriteDescriptorTable(param->material->m_descriptor_table, 0, 1, &param->material->constant_buffer->GetNative());
			m_Device->WriteDescriptorTable(param->material->m_descriptor_table, 1, 1, &param->material->BaseColor.srv);
			m_Device->WriteDescriptorTable(param->material->m_descriptor_table, 2, 1, &param->material->Normals.srv);
			m_Device->WriteDescriptorTable(param->material->m_descriptor_table, 3, 1, &param->material->Surface.srv);

			command_list->SetGraphicsRootDescriptorTable(0, param->material->m_descriptor_table);
			command_list->SetGraphicsRootDescriptorTable(1, param->object->m_descriptor_table);
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