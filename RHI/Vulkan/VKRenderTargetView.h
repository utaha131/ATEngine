#ifndef _RHI_VK_RENDER_TARGET_VIEW_H_
#define _RHI_VK_RENDER_TARGET_VIEW_H_
#include "../IRenderTargetView.h"
#include <vulkan/vulkan.h>
#include <assert.h>
#include "VKTexture.h"
#include "VKBuffer.h"

namespace RHI::VK {
	class VKRenderTargetView : public RHI::IRenderTargetView {
	public:
		VKRenderTargetView(const RenderTargetViewDescription& description, VKTexture* texture, VkDevice vk_device, VkImageView vk_image_view) :
			RHI::IRenderTargetView(description),
			m_ResourceType(ResourceType::IMAGE),
			m_Texture(texture),
			m_VKImageView(vk_image_view),
			m_VKDevice(vk_device)
		{

		}
		VKRenderTargetView(const RenderTargetViewDescription& description, VKBuffer* buffer, VkDevice vk_device, VkBufferView vk_buffer_view) :
			RHI::IRenderTargetView(description),
			m_ResourceType(ResourceType::BUFFER),
			m_VKBufferView(vk_buffer_view),
			m_VKDevice(vk_device)
		{

		}
		~VKRenderTargetView() override {
			if (m_ResourceType == ResourceType::IMAGE) {
				vkDestroyImageView(m_VKDevice, m_VKImageView, VK_NULL_HANDLE);
			}
			else {
				vkDestroyBufferView(m_VKDevice, m_VKBufferView, VK_NULL_HANDLE);
			}
		}
		inline VkImageView GetNativeImageView() const {
			return m_VKImageView;
		}

		inline VkBufferView GetNativeBufferView() const {
			return m_VKBufferView;
		}

		inline VKTexture* GetTexture() const {
			return m_Texture;
		}
	private:
		enum class ResourceType {
			BUFFER,
			IMAGE
		} m_ResourceType;
		union {
			struct {
				VKTexture* m_Texture;
				VkImageView m_VKImageView;
			};
			struct {
				VkBufferView m_VKBufferView;
			};
		};
		VkDevice m_VKDevice;
	};

	inline VkImageViewCreateInfo VKConvertRenderTargetViewDescription(const RHI::RenderTargetViewDescription& description, RHI::Texture texture) {
		VkImageViewCreateInfo vk_image_view_create_info = {};
		vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vk_image_view_create_info.format = VKConvertFormat(description.Format);
		vk_image_view_create_info.image = static_cast<VKTexture*>(texture)->GetNative();
		switch (description.ViewDimension) {
		case RHI::RenderTargetViewViewDimension::TEXTURE_1D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_1D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture1DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture1DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1DArray.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_2D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2D.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_2D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture2DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture2DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2DArray.MipSlice;
			vk_image_view_create_info.subresourceRange.levelCount = 1;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_3D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
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