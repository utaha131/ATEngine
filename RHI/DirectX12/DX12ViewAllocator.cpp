#include "DX12ViewAllocator.h"

namespace RHI::DX12 {
	DX12ViewAllocator::DX12ViewAllocator(ID3D12Device* dx12_device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, uint64_t allocate_size) :
		m_DX12Device(dx12_device),
		m_HeapType(heap_type),
		m_AllocateSize(allocate_size),
		m_IncrementSize(m_DX12Device->GetDescriptorHandleIncrementSize(m_HeapType)),
		m_HeapPool(std::vector<ID3D12DescriptorHeap*>())
	{

	}

	D3D12_CPU_DESCRIPTOR_HANDLE DX12ViewAllocator::AllocateHandle() {
		assert(m_DX12Device != nullptr);

		if (m_FreeList == nullptr) {
			D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
			heap_desc.NumDescriptors = m_AllocateSize;
			heap_desc.NodeMask = 0;
			heap_desc.Type = m_HeapType;
			heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ID3D12DescriptorHeap* new_heap;
			HRESULT result = m_DX12Device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&new_heap));

			if (FAILED(result)) {
				OutputDebugString(L"FAILED TO CREATE NEW OFFLINE HEAP.");
			}
			m_HeapPool.push_back(new_heap);

			Handle* head = m_FreeList = new Handle{};

			for (uint32_t i = 0u; i < m_AllocateSize; ++i) {
				CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(head->Native, new_heap->GetCPUDescriptorHandleForHeapStart(), i, m_IncrementSize);
				head->Next = (i == m_AllocateSize - 1) ? nullptr : new Handle{};
				head = head->Next;
			}
		}

		Handle* return_handle = m_FreeList;
		m_FreeList = m_FreeList->Next;
		D3D12_CPU_DESCRIPTOR_HANDLE return_value = return_handle->Native;
		delete return_handle;
		return return_value;
	}

	void DX12ViewAllocator::FreeHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
		Handle* current_head = m_FreeList;
		m_FreeList = new Handle{};
		m_FreeList->Native = handle;
		m_FreeList->Next = current_head;
	}
}