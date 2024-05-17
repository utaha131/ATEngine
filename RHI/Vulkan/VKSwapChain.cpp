#include "VKSwapChain.h"
#include <Windows.h>

namespace RHI::VK {
	VKSwapChain::VKSwapChain(const SwapChainDescription& description, VKDevice& device, std::vector<VkSemaphore>& aquire_image_semaphores, std::vector<VkSemaphore>& ready_to_present_semaphores, VkSwapchainKHR vk_swap_chain) :
		RHI::ISwapChain(description),
		m_VKSwapChain(vk_swap_chain),
		m_Device(device),
		m_AcquireImageSemaphores(aquire_image_semaphores),
		m_ReadyToPresentSemaphores(ready_to_present_semaphores)
	{
		std::vector<VkImage> textures = std::vector<VkImage>(description.BufferCount);
		VkDevice native_device = device.GetNative();
		vkGetSwapchainImagesKHR(native_device, vk_swap_chain, &m_Description.BufferCount, textures.data());
		for (uint32_t i = 0u; i < m_BackBuffers.size(); ++i) {
			TextureDescription buffer_desc;
			buffer_desc.TextureType = RHI::TextureType::TEXTURE_2D;
			buffer_desc.Format = m_Description.BufferFormat;
			buffer_desc.Width = description.Width;
			buffer_desc.Height = description.Height;
			buffer_desc.DepthOrArray = 1u;
			buffer_desc.MipLevels = 1u;
			buffer_desc.UsageFlags = RHI::TextureUsageFlag::RENDER_TARGET;
			m_BackBuffers[i] = new VKTexture(buffer_desc, textures[i]);
		}
	}

	VKSwapChain::~VKSwapChain() {
		for (uint32_t i = 0u; i < m_AcquireImageSemaphores.size(); ++i) {
			vkDestroySemaphore(m_Device.GetNative(), m_AcquireImageSemaphores[i], VK_NULL_HANDLE);
		}
		for (uint32_t i = 0u; i < m_ReadyToPresentSemaphores.size(); ++i) {
			vkDestroySemaphore(m_Device.GetNative(), m_ReadyToPresentSemaphores[i], VK_NULL_HANDLE);
		}
		vkDestroySwapchainKHR(m_Device.GetNative(), m_VKSwapChain, VK_NULL_HANDLE);
	}

	void VKSwapChain::Resize(uint32_t width, uint32_t height) {

	}

	void VKSwapChain::Present() {
		//m_device.GetQueue(RHI_COMMAND_TYPE_GRAPHICS)->AddSignalSemaphore(m_ready_to_present_semaphore[m_current_buffer_index]);
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_VKSwapChain;
		present_info.pImageIndices = &m_CurrentBufferIndex;
		present_info.pResults = VK_NULL_HANDLE;
		//present_info.waitSemaphoreCount = 0;// 1;// 1;
		//present_info.pWaitSemaphores = &m_ready_to_present_semaphore[m_current_buffer_index];
		m_Device.GetQueue(RHI::CommandType::DIRECT)->Present(present_info, m_ReadyToPresentSemaphores[m_CurrentBufferIndex]);
		m_CurrentBufferIndex = (m_CurrentBufferIndex + 1) % m_Description.BufferCount;
		//
	}

	uint32_t VKSwapChain::GetCurrentBackBufferIndex() {
		//OutputDebugString((L"Index: " + std::to_wstring(m_current_buffer_index) + L"\n").c_str());
		VkResult result = vkAcquireNextImageKHR(m_Device.GetNative(), m_VKSwapChain, UINT64_MAX, m_AcquireImageSemaphores[m_CurrentBufferIndex], VK_NULL_HANDLE, &m_CurrentBufferIndex);

		if (result != VK_SUCCESS) {
			OutputDebugString(L"Error.");
		}
		//OutputDebugString((L"After VK Call Index: " + std::to_wstring(m_current_buffer_index) + L"\n").c_str());
		m_Device.GetQueue(RHI::CommandType::DIRECT)->AddWaitSemaphore(m_AcquireImageSemaphores[m_CurrentBufferIndex], 0);
		return m_CurrentBufferIndex;
	}
}