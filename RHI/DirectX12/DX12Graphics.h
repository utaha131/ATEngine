#ifndef _DX12_FORMAT_H_
#define _DX12_FORMAT_H_
#include "../RHICore.h"
#include "./Headers/dx12.h"

namespace RHI::DX12 {
	constexpr DXGI_FORMAT DX12ConvertFormat(RHI::Format format) {
		switch (format) {
		case RHI::Format::UNKNOWN:
			return DXGI_FORMAT_UNKNOWN;
		case RHI::Format::R32G32B32A32_TYPELESS:
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		case RHI::Format::R32G32B32A32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case RHI::Format::R32G32B32A32_UINT:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case RHI::Format::R32G32B32A32_SINT:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case RHI::Format::R32G32B32_TYPELESS:
			return DXGI_FORMAT_R32G32B32_TYPELESS;
		case RHI::Format::R32G32B32_FLOAT:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case RHI::Format::R32G32B32_UINT:
			return DXGI_FORMAT_R32G32B32_UINT;
		case RHI::Format::R32G32B32_SINT:
			return DXGI_FORMAT_R32G32B32_SINT;
		case RHI::Format::R16G16B16A16_TYPELESS:
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		case RHI::Format::R16G16B16A16_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case RHI::Format::R16G16B16A16_UNORM:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case RHI::Format::R16G16B16A16_UINT:
			return DXGI_FORMAT_R16G16B16A16_UINT;
		case RHI::Format::R16G16B16A16_SNORM:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case RHI::Format::R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case RHI::Format::R32G32_TYPELESS:
			return DXGI_FORMAT_R32G32_TYPELESS;
		case RHI::Format::R32G32_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;
		case RHI::Format::R32G32_UINT:
			return DXGI_FORMAT_R32G32_UINT;
		case RHI::Format::R32G32_SINT:
			return DXGI_FORMAT_R32G32_SINT;
		case RHI::Format::R32G8X24_TYPELESS:
			return DXGI_FORMAT_R32G8X24_TYPELESS;
		case RHI::Format::D32_FLOAT_S8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case RHI::Format::R32_FLOAT_X8X24_TYPELESS:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case RHI::Format::X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		case RHI::Format::R10G10B10A2_TYPELESS:
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		case RHI::Format::R10G10B10A2_UNORM:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case RHI::Format::R10G10B10A2_UINT:
			return DXGI_FORMAT_R10G10B10A2_UINT;
		case RHI::Format::R11G11B10_FLOAT:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case RHI::Format::R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case RHI::Format::R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case RHI::Format::R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case RHI::Format::R8G8B8A8_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case RHI::Format::R8G8B8A8_SNORM:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case RHI::Format::R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case RHI::Format::R16G16_TYPELESS:
			return DXGI_FORMAT_R16G16_TYPELESS;
		case RHI::Format::R16G16_FLOAT:
			return DXGI_FORMAT_R16G16_FLOAT;
		case RHI::Format::R16G16_UNORM:
			return DXGI_FORMAT_R16G16_UNORM;
		case RHI::Format::R16G16_UINT:
			return DXGI_FORMAT_R16G16_UINT;
		case RHI::Format::R16G16_SNORM:
			return DXGI_FORMAT_R16G16_SNORM;
		case RHI::Format::R16G16_SINT:
			return DXGI_FORMAT_R16G16_SINT;
		case RHI::Format::R32_TYPELESS:
			return DXGI_FORMAT_R32_TYPELESS;
		case RHI::Format::D32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;
		case RHI::Format::R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case RHI::Format::R32_UINT:
			return DXGI_FORMAT_R32_UINT;
		case RHI::Format::R32_SINT:
			return DXGI_FORMAT_R32_SINT;
		case RHI::Format::R24G8_TYPELESS:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case RHI::Format::D24_UNORM_S8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case RHI::Format::R24_UNORM_X8_TYPELESS:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case RHI::Format::X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		case RHI::Format::R8G8_TYPELESS:
			return DXGI_FORMAT_R8G8_TYPELESS;
		case RHI::Format::R8G8_UNORM:
			return DXGI_FORMAT_R8G8_UNORM;
		case RHI::Format::R8G8_UINT:
			return DXGI_FORMAT_R8G8_UINT;
		case RHI::Format::R8G8_SNORM:
			return DXGI_FORMAT_R8G8_SNORM;
		case RHI::Format::R8G8_SINT:
			return DXGI_FORMAT_R8G8_SINT;
		case RHI::Format::R16_TYPELESS:
			return DXGI_FORMAT_R16_TYPELESS;
		case RHI::Format::R16_FLOAT:
			return DXGI_FORMAT_R16_FLOAT;
		case RHI::Format::D16_UNORM:
			return DXGI_FORMAT_D16_UNORM;
		case RHI::Format::R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case RHI::Format::R16_UINT:
			return DXGI_FORMAT_R16_UINT;
		case RHI::Format::R16_SNORM:
			return DXGI_FORMAT_R16_SNORM;
		case RHI::Format::R16_SINT:
			return DXGI_FORMAT_R16_SINT;
		case RHI::Format::R8_TYPELESS:
			return DXGI_FORMAT_R8_TYPELESS;
		case RHI::Format::R8_UNORM:
			return DXGI_FORMAT_R8_UNORM;
		case RHI::Format::R8_UINT:
			return DXGI_FORMAT_R8_UINT;
		case RHI::Format::R8_SNORM:
			return DXGI_FORMAT_R8_SNORM;
		case RHI::Format::R8_SINT:
			return DXGI_FORMAT_R8_SINT;
		case RHI::Format::A8_UNORM:
			return DXGI_FORMAT_A8_UNORM;
		case RHI::Format::R1_UNORM:
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
			return DXGI_FORMAT_B5G5R5A1_UNORM;
		case RHI::Format::B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case RHI::Format::B8G8R8X8_UNORM:
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
			return DXGI_FORMAT_BC7_UNORM_SRGB;
		}
	}

