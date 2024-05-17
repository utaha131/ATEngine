#include "VKBuffer.h"

namespace RHI::VK {
	VKBuffer::VKBuffer(const BufferDescription& description, VkDevice vk_device, VKResourceHeap& resource_heap, uint64_t offset, bool is_dedicated, VkBuffer vk_buffer) :
		RHI::IBuffer(description),
		m_VKBuffer(vk_buffer),
		m_ResourceHeap(&resource_heap),
		m_Offset(offset),
		m_IsDedicated(is_dedicated),
		m_VKDevice(vk_device)
	{

	}

	VKBuffer::~VKBuffer() {
		vkDestroyBuffer(m_VKDevice, m_VKBuffer, VK_NULL_HANDLE);
		if (m_IsDedicated) {
			delete m_ResourceHeap;
			m_ResourceHeap = RHI_NULL_HANDLE;
		}
	}

	void VKBuffer::Map() {
		if (!m_Mapped) {
			m_ResourceHeap->Map(reinterpret_cast<void**>(&m_MappedData));
			m_Mapped = true;
		}
	}

	void VKBuffer::CopyData(uint64_t offset, const void* data, uint64_t size) {
		if (m_Mapped) {
			memcpy(&m_MappedData[m_Offset + offset], data, size);
		}
	}

	void VKBuffer::Unmap() {
		if (m_Mapped) {
			m_MappedData = nullptr;
			m_Mapped = false;
		}
	}
}