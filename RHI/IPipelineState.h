#ifndef _RHI_I_PIPELINE_STATE_H_
#define _RHI_I_PIPELINE_STATE_H_
#include "RHICore.h"
#include <assert.h>

namespace RHI {
	typedef struct GraphicsPipelineStateDescription {
		RHI::RootSignature RootSignature = RHI_NULL_HANDLE;
		RHI::Shader VertexShader = RHI_NULL_HANDLE;
		RHI::Shader DomainShader = RHI_NULL_HANDLE;
		RHI::Shader HullShader = RHI_NULL_HANDLE;
		RHI::Shader GeometryShader = RHI_NULL_HANDLE;
		RHI::Shader PixelShader = RHI_NULL_HANDLE;
		RHI::BlendDescription BlendDescription;
		RHI::RasterizerDescription RasterizerDescription;
		RHI::DepthStencilDescription DepthStencilDescription;
		RHI::InputLayout InputLayout;
		RHI::PrimitiveTopology PrimitiveTopology;
		uint32_t NumberOfRenderTargets;
		RHI::Format RenderTargetFormats[8];
		RHI::Format DepthFormat;
		RHI::SampleDescription SampleDescription;
		uint32_t NodeMask;
	} GraphicsPipelineStateDescription;

	typedef struct ComputePipelineStateDescription {
		RHI::RootSignature RootSignature = RHI_NULL_HANDLE;
		RHI::Shader ComputeShader = RHI_NULL_HANDLE;
		uint32_t NodeMask;
	} ComputePipelineStateDescription;

	class IPipelineState {
	public:
		virtual ~IPipelineState() {}
		const GraphicsPipelineStateDescription& GetDescriptionGraphics() const {
			assert(m_Type == PipelineType::GRAPHICS);
			return m_DescriptionGraphics;
		};

		const ComputePipelineStateDescription& GetDescriptionCompute() const {
			assert(m_Type == PipelineType::COMPUTE);
			return m_DescriptionCompute;
		};
	protected:
		enum class PipelineType {
			GRAPHICS,
			COMPUTE
		} m_Type;

		union {
			GraphicsPipelineStateDescription m_DescriptionGraphics;
			ComputePipelineStateDescription m_DescriptionCompute;
		};
		IPipelineState(const GraphicsPipelineStateDescription& description) :
			m_Type(PipelineType::GRAPHICS),
			m_DescriptionGraphics(description) {};
		IPipelineState(const ComputePipelineStateDescription& description) :
			m_Type(PipelineType::COMPUTE),
			m_DescriptionCompute(description) {};
	};
}
#endif