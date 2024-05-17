#include "DX12FrameBuffer.h"

namespace RHI::DX12 {
	DX12FrameBuffer::DX12FrameBuffer(const RHI::FrameBufferDescription& description) :
		RHI::IFrameBuffer(description),
		m_RenderTargets(std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>(description.RenderTargetViews.size()))
	{
		for (uint32_t i = 0u; i < m_RenderTargets.size(); ++i) {
			m_RenderTargets[i] = static_cast<const DX12RenderTargetView*>(description.RenderTargetViews[i])->GetNative();
		}
	}

	DX12FrameBuffer::~DX12FrameBuffer() {
		m_RenderTargets.clear();
	}
}