#ifndef _RHI_VK_DEPTH_STENCIL_VIEW_H_
#define _RHI_VK_DEPTH_STENCIL_VIEW_H_
#include "../IDepthStencilView.h"
#include <vulkan/vulkan.h>
#include "VKTexture.h"

namespace RHI::VK {
	class VKDepthStencilView : public RHI::IDepthStencilView {
	public:
		VKDepthStencilView(const DepthStencilViewDescription& description, VkDevice vk_device, VkImageView vk_image_view, VKTexture* texture) :
			RHI::IDepthStencilView(description),
			m_Texture(texture),
			m_VKImageView(vk_image_view),
			m_VKDevice(vk_device)
		{

		}
		~VKDepthStencilView() override {
			vkDestroyImageView(m_VKDevice, m_VKImageView, VK_NULL_HANDLE);
		}
		VkImageView GetNative() const { return m_VKImageView; }
		inline VKTexture* GetTexture() const {
			return m_Texture;
		}
	private:
		VKTexture* m_Texture;
		VkImageView m_VKImageView;
		VkDevice m_VKDevice;
	};

	inline VkImageViewCreateInfo VKConvertDepthStencilViewDescription(const RHI::DepthStencilViewDescription& description, RHI::Texture texture) {
		VkImageViewCreateInfo vk_image_view_create_info = {};
		vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vk_image_view_create_info.format = VKConvertFormat(description.Format);
		vk_image_view_create_info.image = static_cast<VKTexture*>(texture)->GetNative();
		switch (description.ViewDimension) {
		case RHI::DepthStencilViewViewDimension::TEXTURE_1D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::DepthStencilViewViewDimension::TEXTURE_1D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture1DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture1DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1DArray.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::DepthStencilViewViewDimension::TEXTURE_2D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::DepthStencilViewViewDimension::TEXTURE_2D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture2DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture2DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2DArray.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		}
		vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		return vk_image_view_create_info;
	}
}
#endif