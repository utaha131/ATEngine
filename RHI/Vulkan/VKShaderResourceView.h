#ifndef _RHI_VK_SHADER_RESOURCE_VIEW_H_
#define _RHI_VK_SHADER_RESOURCE_VIEW_H_
#include "../IShaderResourceView.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKShaderResourceView : public IShaderResourceView {
	public:
		VKShaderResourceView(const ShaderResourceViewDescription& description, VkDevice vk_device, VkImageView vk_image_view) :
			RHI::IShaderResourceView(description),
			m_ResourceType(ResourceType::IMAGE),
			m_VKImageView(vk_image_view),
			m_VKDevice(vk_device)
		{
			m_VKDescriptorImageInfo.imageLayout = VKConvertResourceStateToImageLayout(RHI::TextureState::PIXEL_SHADER_RESOURCE);
			m_VKDescriptorImageInfo.imageView = vk_image_view;
			m_VKDescriptorImageInfo.sampler = VK_NULL_HANDLE;
		}
		VKShaderResourceView(const ShaderResourceViewDescription& description, VkDevice vk_device, VkBufferView vk_buffer_view) :
			RHI::IShaderResourceView(description),
			m_ResourceType(ResourceType::BUFFER),
			m_VKBufferView(m_VKBufferView),
			m_VKDevice(vk_device)
		{

		}
		~VKShaderResourceView() override {
			if (m_ResourceType == ResourceType::IMAGE) {
				vkDestroyImageView(m_VKDevice, m_VKImageView, VK_NULL_HANDLE);
			}
			else {
				vkDestroyBufferView(m_VKDevice, m_VKBufferView, VK_NULL_HANDLE);
			}
		}

		VkDescriptorImageInfo GetNativeImageInfo() const { return m_VKDescriptorImageInfo; }
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
			VkBufferView m_VKBufferView;
		};
		VkDevice m_VKDevice;
	};

	inline VkImageViewCreateInfo VKConvertShaderResourceViewDescription(const RHI::ShaderResourceViewDescription& description, RHI::Texture texture) {
		VkImageViewCreateInfo vk_image_view_create_info = {};
		vk_image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		vk_image_view_create_info.format = VKConvertFormat(description.Format);
		vk_image_view_create_info.image = static_cast<VKTexture*>(texture)->GetNative();
		switch (description.ViewDimension) {
		case RHI::ShaderResourceViewViewDimension::TEXTURE_1D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1D.MostDetailedMip;
			vk_image_view_create_info.subresourceRange.levelCount = description.Texture1D.MipLevels;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_1D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture1DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture1DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture1DArray.MostDetailedMip;
			vk_image_view_create_info.subresourceRange.levelCount = description.Texture1DArray.MipLevels;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_2D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2D.MostDetailedMip;
			vk_image_view_create_info.subresourceRange.levelCount = description.Texture2D.MipLevels;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_2D_ARRAY:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = description.Texture2DArray.FirstArraySlice;
			vk_image_view_create_info.subresourceRange.layerCount = description.Texture2DArray.ArraySize;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture2DArray.MostDetailedMip;
			vk_image_view_create_info.subresourceRange.levelCount = description.Texture2DArray.MipLevels;
			if (description.Texture2DArray.ArraySize == 1) {
				vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			}
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_3D:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = 1;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.Texture3D.MostDetailedMip;
			vk_image_view_create_info.subresourceRange.levelCount = description.Texture3D.MipLevels;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_CUBE:
			vk_image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			vk_image_view_create_info.subresourceRange.baseArrayLayer = 0;
			vk_image_view_create_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
			vk_image_view_create_info.subresourceRange.baseMipLevel = description.TextureCube.MostDetailedMip;
			vk_image_view_create_info.subresourceRange.levelCount = description.TextureCube.MipLevels;
			break;
		}
		vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		if (RHI_Format_Is_Depth(description.Format)) {
			vk_image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		vk_image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		vk_image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		return vk_image_view_create_info;
	}
}
#endif