	//constexpr DXGI_FORMAT ConvertToTypelessFormat(RHI::Format format) {
	//	switch (format) {
	//	case RHI::Format::UNKNOWN:
	//		return DXGI_FORMAT_UNKNOWN;
	//	//FORMAT_R32G32B32A32_TYPELESS = 1,
	//	case RHI::Format::R32G32B32A32_FLOAT:
	//	case RHI::Format::R32G32B32A32_UINT:
	//	case RHI::Format::R32G32B32A32_SINT:
	//		return DXGI_FORMAT_R32G32B32A32_TYPELESS;
	//	//FORMAT_R32G32B32_TYPELESS = 5,
	//	case RHI::Format::R32G32B32_FLOAT:
	//	case RHI::Format::R32G32B32_UINT:
	//	case RHI_FORMAT_R32G32B32_SINT:
	//		return DXGI_FORMAT_R32G32B32_TYPELESS;
	//	//FORMAT_R16G16B16A16_TYPELESS = 9,
	//	case RHI_FORMAT_R16G16B16A16_FLOAT:
	//	case RHI_FORMAT_R16G16B16A16_UNORM:
	//	case RHI_FORMAT_R16G16B16A16_UINT:
	//	case RHI_FORMAT_R16G16B16A16_SNORM:
	//	case RHI_FORMAT_R16G16B16A16_SINT:
	//		return DXGI_FORMAT_R16G16B16A16_TYPELESS;
	//	//FORMAT_R32G32_TYPELESS = 15,
	//	case RHI_FORMAT_R32G32_FLOAT:
	//	case RHI_FORMAT_R32G32_UINT:
	//	case RHI_FORMAT_R32G32_SINT:
	//		return DXGI_FORMAT_R32G32_TYPELESS;
	//	//FORMAT_R32G8X24_TYPELESS = 19,
	//	case RHI_FORMAT_D32_FLOAT_S8X24_UINT:
	//		return DXGI_FORMAT_R32G8X24_TYPELESS;
	//	//FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
	//	case RHI_FORMAT_X32_TYPELESS_G8X24_UINT:
	//		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
	//	//FORMAT_R10G10B10A2_TYPELESS = 23,
	//	case RHI_FORMAT_R10G10B10A2_UNORM:
	//	case RHI_FORMAT_R10G10B10A2_UINT:
	//	case RHI_FORMAT_R11G11B10_FLOAT:
	//		return DXGI_FORMAT_R10G10B10A2_TYPELESS;
	//	//FORMAT_R8G8B8A8_TYPELESS = 27,
	//	case RHI_FORMAT_R8G8B8A8_UNORM:
	//	case RHI_FORMAT_R8G8B8A8_UNORM_SRGB:
	//	case RHI_FORMAT_R8G8B8A8_UINT:
	//	case RHI_FORMAT_R8G8B8A8_SNORM:
	//	case RHI_FORMAT_R8G8B8A8_SINT:
	//		return DXGI_FORMAT_R8G8B8A8_TYPELESS;
	//	//FORMAT_R16G16_TYPELESS = 33,
	//	case RHI_FORMAT_R16G16_FLOAT:
	//	case RHI_FORMAT_R16G16_UNORM:
	//	case RHI_FORMAT_R16G16_UINT:
	//	case RHI_FORMAT_R16G16_SNORM:
	//	case RHI_FORMAT_R16G16_SINT:
	//		return DXGI_FORMAT_R16G16_TYPELESS;
	//	//FORMAT_R32_TYPELESS = 39,
	//	case RHI_FORMAT_D32_FLOAT:
	//	case RHI_FORMAT_R32_FLOAT:
	//	case RHI_FORMAT_R32_UINT:
	//	case RHI_FORMAT_R32_SINT:
	//		return DXGI_FORMAT_R32_TYPELESS;
	//	//FORMAT_R24G8_TYPELESS = 44,
	//	case RHI_FORMAT_D24_UNORM_S8_UINT:
	//		return DXGI_FORMAT_R24G8_TYPELESS;
	//	//FORMAT_R24_UNORM_X8_TYPELESS = 46,
	//	case RHI_FORMAT_X24_TYPELESS_G8_UINT:
	//		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	//	//FORMAT_R8G8_TYPELESS = 48,
	//	case RHI_FORMAT_R8G8_UNORM:
	//	case RHI_FORMAT_R8G8_UINT:
	//	case RHI_FORMAT_R8G8_SNORM:
	//	case RHI_FORMAT_R8G8_SINT:
	//		return DXGI_FORMAT_R8G8_TYPELESS;
	//	//FORMAT_R16_TYPELESS = 53,
	//	case RHI_FORMAT_R16_FLOAT:
	//	case RHI_FORMAT_D16_UNORM:
	//	case RHI_FORMAT_R16_UNORM:
	//	case RHI_FORMAT_R16_UINT:
	//	case RHI_FORMAT_R16_SNORM:
	//	case RHI_FORMAT_R16_SINT:
	//		return DXGI_FORMAT_R16_TYPELESS;
	//	//FORMAT_R8_TYPELESS = 60,
	//	case RHI_FORMAT_R8_UNORM:
	//	case RHI_FORMAT_R8_UINT:
	//	case RHI_FORMAT_R8_SNORM:
	//	case RHI_FORMAT_R8_SINT:
	//	case RHI_FORMAT_A8_UNORM:
	//		return DXGI_FORMAT_R8_TYPELESS;
	//	case RHI_FORMAT_B8G8R8A8_UNORM:
	//	//FORMAT_B8G8R8A8_TYPELESS = 90,
	//	case RHI_FORMAT_B8G8R8A8_UNORM_SRGB:
	//		return DXGI_FORMAT_B8G8R8A8_TYPELESS;
	//	//FORMAT_B8G8R8X8_TYPELESS = 92,
	//	case RHI_FORMAT_B8G8R8X8_UNORM_SRGB:
	//		return DXGI_FORMAT_B8G8R8X8_TYPELESS;
	//	//FORMAT_BC1_TYPELESS = 70,
	//	case RHI_FORMAT_BC1_UNORM:
	//	case RHI_FORMAT_BC1_UNORM_SRGB:
	//		return DXGI_FORMAT_BC1_TYPELESS;
	//	//FORMAT_BC2_TYPELESS = 73,
	//	case RHI_FORMAT_BC2_UNORM:
	//	case RHI_FORMAT_BC2_UNORM_SRGB:
	//		return DXGI_FORMAT_BC2_TYPELESS;
	//	//FORMAT_BC3_TYPELESS = 76,
	//	case RHI_FORMAT_BC3_UNORM:
	//	case RHI_FORMAT_BC3_UNORM_SRGB:
	//		return DXGI_FORMAT_BC3_TYPELESS;
	//	//FORMAT_BC4_TYPELESS = 79,
	//	case RHI_FORMAT_BC4_UNORM:
	//	case RHI_FORMAT_BC4_SNORM:
	//		return DXGI_FORMAT_BC4_TYPELESS;
	//	//FORMAT_BC5_TYPELESS = 82,
	//	case RHI_FORMAT_BC5_UNORM:
	//	case RHI_FORMAT_BC5_SNORM:
	//		return DXGI_FORMAT_BC5_TYPELESS;
	//	//FORMAT_BC6H_TYPELESS = 94,
	//	case RHI_FORMAT_BC6H_UF16:
	//	case RHI_FORMAT_BC6H_SF16:
	//		return DXGI_FORMAT_BC6H_TYPELESS;
	//	//FORMAT_BC7_TYPELESS,
	//	case RHI_FORMAT_BC7_UNORM:
	//	case RHI_FORMAT_BC7_UNORM_SRGB:
	//		return DXGI_FORMAT_BC7_TYPELESS;
	//	}
	//}

