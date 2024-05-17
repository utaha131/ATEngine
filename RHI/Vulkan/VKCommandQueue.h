#ifndef _VK_COMMAND_QUEUE_H_
#define _VK_COMMAND_QUEUE_H_
#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <stdint.h>
#include <string>
#include "VKFence.h"

namespace RHI::VK {
	class VKCommandQueue {
	public:
		VKCommandQueue(VkDevice vk_device, VkSemaphore vk_tracking_semaphore, VkQueue vk_queue);
		~VKCommandQueue();
		void AddWaitSemaphore(VkSemaphore vk_semaphore, uint64_t value);
		void Present(VkPresentInfoKHR& vk_present_info, VkSemaphore vk_present_semaphore);
		void Submit(VkSubmitInfo& vk_submit_info);
		void Signal(VKFence& fence, uint64_t fence_value);
	private:
		VkDevice m_VKDevice;
		VkQueue m_VKQueue;
		std::vector<VkSemaphore> m_WaitSemaphores;
		std::vector<uint64_t> m_WaitValues;
		VkSemaphore m_VKTrackingSemaphore;
		uint64_t m_TrackingSemaphoreValue;
	};
}
#endif