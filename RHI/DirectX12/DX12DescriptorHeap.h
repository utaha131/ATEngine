#ifndef _DX12_DESCRIPTOR_HEAP_H_
#define _DX12_DESCRIPTOR_HEAP_H_
#include "Headers/dx12.h"
#include "DX12RootSignature.h"
#include "../IDescriptorTable.h"
#include <list>

namespace RHI::DX12 {
	class DX12DescriptorHeap;

	struct AllocationInfo {
		uint64_t offset;
		uint64_t size;
	};

	//Free List Descriptor Range Allocator.
	class DX12DescriptorTable : public RHI::IDescriptorTable {
	public:
		DX12DescriptorTable(D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle, DX12DescriptorHeap& descriptor_heap, AllocationInfo& allocation_info) :
			m_DescriptorHeap(descriptor_heap),
			m_AllocationInfo(allocation_info),
			m_CPUHandle(cpu_handle),
			m_GPUHandle(gpu_handle)
		{

		}

		~DX12DescriptorTable() override;

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_CPUHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_GPUHandle; }
	private:
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
		DX12DescriptorHeap& m_DescriptorHeap;
	public:
		AllocationInfo m_AllocationInfo;
		friend DX12DescriptorHeap;
	};

	class DX12DescriptorHeap {
	public:
		DX12DescriptorHeap(uint32_t increment_size, ID3D12DescriptorHeap* dx12_descriptor_heap, uint64_t max_size);
		~DX12DescriptorHeap();
		RHI::DescriptorTable AllocateTable(const RHI::RootDescriptorTable& root_descriptor_table);
		void FreeTable(RHI::DescriptorTable table) {
			FreeBlock(static_cast<DX12DescriptorTable*>(table)->m_AllocationInfo);
		}
		ID3D12DescriptorHeap* GetNative() const { return m_DX12DescriptorHeap; }
	private:
		struct Block {
			uint64_t offset;
			uint64_t size;
			Block* next;
		};

		void Coalesce(Block* previous, Block* current, Block* next) {
			if (current != nullptr && current->offset + current->size == next->offset) {
				current->size += next->size;
				if (next->next != nullptr) {
					current->next = next->next;
				}
				else {
					current->next = nullptr;
				}
				delete next;
			}

			if (previous != nullptr && previous->offset + previous->size == current->offset) {
				previous->size += current->size;
				if (current->next != nullptr) {
					previous->next = current->next;
				}
				else {
					previous->next = nullptr;
				}
				delete current;
			}
		}

		void FreeBlock(AllocationInfo& allocation) {

			Block* freed_block = new Block{};
			freed_block->offset = allocation.offset;
			freed_block->size = allocation.size;
			freed_block->next = nullptr;

			if (m_FreeList == nullptr) {
				m_FreeList = freed_block;
				return;
			}
			else if (freed_block->offset < m_FreeList->offset) {
				freed_block->next = m_FreeList;
				m_FreeList = freed_block;
				Coalesce(nullptr, freed_block, freed_block->next);
				return;
			}

			Block* head = m_FreeList->next, * previous = m_FreeList;

			while (head != nullptr) {
				if (previous->offset < freed_block->offset && freed_block->offset < head->offset) {
					previous->next = freed_block;
					freed_block->next = head;
					Coalesce(previous, freed_block, head);
					return;
				}
				previous = head;
				head = head->next;
			}

			previous->next = freed_block;
		}

		AllocationInfo FindBlock(uint64_t size) {
			assert(m_FreeList != nullptr);

			Block* head = m_FreeList, * previous = nullptr, * smallest = nullptr, * smallest_previous = nullptr;

			while (head != nullptr) {
				if (head->size == size) {
					smallest = head;
					smallest_previous = previous;
					break;
				}
				else if (size < head->size) {
					if (smallest == nullptr) {
						smallest_previous = previous;
						smallest = head;
						continue;
					}
					else if (head->size < smallest->size) {
						smallest_previous = previous;
						smallest = head;
					}
				}
				previous = head;
				head = head->next;
			}

			AllocationInfo allocation;
			allocation.offset = smallest->offset;
			allocation.size = size;

			smallest->offset += size;
			smallest->size -= size;
			if (smallest->size == 0) {
				if (smallest_previous != nullptr) {
					smallest_previous->next = smallest->next;
				}
				else {
					m_FreeList = smallest->next;
				}
				delete smallest;
			}

			return allocation;
		}
	private:
		ID3D12DescriptorHeap* m_DX12DescriptorHeap;
		uint64_t m_IncrementSize;
		D3D12_CPU_DESCRIPTOR_HANDLE m_StartCPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_StartGPUHandle;
		Block* m_FreeList;
	};
}
#endif