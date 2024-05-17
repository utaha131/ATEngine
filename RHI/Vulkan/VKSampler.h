#ifndef _VK_SAMPLER_H_
#define _VK_SAMPLER_H_
#include "../ISampler.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKSampler : public RHI::ISampler {
	public:
		VKSampler(const SamplerDescription& description, VkDevice vk_device, VkSampler vk_sampler, VkDescriptorImageInfo& vk_descriptor_image_info) :
			ISampler(description),
			m_VKDevice(vk_device),
			m_VKSampler(vk_sampler),
			m_VKDescriptorImageInfo(vk_descriptor_image_info)
		{

		}
		~VKSampler() override {
			vkDestroySampler(m_VKDevice, m_VKSampler, VK_NULL_HANDLE);
		}
		VkDescriptorImageInfo GetNative() const { return m_VKDescriptorImageInfo; };
	private:
		VkDevice m_VKDevice;
		VkSampler m_VKSampler;
		VkDescriptorImageInfo m_VKDescriptorImageInfo;
	};
}
#endif