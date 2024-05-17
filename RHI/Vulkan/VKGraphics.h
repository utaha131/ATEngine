#ifndef _RHI_VK_FORMAT_H_
#define _RHI_VK_FORMAT_H_
#include <vulkan/vulkan.h>
#include "../RHICore.h"

namespace RHI::VK {
	constexpr VkFormat VKConvertFormat(RHI::Format format) {
		switch (format) {
		case RHI::Format::UNKNOWN:
			return VK_FORMAT_UNDEFINED;
		case RHI::Format::R32G32B32A32_TYPELESS:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case RHI::Format::R32G32B32A32_FLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case RHI::Format::R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case RHI::Format::R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		case RHI::Format::R32G32B32_TYPELESS:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case RHI::Format::R32G32B32_FLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case RHI::Format::R32G32B32_UINT:
			return VK_FORMAT_R32G32B32_UINT;
		case RHI::Format::R32G32B32_SINT:
			return VK_FORMAT_R32G32B32_SINT;
		case RHI::Format::R16G16B16A16_TYPELESS:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case RHI::Format::R16G16B16A16_FLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case RHI::Format::R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case RHI::Format::R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case RHI::Format::R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case RHI::Format::R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
		case RHI::Format::R32G32_TYPELESS:
			return VK_FORMAT_R32G32_SFLOAT;
		case RHI::Format::R32G32_FLOAT:
			return VK_FORMAT_R32G32_SFLOAT;
		case RHI::Format::R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case RHI::Format::R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case RHI::Format::R32G8X24_TYPELESS:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case RHI::Format::D32_FLOAT_S8X24_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
			/*case RHI::Format::R32_FLOAT_X8X24_TYPELESS:
				return VK_FORMAT_D32_SFLOAT_S8_UINT;*/
				/*case RHI::Format::X32_TYPELESS_G8X24_UINT:
					return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;*/
		case RHI::Format::R10G10B10A2_TYPELESS:
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case RHI::Format::R10G10B10A2_UNORM:
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		case RHI::Format::R10G10B10A2_UINT:
			return VK_FORMAT_A2R10G10B10_UINT_PACK32;
		case RHI::Format::R11G11B10_FLOAT:
			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case RHI::Format::R8G8B8A8_TYPELESS:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case RHI::Format::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case RHI::Format::R8G8B8A8_UNORM_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case RHI::Format::R8G8B8A8_UINT:
			return VK_FORMAT_R8G8B8A8_UINT;
		case RHI::Format::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case RHI::Format::R8G8B8A8_SINT:
			return VK_FORMAT_R8G8B8A8_SINT;
		case RHI::Format::R16G16_TYPELESS:
			return VK_FORMAT_R16G16_SFLOAT;
		case RHI::Format::R16G16_FLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
		case RHI::Format::R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case RHI::Format::R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case RHI::Format::R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case RHI::Format::R16G16_SINT:
			return  VK_FORMAT_R16G16_SINT;
		case RHI::Format::R32_TYPELESS:
			return VK_FORMAT_R32_SFLOAT;
		case RHI::Format::D32_FLOAT:
			return VK_FORMAT_D32_SFLOAT;
		case RHI::Format::R32_FLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case RHI::Format::R32_UINT:
			return VK_FORMAT_R32_UINT;
		case RHI::Format::R32_SINT:
			return VK_FORMAT_R32_SINT;
		case RHI::Format::R24G8_TYPELESS:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case RHI::Format::D24_UNORM_S8_UINT:
			return VK_FORMAT_D24_UNORM_S8_UINT;
			/*case RHI::Format::R24_UNORM_X8_TYPELESS:
				return VK_FORMAT_X8_D24_UNORM_PACK32;*/
				/*case RHI::Format::X24_TYPELESS_G8_UINT:
					return DXGI_FORMAT_X24_TYPELESS_G8_UINT;*/
		case RHI::Format::R8G8_TYPELESS:
			return VK_FORMAT_R8G8_UNORM;
		case RHI::Format::R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case RHI::Format::R8G8_UINT:
			return VK_FORMAT_R8G8_UINT;
		case RHI::Format::R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case RHI::Format::R8G8_SINT:
			return VK_FORMAT_R8G8_SINT;
		case RHI::Format::R16_TYPELESS:
			return VK_FORMAT_R16_UNORM;
		case RHI::Format::R16_FLOAT:
			return VK_FORMAT_R16_SFLOAT;
		case RHI::Format::D16_UNORM:
			return VK_FORMAT_D16_UNORM;
		case RHI::Format::R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case RHI::Format::R16_UINT:
			return VK_FORMAT_R16_UINT;
		case RHI::Format::R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case RHI::Format::R16_SINT:
			return VK_FORMAT_R16_SINT;
		case RHI::Format::R8_TYPELESS:
			return VK_FORMAT_R8_UNORM;
		case RHI::Format::R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case RHI::Format::R8_UINT:
			return VK_FORMAT_R8_UINT;
		case RHI::Format::R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case RHI::Format::R8_SINT:
			return VK_FORMAT_R8_SINT;
		case RHI::Format::A8_UNORM:
			return VK_FORMAT_A8_UNORM_KHR;
			/*case RHI::Format::R1_UNORM:
				return DXGI_FORMAT_R1_UNORM;
			case RHI::Format::R9G9B9E5_SHAREDEXP:
				return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
			case RHI::Format::R8G8_B8G8_UNORM:
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			case RHI::Format::G8R8_G8B8_UNORM:
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
			case RHI::Format::BC1_TYPELESS:
				return DXGI_FORMAT_BC1_TYPELESS;
			case RHI::Format::BC1_UNORM:
				return DXGI_FORMAT_BC1_UNORM;
			case RHI::Format::BC1_UNORM_SRGB:
				return DXGI_FORMAT_BC1_UNORM_SRGB;
			case RHI::Format::BC2_TYPELESS:
				return DXGI_FORMAT_BC2_TYPELESS;
			case RHI::Format::BC2_UNORM:
				return DXGI_FORMAT_BC2_UNORM;
			case RHI::Format::BC2_UNORM_SRGB:
				return DXGI_FORMAT_BC2_UNORM_SRGB;
			case RHI::Format::BC3_TYPELESS:
				return DXGI_FORMAT_BC3_TYPELESS;
			case RHI::Format::BC3_UNORM:
				return DXGI_FORMAT_BC3_UNORM;
			case RHI::Format::BC3_UNORM_SRGB:
				return DXGI_FORMAT_BC3_UNORM_SRGB;
			case RHI::Format::BC4_TYPELESS:
				return DXGI_FORMAT_BC4_TYPELESS;
			case RHI::Format::BC4_UNORM:
				return DXGI_FORMAT_BC4_UNORM;
			case RHI::Format::BC4_SNORM:
				return DXGI_FORMAT_BC4_SNORM;
			case RHI::Format::BC5_TYPELESS:
				return DXGI_FORMAT_BC5_TYPELESS;
			case RHI::Format::BC5_UNORM:
				return DXGI_FORMAT_BC5_UNORM;
			case RHI::Format::BC5_SNORM:
				return DXGI_FORMAT_BC5_SNORM;
			case RHI::Format::B5G6R5_UNORM:
				return DXGI_FORMAT_B5G6R5_UNORM;
			case RHI::Format::B5G5R5A1_UNORM:
				return DXGI_FORMAT_B5G5R5A1_UNORM;*/
		case RHI::Format::B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
			/*case RHI::Format::B8G8R8X8_UNORM:
				return DXGI_FORMAT_B8G8R8X8_UNORM;
			case RHI::Format::R10G10B10_XR_BIAS_A2_UNORM:
				return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
			case RHI::Format::B8G8R8A8_TYPELESS:
				return DXGI_FORMAT_B8G8R8A8_TYPELESS;
			case RHI::Format::B8G8R8A8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
			case RHI::Format::B8G8R8X8_TYPELESS:
				return DXGI_FORMAT_B8G8R8X8_TYPELESS;
			case RHI::Format::B8G8R8X8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
			case RHI::Format::BC6H_TYPELESS:
				return DXGI_FORMAT_BC6H_TYPELESS;
			case RHI::Format::BC6H_UF16:
				return DXGI_FORMAT_BC6H_UF16;
			case RHI::Format::BC6H_SF16:
				return DXGI_FORMAT_BC6H_SF16;
			case RHI::Format::BC7_TYPELESS:
				return DXGI_FORMAT_BC7_TYPELESS;
			case RHI::Format::BC7_UNORM:
				return DXGI_FORMAT_BC7_UNORM;
			case RHI::Format::BC7_UNORM_SRGB:
				return DXGI_FORMAT_BC7_UNORM_SRGB;*/
		}
	}

