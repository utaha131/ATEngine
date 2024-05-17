#ifndef _RHI_VK_SWAP_CHAIN_H_
#define _RHI_VK_SWAP_CHAIN_H_
#include "../ISwapChain.h"
#include <vulkan/vulkan.h>
#include "VKTexture.h"
#include <vector>
#include "VKDevice.h"

namespace RHI::VK {
	class VKSwapChain : public ISwapChain {
	public:
		VKSwapChain(const SwapChainDescription& description, VKDevice& device, std::vector<VkSemaphore>& aquire_image_semaphores, std::vector<VkSemaphore>& ready_to_present_semaphores, VkSwapchainKHR vk_swap_chain);
		~VKSwapChain() override;
		void Resize(uint32_t width, uint32_t height) override;
		void Present() override;
		uint32_t GetCurrentBackBufferIndex() override;
	private:
		VkSwapchainKHR m_VKSwapChain;
		std::vector<VkSemaphore> m_AcquireImageSemaphores;
		std::vector<VkSemaphore> m_ReadyToPresentSemaphores;
		VKDevice& m_Device;
	};
}
#endif