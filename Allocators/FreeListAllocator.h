#ifndef _AT_FREE_LIST_ALLOCATOR_H_
#define _AT_FREE_LIST_ALLOCATOR_H_
#include <list>
#include <stdint.h>
#include <vector>

namespace AT {
	class FreeListAllocator {
	public:
		
		struct Memory {
			void* ptr;
			size_t Size;
			Memory* Next;
		};

		struct Allocation {
			void* Offset;
			uint64_t Padding;
			uint64_t Size;
		};

		FreeListAllocator(void* start, size_t size) {
			
		}

		~FreeListAllocator() {
			
		}

	private:
		Memory* m_free_list;
	};
}
#endif