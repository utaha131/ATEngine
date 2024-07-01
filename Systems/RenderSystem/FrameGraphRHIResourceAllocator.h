#ifndef _AT_RENDER_SYSTEM_FRAME_GRAPH_RESOURCE_ALLOCATOR_H_
#define _AT_RENDER_SYSTEM_FRAME_GRAPH_RESOURCE_ALLOCATOR_H_
#include "../RHI/RHI.h"
#include "GPUMemoryAllocator.h"
#include "./FrameGraphResources.h"
#include "./GPUConstantBuffer.h"
#define _128_MB_ 134217728 
#define _DEFAULT_HEAP_SIZE_ _128_MB_
#define _UPLOAD_HEAP_SIZE_  _128_MB_ 

namespace AT {
	class FrameGraphRHIResourceAllocator {
	public:
		FrameGraphRHIResourceAllocator(RHI::Device device) :
			m_Device(device),
			m_DefaultAllocator(GPUMemoryAllocator::LinearAllocator(RHI_NULL_HANDLE, 0)),
			m_UploadAllocator(GPUMemoryAllocator::LinearAllocator(RHI_NULL_HANDLE, 0))
		{
			RHI::ResourceHeapDescription default_heap_description;
			default_heap_description.ResourceHeapType = RHI::ResourceHeapType::DEFAULT;
			default_heap_description.Size = _DEFAULT_HEAP_SIZE_;
			m_Device->CreateResourceHeap(default_heap_description, m_DefaultHeap);
			m_DefaultAllocator = GPUMemoryAllocator::LinearAllocator(m_DefaultHeap, default_heap_description.Size);

			RHI::ResourceHeapDescription upload_heap_description;
			upload_heap_description.ResourceHeapType = RHI::ResourceHeapType::UPLOAD;
			upload_heap_description.Size = _UPLOAD_HEAP_SIZE_;
			m_Device->CreateResourceHeap(upload_heap_description, m_UploadHeap);
			m_UploadAllocator = GPUMemoryAllocator::LinearAllocator(m_UploadHeap, default_heap_description.Size);
		}

		~FrameGraphRHIResourceAllocator() {
			Clear();
			delete m_DefaultHeap;
			delete m_UploadHeap;
		}

		void Clear() {
			for (auto texture : m_Textures) {
				delete texture;
			}
			m_Textures.clear();

			for (auto buffer : m_Buffers) {
				delete buffer;
			}
			m_Buffers.clear();


			m_DefaultAllocator.Clear();
			m_UploadAllocator.Clear();
		}

		RHI::Texture CreateTexture(const FrameGraphTextureDescription& description, RHI::TextureUsageFlag flags, std::optional<RHI::TextureClearValue>& clear_value) {
			RHI::TextureDescription texture_description;
			texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
			texture_description.Format = description.Format;
			texture_description.Width = description.Width;
			texture_description.Height = description.Height;
			texture_description.DepthOrArray = description.ArraySize;
			texture_description.MipLevels = description.MipLevels;
			texture_description.UsageFlags = flags;

			RHI::AllocationInfo allocation_info = m_Device->GetResourceAllocationInfo(texture_description);
			GPUMemoryAllocator::LinearAllocator::Allocation allocation = m_DefaultAllocator.Allocate(allocation_info.Size, allocation_info.Alignment);
			
			RHI::Texture texture;
			m_Device->CreateTexture(m_DefaultAllocator.GetResourceHeap(), allocation.offset + allocation.padding, clear_value, texture_description, texture);
			m_Textures.emplace_back(texture);
			return texture;
		}

		RHI::Buffer CreateUploadBuffer(uint64_t size) {
			RHI::BufferDescription buffer_description;
			buffer_description.Size = size;
			buffer_description.UsageFlags = RHI::BufferUsageFlag::UNIFORM_BUFFER;

			RHI::AllocationInfo allocation_info = m_Device->GetResourceAllocationInfo(buffer_description);
			GPUMemoryAllocator::LinearAllocator::Allocation allocation = m_UploadAllocator.Allocate(allocation_info.Size, allocation_info.Alignment);
			RHI::Buffer buffer;
			m_Device->CreateBuffer(m_UploadAllocator.GetResourceHeap(), allocation.offset + allocation.padding, RHI::BufferState::COMMON, buffer_description, buffer);
			m_Buffers.emplace_back(buffer);
			return buffer;
		}

		template <typename T> GPUConstantBuffer* CreateConstantBuffer() {
			uint64_t size = (sizeof(T) + 255) & ~255;
			RHI_Buffer buffer = CreateUploadBuffer(size);
			buffer->Map();
			m_constant_buffers.push_back(new GPUConstantBuffer(m_device, buffer));
			return m_constant_buffers.back();
		}

	private:
		RHI::Device m_Device;
		std::list<RHI::Texture> m_Textures;
		std::list<RHI::Buffer> m_Buffers;
		RHI::ResourceHeap m_DefaultHeap, m_UploadHeap;
		GPUMemoryAllocator::LinearAllocator m_DefaultAllocator;
		GPUMemoryAllocator::LinearAllocator m_UploadAllocator;
	};
}
#endif