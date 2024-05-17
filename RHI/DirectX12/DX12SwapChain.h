#ifndef _RHI_DX12_SWAP_CHAIN_H_
#define _RHI_DX12_SWAP_CHAIN_H_
#include "../ISwapChain.h"
#include "Headers/dx12.h"
#include "DX12Texture.h"
#include "DX12Device.h"


namespace RHI::DX12 {
	class DX12SwapChain : public RHI::ISwapChain {
	public:
		DX12SwapChain(const RHI::SwapChainDescription& description, IDXGISwapChain* dx12_swap_chain);
		~DX12SwapChain() override;

		inline IDXGISwapChain* GetNative() const { return m_DX12SwapChain; }
		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;
		uint32_t GetCurrentBackBufferIndex() override;
	private:
		IDXGISwapChain* m_DX12SwapChain;
	};
}
#endif