	constexpr DXGI_FORMAT DepthToColorFormat(RHI::Format format) {
		switch (format) {
		case RHI::Format::D32_FLOAT_S8X24_UINT:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case RHI::Format::D16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		}
	}

	constexpr D3D12_RECT DX12ConvertRect(const RHI::Rect& rect) {
		D3D12_RECT native_rect = {
			.left = rect.Left,
			.top = rect.Top,
			.right = rect.Right,
			.bottom = rect.Bottom,
		};
		return native_rect;
	};

	constexpr D3D12_VIEWPORT DX12ConvertViewport(const RHI::Viewport& viewport) {
		D3D12_VIEWPORT native_viewport = {
			.TopLeftX = viewport.TopLeftX,
			.TopLeftY = viewport.TopLeftY,
			.Width = viewport.Width,
			.Height = viewport.Height,
			.MinDepth = viewport.MinDepth,
			.MaxDepth = viewport.MaxDepth,
		};
		return native_viewport;
	};

	constexpr D3D12_DEPTH_WRITE_MASK DX12ConvertDepthWriteMask(RHI::DepthWriteMask depth_write_mask) {
		switch (depth_write_mask) {
		case RHI::DepthWriteMask::ZERO:
			return D3D12_DEPTH_WRITE_MASK_ZERO;
		case RHI::DepthWriteMask::ALL:
			return D3D12_DEPTH_WRITE_MASK_ALL;
		}
	};

