#ifndef _DX12_FRAME_BUFFER_H_
#define _DX12_FRAME_BUFFER_H_
#include "./Headers/dx12.h"
#include "DX12RenderTargetView.h"
#include "../IFrameBuffer.h"

namespace RHI::DX12 {
	class DX12FrameBuffer : public RHI::IFrameBuffer {
	public:
		DX12FrameBuffer(const RHI::FrameBufferDescription& description);
		~DX12FrameBuffer() override;
		D3D12_CPU_DESCRIPTOR_HANDLE* GetRenderTargets() { return m_RenderTargets.data(); };
	private:
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_RenderTargets;
	};
}
#endif