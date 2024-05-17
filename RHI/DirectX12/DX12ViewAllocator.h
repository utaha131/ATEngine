#ifndef _RHI_DX12_VIEW_ALLOCATOR_H_
#define _RHI_DX12_VIEW_ALLOCATOR_H_
#include "./Headers/dx12.h"
#include "../RHICore.h"
#include <vector>
#include <queue>
#include <iostream>

namespace RHI::DX12 {
	class DX12ViewAllocator {
	public:
		DX12ViewAllocator() :
			m_DX12Device(nullptr),
			m_HeapType(D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES),
			m_IncrementSize(0u),
			m_AllocateSize(0u)
		{

		}
		~DX12ViewAllocator() {
			Handle* head = m_FreeList;
			while (head != nullptr) {
				Handle* current = head;
				head = head->Next;
				delete current;
			}
			for (uint32_t i = 0; i < m_HeapPool.size(); ++i) {
				m_HeapPool[i]->Release();
			}
			m_HeapPool.clear();
		}
		DX12ViewAllocator(ID3D12Device* dx12_device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, uint64_t allocate_size);
		D3D12_CPU_DESCRIPTOR_HANDLE AllocateHandle();
		void FreeHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle);
	private:

		ID3D12Device* m_DX12Device;
		std::vector<ID3D12DescriptorHeap*> m_HeapPool;
		struct Handle {
			D3D12_CPU_DESCRIPTOR_HANDLE Native;
			Handle* Next;
		};
		D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
		Handle* m_FreeList = nullptr;
		uint32_t m_AllocateSize;
		uint32_t m_IncrementSize;
	};
}
#endif