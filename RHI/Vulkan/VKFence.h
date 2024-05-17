#ifndef _VK_FENCE_H_
#define _VK_FENCE_H_
#include "../IFence.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKFence : public RHI::IFence {
	public:
		VKFence(VkDevice vk_device, VkSemaphore vk_sempahore) :
			m_VKDevice(vk_device),
			m_VKSemaphore(vk_sempahore)
		{}

		~VKFence() override {
			vkDestroySemaphore(m_VKDevice, m_VKSemaphore, VK_NULL_HANDLE);
		}

		void Signal(uint64_t value) const override {
			VkSemaphoreSignalInfo vk_semaphore_signal_info;
			vk_semaphore_signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
			vk_semaphore_signal_info.pNext = VK_NULL_HANDLE;
			vk_semaphore_signal_info.semaphore = m_VKSemaphore;
			vk_semaphore_signal_info.value = value;
			vkSignalSemaphore(m_VKDevice, &vk_semaphore_signal_info);
		}

		uint64_t GetCompletedValue() const override {
			uint64_t value;
			vkGetSemaphoreCounterValue(m_VKDevice, m_VKSemaphore, &value);
			return value;
		}

		VkSemaphore GetNative() const { return m_VKSemaphore; }

	private:
		VkDevice m_VKDevice;
		VkSemaphore m_VKSemaphore;
	};
}
#endif