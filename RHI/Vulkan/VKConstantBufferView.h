#ifndef _RHI_VK_CONSTANT_BUFFER_VIEW_H_
#define _RHI_VK_CONSTANT_BUFFER_VIEW_H_
#include "../IConstantBufferView.h"
#include "VKBuffer.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKConstantBufferView : public RHI::IConstantBufferView {
	public:
		VKConstantBufferView(const ConstantBufferViewDescription& description, VkDescriptorBufferInfo vk_descriptor_buffer_info) :
			RHI::IConstantBufferView(description),
			m_VKDescriptorBufferInfo(vk_descriptor_buffer_info)
		{}
		const VkDescriptorBufferInfo& GetNative() const { return m_VKDescriptorBufferInfo; };
	private:
		VkDescriptorBufferInfo m_VKDescriptorBufferInfo;
	};
}
#endif