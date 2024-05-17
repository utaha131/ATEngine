#ifndef _AT_LINEAR_ALLOCATOR_H_
#define _AT_LINEAR_ALLOCATOR_H_
#include <stdint.h>

namespace AT {
	template <typename T> class LinearAllocator {
	public:

		struct Memory {
			uint64_t offset;
			T native;
		};

		LinearAllocator(uint64_t increase_size) :
			m_pool_index(0),
			m_offset(0),
			m_increase_size(increase_size)
		{
			
		}

		~LinearAllocator() {
			for (uint32_t i = 0; i < m_memory.size(); ++i) {
				delete m_memory[i];
			}
			m_memory.clear();
		}

		Memory* Allocate(uint64_t size, uint64_t alignment) {
			uint64_t b = alignment - 1;
			uint64_t padding = ((m_offset + b) & ~b) - m_offset;//(std::ceil((float)m_offset / (float)alignment) * alignment) - m_offset;
			size += padding;
			if (m_memory.size() == 0) {
				return nullptr;
			}
			if (m_offset + size > m_increase_size) {//if (m_offset + padding size > m_increase_size) {
				m_pool_index += 1;
				m_offset = 0;
			} 
			if (m_pool_index >= m_memory.size()) {
				return nullptr;
			} else {
				Memory* memory = new Memory{};
				memory->native = m_memory[m_pool_index];
				memory->offset = m_offset + padding;
				m_offset += size;//padding + size;
				return memory;
			}
			/*else {
				memory.native = m_memory[m_pool_index];
				memory.offset = m_offset + padding;
				m_offset += padding + size;
				return 1;
			}*/
		}

		void AddMemory(T memory) {
			m_memory.push_back(memory);
		}

		void Free(Memory& memory) {

		}

		void Clear() {
			m_offset = 0;
			m_pool_index = 0;
		}

		uint64_t IncreaseSize() const { return m_increase_size; }
	private:
		uint64_t m_pool_index;
		uint64_t m_offset;
		uint64_t m_increase_size;
		std::vector<T> m_memory;
	};
}
#endif