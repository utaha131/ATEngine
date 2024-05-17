#include "VKCommandQueue.h"
#include <assert.h>
#include <Windows.h>

namespace RHI::VK {
	VKCommandQueue::VKCommandQueue(VkDevice vk_device, VkSemaphore vk_tracking_semaphore, VkQueue vk_queue) :
		m_VKQueue(vk_queue),
		m_WaitSemaphores(std::vector<VkSemaphore>()),
		m_WaitValues(std::vector<uint64_t>()),
		m_VKTrackingSemaphore(vk_tracking_semaphore),
		m_TrackingSemaphoreValue(0ull),
		m_VKDevice(vk_device)
	{

	}

	VKCommandQueue::~VKCommandQueue() {
		for (uint32_t i = 0u; i < m_WaitSemaphores.size(); ++i) {
			vkDestroySemaphore(m_VKDevice, m_WaitSemaphores[i], VK_NULL_HANDLE);
		}
		vkDestroySemaphore(m_VKDevice, m_VKTrackingSemaphore, VK_NULL_HANDLE);
	}

	void VKCommandQueue::AddWaitSemaphore(VkSemaphore semaphore, uint64_t value) {
		m_WaitSemaphores.push_back(semaphore);
		m_WaitValues.push_back(value);
	}

	void VKCommandQueue::Present(VkPresentInfoKHR& present_info, VkSemaphore present_semaphore) {
		VkTimelineSemaphoreSubmitInfo timeline_submit_info = {};
		timeline_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timeline_submit_info.waitSemaphoreValueCount = 1;
		uint64_t wait_value = m_TrackingSemaphoreValue;
		timeline_submit_info.pWaitSemaphoreValues = &wait_value;
		++m_TrackingSemaphoreValue;
		timeline_submit_info.signalSemaphoreValueCount = 2;
		uint64_t signal_values[2] = { m_TrackingSemaphoreValue, 0ull };
		timeline_submit_info.pSignalSemaphoreValues = signal_values;

		VkSubmitInfo buffer_submit = {};
		buffer_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		buffer_submit.pNext = &timeline_submit_info;

		buffer_submit.waitSemaphoreCount = 1;
		buffer_submit.pWaitSemaphores = &m_VKTrackingSemaphore;
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
		buffer_submit.pWaitDstStageMask = waitStages;

		buffer_submit.signalSemaphoreCount = 2;
		VkSemaphore signal_semaphores[2] = { m_VKTrackingSemaphore, present_semaphore };
		buffer_submit.pSignalSemaphores = signal_semaphores;
		buffer_submit.commandBufferCount = 0;
		vkQueueSubmit(m_VKQueue, 1, &buffer_submit, VK_NULL_HANDLE);

		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &present_semaphore;
		
		vkQueuePresentKHR(m_VKQueue, &present_info);
	}

	void VKCommandQueue::Submit(VkSubmitInfo& submit_info) {
		std::vector<VkSemaphore> wait_semaphores = std::vector <VkSemaphore>();
		std::vector<uint64_t> wait_values = std::vector<uint64_t>();
		wait_semaphores.push_back(m_VKTrackingSemaphore);
		uint64_t temp = m_TrackingSemaphoreValue;
		wait_values.push_back(temp);

		for (uint32_t i = 0u; i < m_WaitSemaphores.size(); ++i) {
			wait_semaphores.push_back(m_WaitSemaphores.at(i));
			wait_values.push_back(m_WaitValues.at(i));
		}

		VkTimelineSemaphoreSubmitInfo timeline_submit_info = {};
		timeline_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timeline_submit_info.waitSemaphoreValueCount = wait_values.size();
		timeline_submit_info.pWaitSemaphoreValues = wait_values.data();
		++m_TrackingSemaphoreValue;
		timeline_submit_info.signalSemaphoreValueCount = 1;
		timeline_submit_info.pSignalSemaphoreValues = &m_TrackingSemaphoreValue;

		submit_info.pNext = &timeline_submit_info;
		submit_info.waitSemaphoreCount = wait_semaphores.size();
		submit_info.pWaitSemaphores = wait_semaphores.data();
		std::vector<VkPipelineStageFlags> wait_stages = std::vector<VkPipelineStageFlags>(wait_semaphores.size(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		submit_info.pWaitDstStageMask = wait_stages.data();
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &m_VKTrackingSemaphore;
		vkQueueSubmit(m_VKQueue, 1, &submit_info, VK_NULL_HANDLE);
		m_WaitSemaphores.clear();
		m_WaitValues.clear();
	}

	void VKCommandQueue::Signal(VKFence& fence, uint64_t fence_value) {
		VkTimelineSemaphoreSubmitInfo timeline_submit_info = {};
		timeline_submit_info.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timeline_submit_info.waitSemaphoreValueCount = 1;
		uint64_t wait_value = m_TrackingSemaphoreValue;
		timeline_submit_info.pWaitSemaphoreValues = &wait_value;

		++m_TrackingSemaphoreValue;

		uint64_t value;
		timeline_submit_info.signalSemaphoreValueCount = 2;
		uint64_t signal_values[2] = { m_TrackingSemaphoreValue, fence_value };

		timeline_submit_info.pSignalSemaphoreValues = signal_values;

		VkSubmitInfo buffer_submit = {};
		buffer_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		buffer_submit.pNext = &timeline_submit_info;

		buffer_submit.waitSemaphoreCount = 1;
		buffer_submit.pWaitSemaphores = &m_VKTrackingSemaphore;
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
		buffer_submit.pWaitDstStageMask = waitStages;

		buffer_submit.signalSemaphoreCount = 2;
		VkSemaphore signal_semaphores[2] = { m_VKTrackingSemaphore, fence.GetNative() };
		/*buffer_submit.signalSemaphoreCount = 1;	
		VkSemaphore signal_semaphores[1] = { fence.GetNative() };*/

		buffer_submit.pSignalSemaphores = signal_semaphores;
		buffer_submit.commandBufferCount = 0;
		vkQueueSubmit(m_VKQueue, 1, &buffer_submit, VK_NULL_HANDLE);
	}
}