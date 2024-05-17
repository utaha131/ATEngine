#ifndef _RHI_VK_RESOURCE_HEAP_H_
#define _RHI_VK_RESOURCE_HEAP_H_
#include "../IResourceHeap.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKResourceHeap : public RHI::IResourceHeap {
	public:

		VKResourceHeap(const ResourceHeapDescription& description, VkDevice vk_device, VkDeviceMemory vk_device_memory);
		~VKResourceHeap() override;
		inline VkDeviceMemory GetNative() const { return m_VKDeviceMemory; }
		void Map(void** pp_mapped_data);
		void Unmap();
	private:
		bool m_Mapped;
		unsigned char* m_MappedData;
		VkDeviceMemory m_VKDeviceMemory;
		VkDevice m_VKDevice;
	};
}
#endif