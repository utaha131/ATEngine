#ifndef _AT_RENDER_SYSTEM_GPU_MEMORY_ALLOCATOR_H_
#define _AT_RENDER_SYSTEM_GPU_MEMORY_ALLOCATOR_H_
#include <stdint.h>
#include "../RHI/RHI.h"

namespace AT {
	class GPUMemoryAllocator {
	public:
		class LinearAllocator{
		public:
			struct Allocation {
				uint64_t offset;
				uint64_t padding;
				uint64_t size = 0;
			};
			LinearAllocator(RHI::ResourceHeap heap, uint64_t max_size) :
				m_heap(heap),
				m_offset(0),
				m_max_size(max_size)
			{

			}

			Allocation Allocate(uint64_t size, uint64_t alignment) {
				assert(alignment > 0, "Alignment must be greater than 0.");
				uint64_t b = alignment - 1;
				uint64_t padding = ((m_offset + b) & ~b) - m_offset;
				assert(m_offset + padding + size < m_max_size, "Out of Memory.");
				Allocation allocation;
				allocation.offset = m_offset;
				allocation.padding = padding;
				allocation.size = size;
				m_offset += padding + size;
				return allocation;
			}

			void Free(const Allocation& allocation) {
				assert(false, "Use LinearAllocator.clear() instead.");
			}

			void Clear() {
				m_offset = 0;
			}
			
			RHI::ResourceHeap GetResourceHeap() const {
				return m_heap;
			}

		private:
			uint64_t m_offset;
			uint64_t m_max_size;
			RHI::ResourceHeap m_heap;
		};

		class FreeListAllocator {
		public:
			struct Memory {
				uint64_t offset;
				uint64_t size;
				Memory* next;
			};

			struct Allocation {
				uint64_t offset;
				uint64_t padding;
				uint64_t size;
			};

			FreeListAllocator() {

			}

			FreeListAllocator(RHI::ResourceHeap heap, uint64_t max_size) :
				m_heap(heap),
				m_max_size(max_size)
			{
				m_free_list = new Memory{};
				m_free_list->offset = 0;
				m_free_list->size = max_size;
				m_free_list->next = nullptr;
			}

			Allocation Allocate(uint64_t size, uint64_t alignment) {
				assert(alignment > 0, "Alignment must be greater than 0.");

				//Best Fit Algoritm to reduce Fragmentation.
				assert(m_free_list != nullptr, "FreeListAllocator No more Memory.");
				uint64_t b = alignment - 1;
				Memory* head = m_free_list, *previous = nullptr, *smallest = nullptr, *smallest_previous = nullptr;
				uint64_t padding = 0;
				while (head != nullptr) {
					uint64_t pad = ((head->offset + b) & ~b) - head->offset;
					if (head->size == size + pad) {
						smallest = head;
						smallest_previous = previous;
						padding = pad;
						break;
					}
					else if (size + pad < head->size) {
						if (smallest == nullptr) {
							smallest_previous = previous;
							padding = pad;
							smallest = head;
							continue;
						} else if (head->size < smallest->size) {
							smallest_previous = previous;
							padding = pad;
							smallest = head;
						}
					}
					previous = head;
					head = head->next;
				}

				assert(smallest != nullptr, "FreeListAllocator Memory Insufficient.");
			
				Allocation allocation;
				allocation.offset = smallest->offset;
				allocation.padding = padding;
				allocation.size = size;
				
				smallest->offset += padding + size;
				smallest->size -= padding + size;
				if (smallest->size == 0) {
					if (smallest_previous != nullptr) {
						smallest_previous->next = smallest->next;
					}
					else {
						m_free_list = smallest->next;
					}
					delete smallest;
				}

				return allocation;
			}

			void Free(const Allocation& allocation) {
				Memory* freed_memory = new Memory{};
				freed_memory->offset = allocation.offset;
				freed_memory->size = allocation.size;
				freed_memory->next = nullptr;

				if (m_free_list == nullptr) {
					m_free_list = freed_memory;
					return;
				}
				else if (freed_memory->offset < m_free_list->offset) {
					freed_memory->next = m_free_list;
					m_free_list = freed_memory;
					Coalesce(nullptr, freed_memory, freed_memory->next);
					return;
				}

				Memory* head = m_free_list->next, * previous = m_free_list;

				while (head != nullptr) {
					if (previous->offset < freed_memory->offset && freed_memory->offset < head->offset) {
						previous->next = freed_memory;
						freed_memory->next = head;
						Coalesce(previous, freed_memory, head);
						return;
					}
					previous = head;
					head = head->next;
				}

				previous->next = freed_memory;
			}

			RHI::ResourceHeap GetResourceHeap() const { 
				return m_heap; 
			}

		protected:
			void Coalesce(Memory* previous, Memory* current, Memory* next) {
				if (current->offset + current->size == next->offset) {
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

		private:
			Memory* m_free_list;
			uint64_t m_max_size;
			RHI::ResourceHeap m_heap;
		};
	};
}
#endif