	constexpr VkRect2D VKConvertRect(const RHI::Rect& rect) {
		VkRect2D vk_rect;/* = {
			.offset = {
				.x = rect.Left,
				.y = rect.Top,
			},
			.extent = {
				.width = static_cast<uint32_t>(rect.Right),
				.height = static_cast<uint32_t>(rect.Bottom),
			},
		};*/
		vk_rect.offset.x = rect.Left;
		vk_rect.offset.y = rect.Top;
		vk_rect.extent.width = static_cast<uint32_t>(rect.Right);
		vk_rect.extent.height = static_cast<uint32_t>(rect.Bottom);
		return vk_rect;
	}

	constexpr VkViewport VKConvertViewport(const RHI::Viewport& viewport) {
		/*VkViewport native_viewport = {
			.x = viewport.Top_Left_X,
			.y = viewport.Top_Left_Y,
			.width = viewport.Width,
			.height = viewport.Height,
			.minDepth = viewport.Min_Depth,
			.maxDepth = viewport.Max_Depth,
		};*/
		VkViewport vk_viewport;
		vk_viewport.x = viewport.TopLeftX;
		vk_viewport.y = viewport.TopLeftY;
		vk_viewport.width = viewport.Width;
		vk_viewport.height = viewport.Height;
		vk_viewport.minDepth = viewport.MinDepth;
		vk_viewport.maxDepth = viewport.MaxDepth;
		return vk_viewport;
	}

