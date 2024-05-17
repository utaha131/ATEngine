#ifndef _RHI_VK_UNORDERED_ACCESS_VIEW_H_
#define _RHI_VK_UNORDERED_ACCESS_VIEW_H_
#include "../IUnorderedAccessView.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKUnorderedAccessView : public IUnorderedAccessView {
	public:
		VKUnorderedAccessView(const UnorderedAccessViewDescription& description, VkDescriptorBufferInfo& vk_descriptor_buffer_info) :
			IUnorderedAccessView(description),
			m_ResourceType(ResourceType::BUFFER),
			m_VKDescriptorBufferInfo(vk_descriptor_buffer_info),
			m_VKDevice(VK_NULL_HANDLE)
		{

		}
		VKUnorderedAccessView(const UnorderedAccessViewDescription& description, VkDevice vk_device, VkImageView vk_image_view) :
			IUnorderedAccessView(description),
			m_ResourceType(ResourceType::IMAGE),
			m_VKDevice(vk_device),
			m_VKImageView(vk_image_view)
		{
			m_VKDescriptorImageInfo.imageLayout = VKConvertResourceStateToImageLayout(RHI::TextureState::UNORDERED_ACCESS);
			m_VKDescriptorImageInfo.imageView = vk_image_view;
			m_VKDescriptorImageInfo.sampler = VK_NULL_HANDLE;
		}

		VkDescriptorImageInfo GetNativeImageInfo() {
			return m_VKDescriptorImageInfo;
		}

		VkDescriptorBufferInfo GetNativeBufferInfo() {
			return m_VKDescriptorBufferInfo;
		}

		~VKUnorderedAccessView() override {
			if (m_ResourceType == ResourceType::IMAGE) {
				vkDestroyImageView(m_VKDevice, m_VKImageView, VK_NULL_HANDLE);
			}
		}
	private:
		enum ResourceType {
			BUFFER,
			IMAGE
		} m_ResourceType;
		union {
			struct {
				VkImageView m_VKImageView;
				VkDescriptorImageInfo m_VKDescriptorImageInfo;
			};
			VkDescriptorBufferInfo m_VKDescriptorBufferInfo;
		};
		VkDevice m_VKDevice;
	};

	inline VkImageViewCreateInfo VKConvertUnorderedAccessViewDescription(const RHI::UnorderedAccessViewDescription& description, RHI::Texture texture) {
		VkImageViewCreateInfo vk_image_view_create_info = {};
		vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vk_image_view_create_info.format = VKConvertFormat(description.Format);
		vk_image_view_create_info.image = static_cast<VKTexture*>(texture)->GetNative();
		switch (description.ViewDimension) {
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_1D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_1D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture1DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture1DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1DArray.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_2D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_2D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture2DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture2DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2DArray.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			if (description.Texture2DArray.ArraySize == 1) {
				vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			}
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_3D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture3D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		}
		vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		return vk_image_view_create_info;
	}
}
#endif