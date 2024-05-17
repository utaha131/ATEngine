#ifndef _AT_RENDER_SYSTEM_OMNIDIRECTIONAL_SHADOW_MAPPING_SHADER_H_
#define _AT_RENDER_SYSTEM_OMNIDIRECTIONAL_SHADOW_MAPPING_SHADER_H_
#include "../GPUShader.h"
namespace AT {
	class OmnidirectionalShadowMappingShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(PassGroup)
		BEGIN_CONSTANTS
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, MVP_Matrix)
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, Model_Matrix)
		DEFINE_CONSTANT(DirectX::XMFLOAT4A, Light_Position)
		DEFINE_CONSTANT(float, Max_Distance)
		END_CONSTANTS
		END_SHADER_PARAMETER_GROUP(PassGroup)

		BEGIN_SHADER_PARAMETERS(OmnidirectionalShadowMappingParameters)
		SHADER_PARAMETER_GROUP(PassGroup, pass)
		END_SHADER_PARAMETERS(OmnidirectionalShadowMappingParameters)

		using Parameters = OmnidirectionalShadowMappingParameters;

		OmnidirectionalShadowMappingShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {

			m_VertexShader = vs;

			m_PixelShader = ps;

			OmnidirectionalShadowMappingParameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			OmnidirectionalShadowMappingParameters* param = static_cast<OmnidirectionalShadowMappingParameters*>(parameters);
			param->pass->constant_buffer->WriteData(param->pass->constants);
			m_Device->WriteDescriptorTable(param->pass->m_descriptor_table, 0, 1, &param->pass->constant_buffer->GetNative());
			command_list->SetGraphicsRootDescriptorTable(0, param->pass->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() { return m_VertexShader; }
		RHI::Shader GetPixelShader() { return m_PixelShader; }
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};

	typedef OmnidirectionalShadowMappingShader OSMShader;
}
#endif