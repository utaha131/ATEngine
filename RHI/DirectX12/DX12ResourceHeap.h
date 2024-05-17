#ifndef _RHI_DX12_RESOURCE_HEAP_H_
#define _RHI_DX12_RESOURCE_HEAP_H_
#include "../IResourceHeap.h"
#include "./Headers/dx12.h"

namespace RHI::DX12 {
	class DX12ResourceHeap : public RHI::IResourceHeap {
	public:
		DX12ResourceHeap(const RHI::ResourceHeapDescription& description, ID3D12Heap* dx12_resource_heap);
		~DX12ResourceHeap() override;

		inline ID3D12Heap* GetNative() const { return m_DX12ResourceHeap; }

	private:
		ID3D12Heap* m_DX12ResourceHeap;
	};
}
#endif