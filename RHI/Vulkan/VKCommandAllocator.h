#ifndef _RHI_VK_COMMAND_ALLOCATOR_H_
#define _RHI_VK_COMMAND_ALLOCATOR_H_
#include "../ICommandAllocator.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKCommandAllocator : public RHI::ICommandAllocator {
	public:
		VKCommandAllocator(CommandType command_type, VkDevice vk_device, VkCommandPool vk_command_pool);
		~VKCommandAllocator() override;
		void Reset() override;
		inline VkCommandPool GetNative() { return m_VKCommandPool; }
	private:
		VkCommandPool m_VKCommandPool;
		VkDevice m_VKDevice;
	};
}
#endif