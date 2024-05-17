#ifndef _RHI_DX12_ADAPTER_H_
#define _RHI_DX12_ADAPTER_H_
#include "../IAdapter.h"
#include "./Headers/dx12.h"

namespace RHI::DX12 {
	class DX12Adapter : public RHI::IAdapter {
	public:
		DX12Adapter(uint64_t vram, RHI::Vendor vendor, IDXGIAdapter* dx12_adapter);
		~DX12Adapter() override;
		inline IDXGIAdapter* GetNative() const { return m_DX12Adapter; }
	private:
		IDXGIAdapter* m_DX12Adapter;
	};
}
#endif