	constexpr D3D12_COMPARISON_FUNC DX12ConvertComparisonFunction(RHI::ComparisonFunction comparison_function) {
		switch (comparison_function) {
		case RHI::ComparisonFunction::NEVER:
			return D3D12_COMPARISON_FUNC_NEVER;
		case RHI::ComparisonFunction::LESS:
			return D3D12_COMPARISON_FUNC_LESS;
		case RHI::ComparisonFunction::EQUAL:
			return D3D12_COMPARISON_FUNC_EQUAL;
		case RHI::ComparisonFunction::LESS_EQUAL:
			return D3D12_COMPARISON_FUNC_LESS_EQUAL;
		case RHI::ComparisonFunction::GREATER:
			return D3D12_COMPARISON_FUNC_GREATER;
		case RHI::ComparisonFunction::NOT_EQUAL:
			return D3D12_COMPARISON_FUNC_NOT_EQUAL;
		case RHI::ComparisonFunction::GREATER_EQUAL:
			return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		case RHI::ComparisonFunction::ALWAYS:
			return D3D12_COMPARISON_FUNC_ALWAYS;
		}
	};

	constexpr D3D12_STENCIL_OP DX12ConverteStencilOperation(RHI::StencilOperation stencil_operation) {
		switch (stencil_operation) {
		case RHI::StencilOperation::KEEP:
			return D3D12_STENCIL_OP_KEEP;
		case RHI::StencilOperation::ZERO:
			return D3D12_STENCIL_OP_ZERO;
		case RHI::StencilOperation::REPLACE:
			return D3D12_STENCIL_OP_REPLACE;
		case RHI::StencilOperation::INCREASE_SATURATE:
			return D3D12_STENCIL_OP_INCR_SAT;
		case RHI::StencilOperation::DECREASE_SATURATE:
			return D3D12_STENCIL_OP_DECR_SAT;
		case RHI::StencilOperation::INVERT:
			return D3D12_STENCIL_OP_INVERT;
		case RHI::StencilOperation::INCREASE:
			return D3D12_STENCIL_OP_INCR;
		case RHI::StencilOperation::DECREASE:
			return D3D12_STENCIL_OP_DECR;
		}
	}

