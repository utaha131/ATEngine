#include "VKResourceHeap.h"

namespace RHI::VK {
	VKResourceHeap::VKResourceHeap(const ResourceHeapDescription& description, VkDevice vk_device, VkDeviceMemory vk_device_memory) :
		IResourceHeap(description),
		m_VKDeviceMemory(vk_device_memory),
		m_VKDevice(vk_device),
		m_Mapped(false),
		m_MappedData(RHI_NULL_HANDLE)
	{

	}

	VKResourceHeap::~VKResourceHeap() {
		Unmap();
		vkFreeMemory(m_VKDevice, m_VKDeviceMemory, VK_NULL_HANDLE);
	}

	void VKResourceHeap::Map(void** pp_mapped_data) {
		if (!m_Mapped) {
			vkMapMemory(m_VKDevice, m_VKDeviceMemory, 0, m_Description.Size, 0, reinterpret_cast<void**>(&m_MappedData));
			m_Mapped = true;
		}
		*pp_mapped_data = m_MappedData;
	}

	void VKResourceHeap::Unmap() {
		if (m_Mapped) {
			vkUnmapMemory(m_VKDevice, m_VKDeviceMemory);
			m_Mapped = false;
		}
	}
}