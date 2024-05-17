#include "DX12PipelineState.h"

namespace RHI::DX12 {
	DX12PipelineState::DX12PipelineState(const RHI::GraphicsPipelineStateDescription& description, ID3D12PipelineState* dx12_pipeline_state) :
		RHI::IPipelineState(description),
		m_DX12PipelineState(dx12_pipeline_state)
	{

	}

	DX12PipelineState::~DX12PipelineState() {
		m_DX12PipelineState->Release();
	}
}