	constexpr D3D12_FILL_MODE DX12ConvertFillMode(RHI::FillMode fill_mode) {
		switch (fill_mode) {
		case RHI::FillMode::WIREFRAME:
			return D3D12_FILL_MODE_WIREFRAME;
		case RHI::FillMode::SOLID:
			return D3D12_FILL_MODE_SOLID;
		}
	}

	constexpr D3D12_CULL_MODE DX12ConvertCullMode(RHI::CullMode cull_mode) {
		switch (cull_mode) {
		case RHI::CullMode::NONE:
			return D3D12_CULL_MODE_NONE;
		case RHI::CullMode::FRONT:
			return D3D12_CULL_MODE_FRONT;
		case RHI::CullMode::BACK:
			return D3D12_CULL_MODE_BACK;
		}
	}

	constexpr D3D12_BLEND DX12ConvertBlend(RHI::Blend blend) {
		switch (blend) {
		case RHI::Blend::ZERO:
			return D3D12_BLEND_ZERO;
		case RHI::Blend::ONE:
			return D3D12_BLEND_ONE;
		case RHI::Blend::SOURCE_COLOR:
			return D3D12_BLEND_SRC_COLOR;
		case RHI::Blend::INVERSE_SOURCE_COLOR:
			return D3D12_BLEND_INV_SRC_COLOR;
		case RHI::Blend::SOURCE_ALPHA:
			return D3D12_BLEND_SRC_ALPHA;
		case RHI::Blend::INVERSE_SOURCE_ALPHA:
			return D3D12_BLEND_INV_SRC_ALPHA;
		case RHI::Blend::DESTINATION_ALPHA:
			return D3D12_BLEND_DEST_ALPHA;
		case RHI::Blend::INVERSE_DESTINATION_ALPHA:
			return D3D12_BLEND_INV_DEST_ALPHA;
		case RHI::Blend::DESTINATION_COLOR:
			return D3D12_BLEND_DEST_COLOR;
		case RHI::Blend::INVERSE_DESTINATION_COLOR:
			return D3D12_BLEND_INV_DEST_COLOR;
		case RHI::Blend::SOURCE_ALPHA_SATURATE:
			return D3D12_BLEND_SRC_ALPHA_SAT;
		case RHI::Blend::BLEND_FACTOR:
			return D3D12_BLEND_BLEND_FACTOR;
		case RHI::Blend::INVERSE_BLEND_FACTOR:
			return D3D12_BLEND_INV_BLEND_FACTOR;
		case RHI::Blend::SOURCE1_COLOR:
			return D3D12_BLEND_SRC1_COLOR;
		case RHI::Blend::INVERSE_SOURCE1_COLOR:
			return D3D12_BLEND_INV_SRC1_COLOR;
		case RHI::Blend::SOURCE1_ALPHA:
			return D3D12_BLEND_SRC1_ALPHA;
		case RHI::Blend::INVERSE_SOURCE1_ALPHA:
			return D3D12_BLEND_INV_SRC1_ALPHA;
		}
	}

