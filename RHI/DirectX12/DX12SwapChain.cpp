#include "DX12SwapChain.h"

namespace RHI::DX12 {
	DX12SwapChain::DX12SwapChain(const RHI::SwapChainDescription& description, IDXGISwapChain* dx12_swap_chain) :
		RHI::ISwapChain(description),
		m_DX12SwapChain(dx12_swap_chain)
	{
		for (uint32_t i = 0u; i < m_BackBuffers.size(); ++i) {
			ID3D12Resource* dx12_texture;
			m_DX12SwapChain->GetBuffer(i, IID_PPV_ARGS(&dx12_texture));
			RHI::TextureDescription texture_description;
			texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
			texture_description.Format = description.BufferFormat;
			texture_description.Width = description.Width;
			texture_description.Height = description.Height;
			texture_description.DepthOrArray = 1u;
			texture_description.MipLevels = 1u;
			texture_description.UsageFlags = RHI::TextureUsageFlag::RENDER_TARGET;
			m_BackBuffers[i] = new DX12Texture(texture_description, dx12_texture);
		}
	}

	DX12SwapChain::~DX12SwapChain() {
		for (uint32_t i = 0u; i < m_BackBuffers.size(); ++i) {
			delete m_BackBuffers[i];
		}
		m_BackBuffers.clear();
		m_DX12SwapChain->Release();
	}

	void DX12SwapChain::Resize(uint32_t width, uint32_t height) {
		m_Description.Width = width;
		m_Description.Height = height;
		for (uint32_t i = 0u; i < m_BackBuffers.size(); ++i) {
			delete m_BackBuffers[i];
		}

		m_DX12SwapChain->ResizeBuffers(m_Description.BufferCount, width, height, DX12ConvertFormat(m_Description.BufferFormat), DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		m_CurrentBufferIndex = 0u;

		for (uint32_t i = 0u; i < m_BackBuffers.size(); ++i) {
			ID3D12Resource* dx12_texture;
			m_DX12SwapChain->GetBuffer(i, IID_PPV_ARGS(&dx12_texture));
			RHI::TextureDescription texture_description;
			texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
			texture_description.Format = m_Description.BufferFormat;
			texture_description.Width = m_Description.Width;
			texture_description.Height = m_Description.Height;
			texture_description.DepthOrArray = 1u;
			texture_description.MipLevels = 1u;
			texture_description.UsageFlags = RHI::TextureUsageFlag::RENDER_TARGET;
			m_BackBuffers[i] = new DX12Texture(texture_description, dx12_texture);
		}
	}

	void DX12SwapChain::Present() {
		m_DX12SwapChain->Present(0, 0);
		m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % m_Description.BufferCount;
	}

	uint32_t DX12SwapChain::GetCurrentBackBufferIndex() {
		return m_CurrentBufferIndex;
	}
}