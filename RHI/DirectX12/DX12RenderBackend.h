#ifndef _RHI_DX12_RENDER_BACKEND_H_
#define _RHI_DX12_RENDER_BACKEND_H_
#include "../IRenderBackend.h"
#include "./Headers/dx12.h"
#include "DX12Adapter.h"
#include "DX12SwapChain.h"

namespace RHI::DX12 {
	class DX12RenderBackend : public RHI::IRenderBackend {
	public:
		DX12RenderBackend(bool debug);
		~DX12RenderBackend() override;
		RHI::Result GetAdapters(std::vector<RHI::Adapter>& adapters) override;
		RHI::Result CreateDevice(const RHI::Adapter adapter, RHI::Device& device) override;
		RHI::Result CreateSwapChain(RHI::Device device, const RHI::SwapChainDescription& description, RHI::SwapChain& swap_chain) override;
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_Factory;
		Microsoft::WRL::ComPtr<ID3D12Debug> m_DebugController;
	};
}
#endif