	constexpr VkBool32 VKConvertDepthWriteMask(RHI::DepthWriteMask depth_write_mask) {
		return depth_write_mask == RHI::DepthWriteMask::ALL ? VK_TRUE : VK_FALSE;
	}

	inline constexpr VkCompareOp VKConvertComparisonFunction(RHI::ComparisonFunction comparison_function) {
		switch (comparison_function) {
		case RHI::ComparisonFunction::NEVER:
			return VK_COMPARE_OP_NEVER;
		case RHI::ComparisonFunction::LESS:
			return VK_COMPARE_OP_LESS;
		case RHI::ComparisonFunction::EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case RHI::ComparisonFunction::LESS_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case RHI::ComparisonFunction::GREATER:
			return VK_COMPARE_OP_GREATER;
		case RHI::ComparisonFunction::NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		case RHI::ComparisonFunction::GREATER_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case RHI::ComparisonFunction::ALWAYS:
			return VK_COMPARE_OP_ALWAYS;
		}
	}

	constexpr VkStencilOp VKConvertStencilOperation(RHI::StencilOperation stencil_operation) {
		switch (stencil_operation) {
		case RHI::StencilOperation::KEEP:
			return VK_STENCIL_OP_KEEP;
		case RHI::StencilOperation::ZERO:
			return VK_STENCIL_OP_ZERO;
		case RHI::StencilOperation::REPLACE:
			return VK_STENCIL_OP_REPLACE;
		case RHI::StencilOperation::INCREASE_SATURATE:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case RHI::StencilOperation::DECREASE_SATURATE:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case RHI::StencilOperation::INVERT:
			return VK_STENCIL_OP_INVERT;
		case RHI::StencilOperation::INCREASE:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case RHI::StencilOperation::DECREASE:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		}
	}

	constexpr VkPolygonMode VKConvertFillMode(RHI::FillMode fill_mode) {
		switch (fill_mode) {
		case RHI::FillMode::WIREFRAME:
			return VK_POLYGON_MODE_LINE;
		case RHI::FillMode::SOLID:
			return VK_POLYGON_MODE_FILL;
		}
	}

	constexpr VkCullModeFlagBits VKConvertCullMode(RHI::CullMode cull_mode) {
		switch (cull_mode) {
		case RHI::CullMode::NONE:
			return VK_CULL_MODE_NONE;
		case RHI::CullMode::FRONT:
			return VK_CULL_MODE_FRONT_BIT;
		case RHI::CullMode::BACK:
			return VK_CULL_MODE_BACK_BIT;
		}
	}

