#include "VKRenderBackend.h"

namespace RHI::VK {
	VKRenderBackend::VKRenderBackend(bool debug) :
		IRenderBackend(debug),
		m_VKInstance(VK_NULL_HANDLE)
	{
		//SDL_Vulkan_LoadLibrary("libvulkan-1.dll");

		//SDL_Window* query_window = SDL_CreateWindow("", 0, 0, 0, 0, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);

		uint32_t extension_count = 0;
		//SDL_Vulkan_GetInstanceExtensions(query_window, &extension_count, nullptr);
		std::vector<const char*> extensions = std::vector<const char*>(extension_count);
		extensions.push_back("VK_KHR_surface");
		extensions.push_back("VK_KHR_win32_surface");
		//SDL_Vulkan_GetInstanceExtensions(query_window, &extension_count, extensions.data());

		//SDL_DestroyWindow(query_window);

		VkApplicationInfo app_desc = {};
		app_desc.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_desc.pApplicationName = "ATEngine";
		app_desc.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_desc.pEngineName = "No Engine";
		app_desc.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_desc.apiVersion = VK_API_VERSION_1_3;


		VkInstanceCreateInfo instance_desc{};
		instance_desc.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_desc.pApplicationInfo = &app_desc;
		instance_desc.ppEnabledExtensionNames = extensions.data();
		instance_desc.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

		const std::vector<const char*> validation_layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		if (m_Debug) {
			instance_desc.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
			instance_desc.ppEnabledLayerNames = validation_layers.data();
		}

		VkResult result = vkCreateInstance(&instance_desc, VK_NULL_HANDLE, &m_VKInstance);
		if (result != VK_SUCCESS) {
			std::cout << "Create Vulkan Instance Failed." << std::endl;
		}
		else {
			std::cout << "Create Vulkan Instance Success." << std::endl;
		}
	}

	VKRenderBackend::~VKRenderBackend() {
		for (auto pair : m_WindowToVkSurfaceMap) {
			vkDestroySurfaceKHR(m_VKInstance, pair.second, VK_NULL_HANDLE);
		}
		vkDestroyInstance(m_VKInstance, VK_NULL_HANDLE);
	}

	RHI::Result VKRenderBackend::GetAdapters(std::vector<Adapter>& adapters) {
		uint32_t adapter_count = 0u;
		vkEnumeratePhysicalDevices(m_VKInstance, &adapter_count, VK_NULL_HANDLE);
		std::vector<VkPhysicalDevice> native_adapters = std::vector<VkPhysicalDevice>(adapter_count);
		vkEnumeratePhysicalDevices(m_VKInstance, &adapter_count, native_adapters.data());
		for (uint32_t i = 0u; i < native_adapters.size(); ++i) {
			VkPhysicalDeviceProperties adapter_properties = {};
			vkGetPhysicalDeviceProperties(native_adapters[i], &adapter_properties);
			RHI::Vendor vendor = RHI::Vendor::INTEL;
			switch (adapter_properties.vendorID) {
			case 0x10DE:
				vendor = RHI::Vendor::NVIDIA;
				break;
			case  0x8086:
				vendor = RHI::Vendor::INTEL;
				break;
			}
			VkPhysicalDeviceMemoryProperties adapter_memory_properties = {};
			vkGetPhysicalDeviceMemoryProperties(native_adapters[i], &adapter_memory_properties);

			uint64_t vram = 0ull;

			for (uint32_t j = 0u; j < adapter_memory_properties.memoryHeapCount; ++j) {
				if (adapter_memory_properties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
					vram = adapter_memory_properties.memoryHeaps[j].size;
				}
			}
			adapters.push_back(new VKAdapter(vram, vendor, native_adapters[i]));
		}

		return RHI::Result::SUCCESS;
	}

	RHI::Result VKRenderBackend::CreateDevice(const Adapter adapter, Device& device) {

		VkPhysicalDeviceFeatures adapter_features;
		vkGetPhysicalDeviceFeatures(static_cast<const VKAdapter*>(adapter)->GetNative(), &adapter_features);

		float queue_priorities[3] = { 1.0f, 1.0f, 1.0f };

		std::array<uint32_t, 3> indices;
		uint32_t queue_family_count = 0u;
		vkGetPhysicalDeviceQueueFamilyProperties(static_cast<const VKAdapter*>(adapter)->GetNative(), &queue_family_count, VK_NULL_HANDLE);
		std::vector<VkQueueFamilyProperties> properties = std::vector<VkQueueFamilyProperties>(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(static_cast<const VKAdapter*>(adapter)->GetNative(), &queue_family_count, properties.data());
		for (uint32_t i = 0u; i < queue_family_count; ++i) {
			if (properties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT| VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)) {
				OutputDebugString(L"Queue Has Graphics Bit.\n");
			}
		}

		VkDeviceQueueCreateInfo queue_desc = {};
		queue_desc.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_desc.queueFamilyIndex = 0;
		queue_desc.queueCount = 3;
		queue_desc.pQueuePriorities = queue_priorities;



		std::vector<const char*> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
			VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME,
		};

