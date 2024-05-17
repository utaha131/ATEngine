#ifndef _AT_RENDER_SYSTEM_GPU_RESOURCES_H_
#define _AT_RENDER_SYSTEM_GPU_RESOURCES_H_
#include "../RHI/RHI.h"
#include "GPUMemoryAllocator.h"

namespace AT {
	class GPUBuffer {
		friend class GPUResourceManager;
	public:
		inline RHI::Buffer GetRHIHandle() const {
			return m_RHI;
		}
	private:
		enum MEMORY_TYPE {
			DEVICE,
			HOST
		} m_MemoryType;
		GPUMemoryAllocator::FreeListAllocator::Allocation m_Allocation;
		RHI::Buffer m_RHI = RHI_NULL_HANDLE;
		RHI::BufferState m_State;
	};

	class GPUTexture {
		friend class GPUResourceManager;
	public:
		inline RHI::Texture GetRHIHandle() const {
			return m_RHI;
		}
	private:
		GPUMemoryAllocator::FreeListAllocator::Allocation m_Allocation;
		RHI::Texture m_RHI = RHI_NULL_HANDLE;
		RHI::TextureState m_State = RHI::TextureState::CREATED;
	};

	typedef GPUBuffer* GPUBufferPtr;
	typedef GPUTexture* GPUTexturePtr;
}
#endif