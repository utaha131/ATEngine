#include "DX12RenderBackend.h"

namespace RHI::DX12 {
	DX12RenderBackend::DX12RenderBackend(bool debug) :
		RHI::IRenderBackend(debug)
	{
		HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory));

		if (FAILED(result)) {
		}

		if (m_Debug) {
			result = D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugController));

			if (FAILED(result)) {

			}
			m_DebugController->EnableDebugLayer();
		}
	}

	DX12RenderBackend::~DX12RenderBackend() {
		m_Factory->Release();
		if (m_Debug) {
			m_DebugController->Release();
		}
	}

	RHI::Result DX12RenderBackend::GetAdapters(std::vector<RHI::Adapter>& adapters) {
		IDXGIAdapter* dx12_adapter;
		for (UINT i = 0; m_Factory->EnumAdapters(i, &dx12_adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC desc;
			dx12_adapter->GetDesc(&desc);
			RHI::Vendor vendor = RHI::Vendor::INTEL;
			switch (desc.VendorId) {
			case 0x10DE:
				vendor = RHI::Vendor::NVIDIA;
				break;
			case  0x8086:
				vendor = RHI::Vendor::INTEL;
				break;
			}
			adapters.push_back(new DX12Adapter(desc.DedicatedVideoMemory, vendor, dx12_adapter));
		}
		return RHI::Result::SUCCESS;
	}

	RHI::Result DX12RenderBackend::CreateDevice(const RHI::Adapter adapter, RHI::Device& device) {
		ID3D12Device5* dx12_device;
		if (FAILED(D3D12CreateDevice(static_cast<const DX12Adapter*>(adapter)->GetNative(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dx12_device)))) {
			return RHI::Result::E_CREATE_DEVICE;
		}
		device = new DX12Device(dx12_device);
		return RHI::Result::SUCCESS;
	}

	RHI::Result DX12RenderBackend::CreateSwapChain(RHI::Device device, const RHI::SwapChainDescription& description, RHI::SwapChain& swap_chain) {
		DXGI_SWAP_CHAIN_DESC swap_chain_description;
		swap_chain_description.BufferDesc.Width = description.Width;
		swap_chain_description.BufferDesc.Height = description.Height;
		swap_chain_description.BufferDesc.RefreshRate.Numerator = 0;
		swap_chain_description.BufferDesc.RefreshRate.Denominator = 0;
		swap_chain_description.BufferDesc.Format = DX12ConvertFormat(description.BufferFormat);
		swap_chain_description.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swap_chain_description.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swap_chain_description.SampleDesc.Count = 1u;
		swap_chain_description.SampleDesc.Quality = 0u;
		swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_description.BufferCount = description.BufferCount;
		swap_chain_description.OutputWindow = static_cast<HWND>(description.Window);//TODO SHOULD BE WINDOW HANDLE wmInfo.info.win.window; //TODO Change this.
		swap_chain_description.Windowed = true;
		swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* dx12_swap_chain;


		if (FAILED(m_Factory->CreateSwapChain(static_cast<const DX12Device*>(device)->m_DX12CommandQueues[(uint32_t)RHI::CommandType::DIRECT], &swap_chain_description, &dx12_swap_chain))) {
			return RHI::Result::E_CREATE_SWAP_CHAIN;
		}

		swap_chain = new DX12SwapChain(description, dx12_swap_chain);
		return RHI::Result::SUCCESS;
	}
}