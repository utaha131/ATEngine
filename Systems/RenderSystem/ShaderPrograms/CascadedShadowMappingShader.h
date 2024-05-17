#ifndef _AT_RENDER_SYSTEM_CASCADED_SHADOW_MAPPING_SHADER_H_
#define _AT_RENDER_SYSTEM_CASCADED_SHADOW_MAPPING_SHADER_H_
#include "../GPUShader.h"

namespace AT {
	class CascadedShadowMappingShader : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(ObjectGroup)
		BEGIN_CONSTANTS
		DEFINE_CONSTANT(DirectX::XMFLOAT4X4A, MVP_Matrix)
		END_CONSTANTS
		END_SHADER_PARAMETER_GROUP(ObjectGroup)

		BEGIN_SHADER_PARAMETERS(CascadedShadowMappingParameters)
		SHADER_PARAMETER_GROUP(ObjectGroup, object)
		END_SHADER_PARAMETERS(CascadedShadowMappingParameters)

		using Parameters = CascadedShadowMappingParameters;
			
		CascadedShadowMappingShader(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {
			m_VertexShader = vs;

			m_PixelShader = ps;

			CascadedShadowMappingParameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {
			CascadedShadowMappingParameters* params = static_cast<CascadedShadowMappingParameters*>(parameters);
			params->object->constant_buffer->WriteData(params->object->constants);
			m_Device->WriteDescriptorTable(params->object->m_descriptor_table, 0, 1, &params->object->constant_buffer->GetNative());
			command_list->SetGraphicsRootDescriptorTable(0, params->object->m_descriptor_table);
		}

		RHI::Shader GetVertexShader() const { return m_VertexShader; }
		RHI::Shader GetPixelShader() const { return m_PixelShader; }
	private:
		RHI::Shader m_VertexShader;
		RHI::Shader m_PixelShader;
	};

	typedef CascadedShadowMappingShader CSMShader;
}
#endif