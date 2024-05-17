#ifndef _RHI_VK_ADAPTER_H_
#define _RHI_VK_ADAPTER_H_
#include "../IAdapter.h"
#include <vulkan/vulkan.h>

namespace RHI::VK{
	class VKAdapter : public RHI::IAdapter {
	public:
		VKAdapter(uint64_t vram, RHI::Vendor vendor, VkPhysicalDevice vk_physical_device) :
			IAdapter(vram, vendor),
			m_VKPhysicalDevice(vk_physical_device)
		{

		}
		~VKAdapter() override {

		}
		inline VkPhysicalDevice GetNative() const { return m_VKPhysicalDevice; };
	private:
		VkPhysicalDevice m_VKPhysicalDevice;
	};
}
#endif