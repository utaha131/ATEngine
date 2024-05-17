#ifndef _RHI_VK_RENDER_PLATFORM_H_
#define _RHI_VK_RENDER_PLATFORM_H_
#include "../IRenderBackend.h"
//#include "../../Platform/Linux/LinuxWindow.h"
#include <vulkan/vulkan.h>
#ifdef _WIN32
	#include <Windows.h>
	#include <vulkan/vulkan_win32.h>
#else
#endif
//#include <SDL.h>
//#include <SDL_vulkan.h>
#include "VKAdapter.h"
#include "VKDevice.h"
#include "VKSwapChain.h"
#include <unordered_map>

namespace RHI::VK {
	class VKRenderBackend : public RHI::IRenderBackend {
	public:
		VKRenderBackend(bool debug);
		~VKRenderBackend() override;
		RHI::Result GetAdapters(std::vector<Adapter>& adapters) override;
		RHI::Result CreateDevice(const Adapter adapter, Device& device) override;
		RHI::Result CreateSwapChain(Device device, const SwapChainDescription& description, SwapChain& swap_chain) override;
		/*RHI_RESULT GetSwapChainFormats() override {
			uint32_t count = 0;

			VkSurfaceCapabilitiesKHR capabilities;
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(v_adapters[0], surface, &capabilities);
			OutputDebugString((L"SwapChain Min Image Count: " + std::to_wstring(capabilities.minImageCount) + L"\n").c_str());
			OutputDebugString((L"SwapChain Min Image Count: " + std::to_wstring(capabilities.maxImageCount) + L"\n").c_str());

			vkGetPhysicalDeviceSurfaceFormatsKHR(v_adapters[0], surface, &count, VK_NULL_HANDLE);
			std::vector<VkSurfaceFormatKHR> surface_formats = std::vector<VkSurfaceFormatKHR>(count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(v_adapters[0], surface, &count, surface_formats.data());
			for (unsigned int i = 0; i < count; ++i) {
				std::cout << surface_formats[i].format << std::endl;
				surface_formats[i].colorSpace;
				OutputDebugString((L"\nSurface Format: " + std::to_wstring(surface_formats[i].format) + L", Color Space: " + std::to_wstring(surface_formats[i].colorSpace) + L"\n").c_str());
			}
		}*/
	private:
		VkInstance m_VKInstance;
		std::unordered_map<void*, VkSurfaceKHR> m_WindowToVkSurfaceMap;
	};
}
#endif