	constexpr D3D12_BLEND_OP DX12ConvertBlendOperation(RHI::BlendOperation blend_operation) {
		switch (blend_operation) {
		case RHI::BlendOperation::ADD:
			return D3D12_BLEND_OP_ADD;
		case RHI::BlendOperation::SUBTRACT:
			return D3D12_BLEND_OP_SUBTRACT;
		case RHI::BlendOperation::REVERSE_SUBTRACT:
			return D3D12_BLEND_OP_REV_SUBTRACT;
		case RHI::BlendOperation::MIN:
			return D3D12_BLEND_OP_MIN;
		case RHI::BlendOperation::MAX:
			return D3D12_BLEND_OP_MAX;
		}
	}

	constexpr D3D12_LOGIC_OP DX12ConvertLogicOperation(RHI::LogicOperation logic_operation) {
		switch (logic_operation) {
		case RHI::LogicOperation::CLEAR:
			return D3D12_LOGIC_OP_CLEAR;
		case RHI::LogicOperation::SET:
			return D3D12_LOGIC_OP_SET;
		case RHI::LogicOperation::COPY:
			return D3D12_LOGIC_OP_COPY;
		case RHI::LogicOperation::COPY_INVERTED:
			return D3D12_LOGIC_OP_COPY_INVERTED;
		case RHI::LogicOperation::NOOP:
			return D3D12_LOGIC_OP_NOOP;
		case RHI::LogicOperation::INVERT:
			return D3D12_LOGIC_OP_INVERT;
		case RHI::LogicOperation::AND:
			return D3D12_LOGIC_OP_AND;
		case RHI::LogicOperation::NAND:
			return D3D12_LOGIC_OP_NAND;
		case RHI::LogicOperation::OR:
			return D3D12_LOGIC_OP_OR;
		case RHI::LogicOperation::NOR:
			return D3D12_LOGIC_OP_NOR;
		case RHI::LogicOperation::XOR:
			return D3D12_LOGIC_OP_XOR;
		case RHI::LogicOperation::EQUIVALENT:
			return D3D12_LOGIC_OP_EQUIV;
		case RHI::LogicOperation::AND_REVERSE:
			return D3D12_LOGIC_OP_AND_REVERSE;
		case RHI::LogicOperation::AND_INVERTED:
			return D3D12_LOGIC_OP_AND_INVERTED;
		case RHI::LogicOperation::OR_REVERSE:
			return D3D12_LOGIC_OP_OR_REVERSE;
		case RHI::LogicOperation::OR_INVERTED:
			return D3D12_LOGIC_OP_OR_INVERTED;
		}
	}

