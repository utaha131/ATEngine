#include "DX12Adapter.h"

namespace RHI::DX12 {
	DX12Adapter::DX12Adapter(uint64_t vram, RHI::Vendor vendor, IDXGIAdapter* dx12_adapter) :
		RHI::IAdapter(vram, vendor),
		m_DX12Adapter(dx12_adapter)
	{

	}

	DX12Adapter::~DX12Adapter() {
		m_DX12Adapter->Release();
	}
}