		VkPhysicalDeviceVulkan12Features vk_features = {};
		vk_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vk_features.timelineSemaphore = VK_TRUE;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature = {};
		dynamic_rendering_feature.pNext = &vk_features;
		dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamic_rendering_feature.dynamicRendering = VK_TRUE;

		VkDeviceCreateInfo device_desc = {};
		device_desc.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_desc.pQueueCreateInfos = &queue_desc;
		device_desc.queueCreateInfoCount = 1;
		device_desc.pEnabledFeatures = &adapter_features;
		device_desc.ppEnabledExtensionNames = extensions.data();
		device_desc.enabledExtensionCount = extensions.size();
		device_desc.pNext = &dynamic_rendering_feature;

		VkPhysicalDeviceTimelineSemaphoreProperties prop;
		prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES;
		prop.pNext = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties2 prop2;
		prop2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		prop2.pNext = &prop;
		vkGetPhysicalDeviceProperties2(static_cast<const VKAdapter*>(adapter)->GetNative(), &prop2);
		OutputDebugString(std::to_wstring(prop.maxTimelineSemaphoreValueDifference).c_str());

		VkDevice native_device;

		if (vkCreateDevice(static_cast<const VKAdapter*>(adapter)->GetNative(), &device_desc, VK_NULL_HANDLE, &native_device) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_DEVICE;
		}

		device = new VKDevice(native_device, static_cast<const VKAdapter*>(adapter)->GetNative());

		return RHI::Result::SUCCESS;
	}

	RHI::Result VKRenderBackend::CreateSwapChain(Device device, const SwapChainDescription& description, SwapChain& swap_chain) {
		VkSurfaceKHR surface;

		/*if (!SDL_Vulkan_CreateSurface(static_cast<LinuxWindow*>(description.Window)->GetNative(), m_instance, &surface)) {
			std::cout << "Vulkan Surface Creation Failed." << SDL_GetError() << std::endl;
		}*/
		VkWin32SurfaceCreateInfoKHR surface_desc = {};
		surface_desc.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_desc.hinstance = 0;
		surface_desc.hwnd = (HWND)(description.Window);
		vkCreateWin32SurfaceKHR(m_VKInstance, &surface_desc, VK_NULL_HANDLE, &surface);
		m_WindowToVkSurfaceMap[description.Window] = surface;
		uint32_t adapter_count = 0u;
		vkEnumeratePhysicalDevices(m_VKInstance, &adapter_count, VK_NULL_HANDLE);
		std::vector<VkPhysicalDevice> adapters = std::vector<VkPhysicalDevice>(adapter_count);
		vkEnumeratePhysicalDevices(m_VKInstance, &adapter_count, adapters.data());

		VkSwapchainCreateInfoKHR swap_chain_desc = {};
		swap_chain_desc.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swap_chain_desc.surface = surface;
		swap_chain_desc.minImageCount = description.BufferCount;
		swap_chain_desc.imageFormat = VKConvertFormat(description.BufferFormat);
		swap_chain_desc.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swap_chain_desc.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swap_chain_desc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_chain_desc.queueFamilyIndexCount = 0;
		swap_chain_desc.pQueueFamilyIndices = nullptr;
		swap_chain_desc.imageExtent = VkExtent2D{ description.Width, description.Height };
		swap_chain_desc.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swap_chain_desc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swap_chain_desc.imageArrayLayers = 1;
		swap_chain_desc.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		swap_chain_desc.clipped = VK_TRUE;
		swap_chain_desc.oldSwapchain = VK_NULL_HANDLE;

		VkSwapchainKHR native_swap_chain;
		VKDevice* vkdevice = static_cast<VKDevice*>(device);

		if (vkCreateSwapchainKHR(vkdevice->GetNative(), &swap_chain_desc, VK_NULL_HANDLE, &native_swap_chain) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_SWAP_CHAIN;
		}

		VkSemaphoreCreateInfo semaphore_desc = {};
		semaphore_desc.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		std::vector<VkSemaphore> acquire_image_semaphores = std::vector<VkSemaphore>(description.BufferCount);
		std::vector<VkSemaphore> ready_to_present_semaphores = std::vector<VkSemaphore>(description.BufferCount);
		for (uint32_t i = 0u; i < description.BufferCount; ++i) {
			vkCreateSemaphore(vkdevice->GetNative(), &semaphore_desc, VK_NULL_HANDLE, &acquire_image_semaphores[i]);
			vkCreateSemaphore(vkdevice->GetNative(), &semaphore_desc, VK_NULL_HANDLE, &ready_to_present_semaphores[i]);
		}

		swap_chain = new VKSwapChain(description, *static_cast<VKDevice*>(device), acquire_image_semaphores, ready_to_present_semaphores, native_swap_chain);

		return RHI::Result::SUCCESS;
	}
}