	constexpr D3D12_COLOR_WRITE_ENABLE DX12ConvertColorWriteMask(RHI::ColorWriteEnable color_write_enable) {
		D3D12_COLOR_WRITE_ENABLE flag = {};
		if (color_write_enable == RHI::ColorWriteEnable::ALL) {
			flag = D3D12_COLOR_WRITE_ENABLE_ALL;
			return flag;
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::RED)) {
			flag = static_cast<D3D12_COLOR_WRITE_ENABLE>(flag | D3D12_COLOR_WRITE_ENABLE_RED);
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::GREEN)) {
			flag = static_cast<D3D12_COLOR_WRITE_ENABLE>(flag | D3D12_COLOR_WRITE_ENABLE_GREEN);
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::BLUE)) {
			flag = static_cast<D3D12_COLOR_WRITE_ENABLE>(flag | D3D12_COLOR_WRITE_ENABLE_BLUE);
		}
		if (has_flag(color_write_enable, RHI::ColorWriteEnable::ALPHA)) {
			flag = static_cast<D3D12_COLOR_WRITE_ENABLE>(flag | D3D12_COLOR_WRITE_ENABLE_ALPHA);
		}
		return flag;
	}

	//constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE DX12ConvertPrimitiveTopologyType(PRIMITIVE_TOPOLOGY_TYPE primitive_topology_type) {
	//	switch (primitive_topology_type) {
	//	case PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED:
	//		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	//	case PRIMITIVE_TOPOLOGY_TYPE_POINT:
	//		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	//	case PRIMITIVE_TOPOLOGY_TYPE_LINE:
	//		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	//	case PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
	//		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//	case PRIMITIVE_TOPOLOGY_TYPE_PATCH:
	//		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	//	}
	//}

	constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE DX12ConvertPrimitiveTopologyType(RHI::PrimitiveTopology primitive_topology) {
		switch (primitive_topology) {
		case RHI::PrimitiveTopology::POINT_LIST:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		case RHI::PrimitiveTopology::LINE_LIST:
		case RHI::PrimitiveTopology::LINE_STRIP:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		case RHI::PrimitiveTopology::TRIANGLE_LIST:
		case RHI::PrimitiveTopology::TRIANGLE_STRIP:
			return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		}
	}

	constexpr D3D_PRIMITIVE_TOPOLOGY DX12ConvertPrimitiveTopology(RHI::PrimitiveTopology primitive_topology) {
		switch (primitive_topology) {
		case RHI::PrimitiveTopology::POINT_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case RHI::PrimitiveTopology::LINE_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case RHI::PrimitiveTopology::LINE_STRIP:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case RHI::PrimitiveTopology::TRIANGLE_LIST:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case RHI::PrimitiveTopology::TRIANGLE_STRIP:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		}
	}

	constexpr D3D12_FILTER DX12ConvertFilter(RHI::Filter filter) {
		switch (filter) {
		case RHI::Filter::MIN_MAG_MIP_POINT:
			return D3D12_FILTER_MIN_MAG_MIP_POINT;
		case RHI::Filter::MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case RHI::Filter::MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case RHI::Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		case RHI::Filter::ANISOTROPIC:
			return D3D12_FILTER_ANISOTROPIC;
		case RHI::Filter::COMPARISON_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case RHI::Filter::COMPARISON_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::COMPARISON_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		case RHI::Filter::COMPARISON_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		case RHI::Filter::COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::COMPARISON_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::COMPARISON_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case RHI::Filter::COMPARISON_ANISOTROPIC:
			return D3D12_FILTER_COMPARISON_ANISOTROPIC;
		case RHI::Filter::MINIMUM_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
		case RHI::Filter::MINIMUM_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::MINIMUM_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case RHI::Filter::MINIMUM_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case RHI::Filter::MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::MINIMUM_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::MINIMUM_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
		case RHI::Filter::MINIMUM_ANISOTROPIC:
			return D3D12_FILTER_MINIMUM_ANISOTROPIC;
		case RHI::Filter::MAXIMUM_MIN_MAG_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
		case RHI::Filter::MAXIMUM_MIN_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::MAXIMUM_MIN_POINT_MAG_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case RHI::Filter::MAXIMUM_MIN_LINEAR_MAG_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case RHI::Filter::MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case RHI::Filter::MAXIMUM_MIN_MAG_LINEAR_MIP_POINT:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case RHI::Filter::MAXIMUM_MIN_MAG_MIP_LINEAR:
			return D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
		case RHI::Filter::MAXIMUM_ANISOTROPIC:
			return D3D12_FILTER_MAXIMUM_ANISOTROPIC;
		}
	}

	constexpr D3D12_TEXTURE_ADDRESS_MODE DX12ConvertTextureAddressMode(RHI::TextureAddressMode texture_address_mode) {
		switch (texture_address_mode) {
		case RHI::TextureAddressMode::WRAP:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case RHI::TextureAddressMode::MIRROR:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case RHI::TextureAddressMode::CLAMP:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case RHI::TextureAddressMode::BORDER:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		case RHI::TextureAddressMode::MIRROR_ONCE:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
		}
	}

	constexpr D3D12_CLEAR_VALUE DX12ConvertClearValue(const RHI::TextureClearValue& clear_value, RHI::Format format) {
		D3D12_CLEAR_VALUE return_value = {};
		return_value.Format = DX12ConvertFormat(format);
		return_value.Color[0] = clear_value.Color[0];
		return_value.Color[1] = clear_value.Color[1];
		return_value.Color[2] = clear_value.Color[2];
		return_value.Color[3] = clear_value.Color[3];
		return_value.DepthStencil.Depth = clear_value.DepthAndStencil.Depth;
		return_value.DepthStencil.Stencil = clear_value.DepthAndStencil.Stencil;
		return return_value;
	}
}
#endif