	constexpr VkBlendFactor VKConvertBlend(RHI::Blend blend) {
		switch (blend) {
		case RHI::Blend::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case RHI::Blend::ONE:
			return VK_BLEND_FACTOR_ONE;
		case RHI::Blend::SOURCE_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case RHI::Blend::INVERSE_SOURCE_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case RHI::Blend::SOURCE_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case RHI::Blend::INVERSE_SOURCE_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case RHI::Blend::DESTINATION_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case RHI::Blend::INVERSE_DESTINATION_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case RHI::Blend::DESTINATION_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case RHI::Blend::INVERSE_DESTINATION_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case RHI::Blend::SOURCE_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case RHI::Blend::BLEND_FACTOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case RHI::Blend::INVERSE_BLEND_FACTOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case RHI::Blend::SOURCE1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case RHI::Blend::INVERSE_SOURCE1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case RHI::Blend::SOURCE1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case RHI::Blend::INVERSE_SOURCE1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		}
	}

	constexpr VkBlendOp VKConvertBlendOperation(RHI::BlendOperation blend_operation) {
		switch (blend_operation) {
		case RHI::BlendOperation::ADD:
			return VK_BLEND_OP_ADD;
		case RHI::BlendOperation::SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case RHI::BlendOperation::REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case RHI::BlendOperation::MIN:
			return VK_BLEND_OP_MIN;
		case RHI::BlendOperation::MAX:
			return VK_BLEND_OP_MAX;
		}
	}

	constexpr VkLogicOp VKConvertLogicOperation(RHI::LogicOperation logic_operation) {
		switch (logic_operation) {
		case RHI::LogicOperation::CLEAR:
			return VK_LOGIC_OP_CLEAR;
		case RHI::LogicOperation::SET:
			return VK_LOGIC_OP_SET;
		case RHI::LogicOperation::COPY:
			return VK_LOGIC_OP_COPY;
		case RHI::LogicOperation::COPY_INVERTED:
			return VK_LOGIC_OP_COPY_INVERTED;
		case RHI::LogicOperation::NOOP:
			return VK_LOGIC_OP_NO_OP;
		case RHI::LogicOperation::INVERT:
			return VK_LOGIC_OP_INVERT;
		case RHI::LogicOperation::AND:
			return VK_LOGIC_OP_AND;
		case RHI::LogicOperation::NAND:
			return VK_LOGIC_OP_NAND;
		case RHI::LogicOperation::OR:
			return VK_LOGIC_OP_OR;
		case RHI::LogicOperation::NOR:
			return VK_LOGIC_OP_NOR;
		case RHI::LogicOperation::XOR:
			return VK_LOGIC_OP_XOR;
		case RHI::LogicOperation::EQUIVALENT:
			return VK_LOGIC_OP_EQUIVALENT;
		case RHI::LogicOperation::AND_REVERSE:
			return VK_LOGIC_OP_AND_REVERSE;
		case RHI::LogicOperation::AND_INVERTED:
			return VK_LOGIC_OP_AND_INVERTED;
		case RHI::LogicOperation::OR_REVERSE:
			return VK_LOGIC_OP_OR_REVERSE;
		case RHI::LogicOperation::OR_INVERTED:
			return VK_LOGIC_OP_OR_INVERTED;
		}
	}

