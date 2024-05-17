#include "DX12DescriptorHeap.h"

namespace RHI::DX12 {
	DX12DescriptorHeap::DX12DescriptorHeap(uint32_t increment_size, ID3D12DescriptorHeap* dx12_descriptor_heap, uint64_t max_size) :
		m_DX12DescriptorHeap(dx12_descriptor_heap),
		m_IncrementSize(increment_size)
	{
		m_StartCPUHandle = m_DX12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_StartGPUHandle = m_DX12DescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		m_FreeList = new Block{};
		m_FreeList->size = max_size;
		m_FreeList->next = nullptr;
	}

	DX12DescriptorHeap::~DX12DescriptorHeap() {

	}

	RHI::DescriptorTable DX12DescriptorHeap::AllocateTable(const RHI::RootDescriptorTable & root_descriptor_table) {
		uint64_t descriptor_count = 0ull;
		for (uint32_t i = 0u; i < root_descriptor_table.Ranges.size(); ++i) {
			descriptor_count += root_descriptor_table.Ranges[i].DescriptorCount;
		}


		AllocationInfo info = FindBlock(descriptor_count);

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_handle{ m_StartCPUHandle };
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpu_handle{ m_StartGPUHandle };
		RHI::DescriptorTable table = new DX12DescriptorTable(cpu_handle.Offset(info.offset, m_IncrementSize), gpu_handle.Offset(info.offset, m_IncrementSize), *this, info);
		return table;
	}

	DX12DescriptorTable::~DX12DescriptorTable() {
		m_DescriptorHeap.FreeTable(this);
	}
}