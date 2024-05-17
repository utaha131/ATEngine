#ifndef _VK_ROOT_SIGNATURE_H_
#define _VK_ROOT_SIGNATURE_H_
#include "../IRootSignature.h"
#include "VKGraphics.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKRootSignature : public IRootSignature {
	public:
		VKRootSignature(const RootSignatureDescription& description, VkDevice vk_device, VkPipelineLayout vk_pipeline_layout) :
			IRootSignature(description),
			m_VKPipelineLayout(vk_pipeline_layout),
			m_VKDevice(vk_device)
		{

		}
		~VKRootSignature() override {
			vkDestroyPipelineLayout(m_VKDevice, m_VKPipelineLayout, VK_NULL_HANDLE);
		}

		inline VkPipelineLayout GetNative() { return m_VKPipelineLayout; }
	private:
		VkPipelineLayout m_VKPipelineLayout;
		VkDevice m_VKDevice;
	};

	constexpr VkDescriptorType VKConvertDescriptorRangeType(RHI::DescriptorRangeType range_type) {
		switch (range_type) {
		case RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case RHI::DescriptorRangeType::SAMPLER_VIEW:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
		case RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		}
	}

	constexpr VkSamplerCreateInfo VKConvertStaticSamplerDescription(RHI::StaticSamplerDescription static_sampler_description) {
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		switch (static_sampler_description.Filter) {
		case RHI::Filter::MIN_MAG_MIP_POINT:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_info.magFilter = VK_FILTER_NEAREST;
			sampler_info.minFilter = VK_FILTER_NEAREST;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_MAG_POINT_MIP_LINEAR:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.magFilter = VK_FILTER_NEAREST;
			sampler_info.minFilter = VK_FILTER_NEAREST;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_POINT_MAG_LINEAR_MIP_POINT:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_NEAREST;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_POINT_MAG_MIP_LINEAR:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_NEAREST;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_LINEAR_MAG_MIP_POINT:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_info.magFilter = VK_FILTER_NEAREST;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.magFilter = VK_FILTER_NEAREST;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_MAG_LINEAR_MIP_POINT:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::MIN_MAG_MIP_LINEAR:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.anisotropyEnable = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			break;
		case  RHI::Filter::ANISOTROPIC:
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_LINEAR;
			sampler_info.anisotropyEnable = VK_TRUE;
			sampler_info.compareEnable = VK_FALSE;
			break;
			/*case  RHI::Filter::COMPARISON_MIN_MAG_MIP_POINT:
			case  RHI::Filter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
			case  RHI::Filter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
			case  RHI::Filter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
			case  RHI::Filter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
			case  RHI::Filter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			case  RHI::Filter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
			case  RHI::Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
			case  RHI::Filter::COMPARISON_ANISOTROPIC:
			case  RHI::Filter::MINIMUM_MIN_MAG_MIP_POINT:
			case  RHI::Filter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
			case  RHI::Filter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			case  RHI::Filter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
			case  RHI::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
			case  RHI::Filter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			case  RHI::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
			case  RHI::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
			case  RHI::Filter::MINIMUM_ANISOTROPIC:
			case  RHI::Filter::MAXIMUM_MIN_MAG_MIP_POINT:
			case  RHI::Filter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
			case  RHI::Filter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			case  RHI::Filter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
			case  RHI::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
			case  RHI::Filter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			case  RHI::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
			case  RHI::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
			case  RHI::Filter::MAXIMUM_ANISOTROPIC:*/
		}
		sampler_info.addressModeU = VKConvertTextureAddressMode(static_sampler_description.AddressU);
		sampler_info.addressModeV = VKConvertTextureAddressMode(static_sampler_description.AddressV);
		sampler_info.addressModeW = VKConvertTextureAddressMode(static_sampler_description.AddressW);
		sampler_info.maxAnisotropy = static_sampler_description.MaxAnisotropy;
		sampler_info.maxLod = RHI_FLOAT32_MAX;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.unnormalizedCoordinates = VK_FALSE;
		sampler_info.compareOp = VKConvertComparisonFunction(static_sampler_description.ComparisonFunction);
		return sampler_info;
	}
}
#endif