	constexpr VkColorComponentFlagBits VKConvertColorWriteMask(RHI::ColorWriteEnable color_write_enable) {
		VkColorComponentFlagBits flag = {};
		if (color_write_enable == RHI::ColorWriteEnable::ALL) {
			flag = static_cast<VkColorComponentFlagBits>(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
			return flag;
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::RED)) {
			flag = static_cast<VkColorComponentFlagBits>(flag | VK_COLOR_COMPONENT_R_BIT);
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::GREEN)) {
			flag = static_cast<VkColorComponentFlagBits>(flag | VK_COLOR_COMPONENT_G_BIT);
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::BLUE)) {
			flag = static_cast<VkColorComponentFlagBits>(flag | VK_COLOR_COMPONENT_B_BIT);
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::ALPHA)) {
			flag = static_cast<VkColorComponentFlagBits>(flag | VK_COLOR_COMPONENT_A_BIT);
		}
		return flag;
	}

	constexpr VkPrimitiveTopology VKConvertPrimitiveToplogy(RHI::PrimitiveTopology primitive_topology) {
		switch (primitive_topology) {
		case RHI::PrimitiveTopology::POINT_LIST:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case RHI::PrimitiveTopology::LINE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case RHI::PrimitiveTopology::LINE_STRIP:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case RHI::PrimitiveTopology::TRIANGLE_LIST:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case RHI::PrimitiveTopology::TRIANGLE_STRIP:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		}
	}

	constexpr VkIndexType VKConvertIndexBufferFormat(RHI::Format format) {
		switch (format) {
		case RHI::Format::R16_UINT:
			return VK_INDEX_TYPE_UINT16;
		case RHI::Format::R32_UINT:
			VK_INDEX_TYPE_UINT32;
		default:
			return VK_INDEX_TYPE_NONE_KHR;
		}
	}

	constexpr VkSamplerCreateInfo VKConvertFilter(RHI::Filter filter) {
		VkSamplerCreateInfo sampler_create_info = {};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		/*sampler_reduction_mode_create_info.reductionMode = VK_SAMPLER_REDUCTION_MODE_MIN;
		sampler_create_info.pNext = &sampler_reduction_mode_create_info;*/

		switch (filter) {
		case RHI::Filter::MIN_MAG_MIP_POINT:
		case RHI::Filter::MINIMUM_MIN_MAG_MIP_POINT:
		case RHI::Filter::MAXIMUM_MIN_MAG_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_MAG_POINT_MIP_LINEAR:
		case RHI::Filter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
		case RHI::Filter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_POINT_MAG_LINEAR_MIP_POINT:
		case RHI::Filter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
		case RHI::Filter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_POINT_MAG_MIP_LINEAR:
		case RHI::Filter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
		case RHI::Filter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_LINEAR_MAG_MIP_POINT:
		case RHI::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
		case RHI::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		case RHI::Filter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
		case RHI::Filter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_MAG_LINEAR_MIP_POINT:
		case RHI::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
		case RHI::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::MIN_MAG_MIP_LINEAR:
		case RHI::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
		case RHI::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::ANISOTROPIC:
		case RHI::Filter::MINIMUM_ANISOTROPIC:
		case RHI::Filter::MAXIMUM_ANISOTROPIC:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_TRUE;
			sampler_create_info.compareEnable = VK_FALSE;
		case RHI::Filter::COMPARISON_MIN_MAG_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_NEAREST;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_NEAREST;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_FALSE;
			sampler_create_info.compareEnable = VK_TRUE;
		case RHI::Filter::COMPARISON_ANISOTROPIC:
			sampler_create_info.minFilter = VK_FILTER_LINEAR;
			sampler_create_info.magFilter = VK_FILTER_LINEAR;
			sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_create_info.anisotropyEnable = VK_TRUE;
			sampler_create_info.compareEnable = VK_TRUE;
		}
		return sampler_create_info;
	}

	constexpr VkSamplerAddressMode VKConvertTextureAddressMode(RHI::TextureAddressMode texture_address_mode) {
		switch (texture_address_mode) {
		case RHI::TextureAddressMode::WRAP:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case RHI::TextureAddressMode::MIRROR:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case RHI::TextureAddressMode::CLAMP:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case RHI::TextureAddressMode::BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case RHI::TextureAddressMode::MIRROR_ONCE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}
	}

	constexpr VkClearValue VKConvertClearValue(const RHI::TextureClearValue& clear_value) {
		VkClearValue return_value = {};
		return_value.color.float32[0] = clear_value.Color[0];
		return_value.color.float32[1] = clear_value.Color[1];
		return_value.color.float32[2] = clear_value.Color[2];
		return_value.color.float32[3] = clear_value.Color[3];
		return_value.depthStencil.depth = clear_value.DepthAndStencil.Depth;
		return_value.depthStencil.stencil = clear_value.DepthAndStencil.Stencil;
		return return_value;
	}

	constexpr VkAttachmentLoadOp ConvertLoadOp(RHI::RenderPassAttachment::LoadOperation load_op) {
		switch (load_op) {
		case RHI::RenderPassAttachment::LoadOperation::LOAD:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case RHI::RenderPassAttachment::LoadOperation::CLEAR:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case RHI::RenderPassAttachment::LoadOperation::DONT_CARE:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
	}

	constexpr VkAttachmentStoreOp ConvertStoreOp(RHI::RenderPassAttachment::StoreOperation store_op) {
		switch (store_op) {
		case RHI::RenderPassAttachment::StoreOperation::STORE:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case RHI::RenderPassAttachment::StoreOperation::DONT_CARE:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
	}
}
#endif