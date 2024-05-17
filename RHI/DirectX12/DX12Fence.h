#ifndef _RHI_DX12_FENCE_H_
#define _RHI_DX12_FENCE_H_
#include "../IFence.h"
#include "Headers/dx12.h"

namespace RHI::DX12 {
	class DX12Fence : public RHI::IFence {
	public:
		DX12Fence(ID3D12Fence* dx12_fence) :
			m_DX12Fence(dx12_fence) {}

		~DX12Fence() override {
			m_DX12Fence->Release();
		}

		void Signal(uint64_t value) const override {
			m_DX12Fence->Signal(value);
		}

		uint64_t GetCompletedValue() const override {
			return m_DX12Fence->GetCompletedValue();
		}

		HRESULT SetEventOnCompletion(uint64_t value, HANDLE hEvent) {
			return m_DX12Fence->SetEventOnCompletion(value, hEvent);
		}

		ID3D12Fence* GetNative() const { return m_DX12Fence; }
	private:
		ID3D12Fence* m_DX12Fence;
	};
}
#endif