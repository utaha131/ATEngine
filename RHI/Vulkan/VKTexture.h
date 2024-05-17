#ifndef _RHI_VK_TEXTURE_H_
#define _RHI_VK_TEXTURE_H_
#include "../ITexture.h"
#include <vulkan/vulkan.h>
#include "VKResourceHeap.h"
#include "VKGraphics.h"

namespace RHI::VK {
	class VKTexture : public ITexture {
	public:
		VKTexture(const TextureDescription& description, VkDevice vk_device, VKResourceHeap& resource_heap, uint64_t offset, bool is_dedicated, VkImage vk_image) :
			ITexture(description),
			m_VKDevice(vk_device),
			m_ResourceHeap(&resource_heap),
			m_Offset(offset),
			m_IsDedicated(is_dedicated),
			m_VKImage(vk_image)
		{

		}

		VKTexture(const TextureDescription& description, VkImage vk_image) :// Constructor for SwapChain Back Buffers;
			ITexture(description) ,
			m_VKDevice(VK_NULL_HANDLE),
			m_ResourceHeap(RHI_NULL_HANDLE),
			m_Offset(0ull),
			m_IsDedicated(false),
			m_VKImage(vk_image)
		{
		}
		~VKTexture() override {
			vkDestroyImage(m_VKDevice, m_VKImage, VK_NULL_HANDLE);
			if (m_IsDedicated) {
				delete m_ResourceHeap;
				m_ResourceHeap = RHI_NULL_HANDLE;
			}
		}
		inline VkImage GetNative() const { return m_VKImage; }
	private:
		VkImage m_VKImage;
		VkDevice m_VKDevice;
		VKResourceHeap* m_ResourceHeap;
		uint64_t m_Offset;
		bool m_IsDedicated;
	};

	constexpr VkAccessFlagBits VKConvertTextureStateToAccessFlag(RHI::TextureState texture_state) {
		switch (texture_state) {
		case RHI::TextureState::CREATED:
			return (VkAccessFlagBits)0;

		case RHI::TextureState::COMMON:
			return (VkAccessFlagBits)0;

		case RHI::TextureState::UNORDERED_ACCESS:
			return (VkAccessFlagBits)(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

		case RHI::TextureState::COPY_DEST:
			return VK_ACCESS_TRANSFER_WRITE_BIT;

		case RHI::TextureState::COPY_SOURCE:
			return VK_ACCESS_TRANSFER_READ_BIT;
#undef GENERIC_READ
		case RHI::TextureState::GENERIC_READ:
			return VK_ACCESS_MEMORY_READ_BIT;

			//Texture Exclusives.
		case RHI::TextureState::RENDER_TARGET:
			return (VkAccessFlagBits)(VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

		case RHI::TextureState::DEPTH_WRITE:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		case RHI::TextureState::DEPTH_READ:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		case RHI::TextureState::PRESENT:
			return VK_ACCESS_MEMORY_READ_BIT;

		case RHI::TextureState::NON_PIXEL_SHADER_RESOURCE:
			return VK_ACCESS_SHADER_READ_BIT;

		case RHI::TextureState::PIXEL_SHADER_RESOURCE:
			return VK_ACCESS_SHADER_READ_BIT;
		}
	}

	constexpr VkImageLayout VKConvertResourceStateToImageLayout(RHI::TextureState texture_state) {
		switch (texture_state) {
		case RHI::TextureState::CREATED:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case RHI::TextureState::COMMON:
			return VK_IMAGE_LAYOUT_GENERAL;

		case RHI::TextureState::UNORDERED_ACCESS:
			return VK_IMAGE_LAYOUT_GENERAL;

		case RHI::TextureState::COPY_DEST:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		case RHI::TextureState::COPY_SOURCE:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
#undef GENERIC_READ
		case RHI::TextureState::GENERIC_READ:
			return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

			//Texture Exclusives.
		case RHI::TextureState::RENDER_TARGET:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		case RHI::TextureState::DEPTH_WRITE:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		case RHI::TextureState::DEPTH_READ:
			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;

		case RHI::TextureState::PRESENT:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		case RHI::TextureState::NON_PIXEL_SHADER_RESOURCE:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		case RHI::TextureState::PIXEL_SHADER_RESOURCE:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		}
	}

	inline VkImageCreateInfo VKConvertTextureDescription(const RHI::TextureDescription& description) {
		VkImageType vk_image_type;
		uint32_t depth = 0;
		uint32_t array_size = 0;
		switch (description.TextureType) {
		case RHI::TextureType::TEXTURE_1D:
			vk_image_type = VK_IMAGE_TYPE_1D;
			depth = 1;
			array_size = description.DepthOrArray;
			break;
		case RHI::TextureType::TEXTURE_2D:
			vk_image_type = VK_IMAGE_TYPE_2D;
			depth = 1;
			array_size = description.DepthOrArray;
			break;
		case RHI::TextureType::TEXTURE_3D:
			vk_image_type = VK_IMAGE_TYPE_3D;
			depth = description.DepthOrArray;
			array_size = 1;
			break;
		}

		VkImageCreateInfo vk_image_create_info = {};
		vk_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		vk_image_create_info.imageType = vk_image_type;
		vk_image_create_info.format = VKConvertFormat(description.Format);
		vk_image_create_info.extent.width = description.Width;
		vk_image_create_info.extent.height = description.Height;
		vk_image_create_info.extent.depth = depth;
		vk_image_create_info.arrayLayers = array_size;
		vk_image_create_info.mipLevels = description.MipLevels;
		vk_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		vk_image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		vk_image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		vk_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (vk_image_create_info.imageType == VK_IMAGE_TYPE_2D && vk_image_create_info.arrayLayers == 6) {
			vk_image_create_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}
    
		if (RHI_Format_Is_Typeless(description.Format)) {
			vk_image_create_info.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		}
		if (has_flag(description.UsageFlags, RHI::TextureUsageFlag::DEPTH_STENCIL)) {
			vk_image_create_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		if (has_flag(description.UsageFlags, RHI::TextureUsageFlag::RENDER_TARGET)) {
			vk_image_create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (has_flag(description.UsageFlags, RHI::TextureUsageFlag::SHADER_RESOURCE)) {
			vk_image_create_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (has_flag(description.UsageFlags, RHI::TextureUsageFlag::UNORDERED_ACCESS)) {
			vk_image_create_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		return vk_image_create_info;
	}
}
#endif