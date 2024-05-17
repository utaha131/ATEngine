#ifndef _AT_ENV_BRDF_H_
#define _AT_ENV_BRDF_H_
#include "../GPUShader.h"

namespace AT {
	class EnvBRDF : public GPUShader {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(PassGroup)
			SHADER_PARAMETER(Texture2D, empty)
		END_SHADER_PARAMETER_GROUP(PassGroup)

		BEGIN_SHADER_PARAMETERS(EnvBRDFShaderParameters)
		SHADER_PARAMETER_GROUP(PassGroup, Pass)
		END_SHADER_PARAMETERS(EnvBRDFShaderParameters)

		using Parameters = EnvBRDFShaderParameters;

		EnvBRDF(RHI::Shader vs, RHI::Shader ps, GPURootSignatureManager& root_signature_manager) : GPUShader(root_signature_manager.GetDevice()) {

			m_VertexShader = vs;

			m_PixelShader = ps;

			Parameters parameters;
			m_RootSignature = root_signature_manager.CreateOrGetRootSignature(parameters.root_signature_description);
		}

		void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const override {

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