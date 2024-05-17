#include "VKCommandAllocator.h"

namespace RHI::VK {
	VKCommandAllocator::VKCommandAllocator(CommandType command_type, VkDevice vk_device, VkCommandPool vk_command_pool) :
		RHI::ICommandAllocator(command_type),
		m_VKCommandPool(vk_command_pool),
		m_VKDevice(vk_device)
	{

	}

	VKCommandAllocator::~VKCommandAllocator() {
		vkDestroyCommandPool(m_VKDevice, m_VKCommandPool, VK_NULL_HANDLE);
	}
	void VKCommandAllocator::Reset() {
		vkResetCommandPool(m_VKDevice, m_VKCommandPool, 0);
	}
}