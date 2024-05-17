#ifndef _RHI_DX12_DEPTH_STENCIL_VIEW_H_
#define _RHI_DX12_DEPTH_STENCIL_VIEW_H_
#include "../IDepthStencilView.h"
#include "./Headers/dx12.h"
#include "DX12ViewAllocator.h"
#include "DX12Graphics.h"
#include "DX12Texture.h"

namespace RHI::DX12 {
	class DX12DepthStencilView : public RHI::IDepthStencilView {
	public:
		DX12DepthStencilView(const RHI::DepthStencilViewDescription& description, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle, DX12Texture* texture) :
			RHI::IDepthStencilView(description),
			m_ViewAllocator(view_allocator),
			m_DX12Handle(dx12_handle),
			m_Texture(texture)
		{

		}

		~DX12DepthStencilView() override {
			m_ViewAllocator.FreeHandle(m_DX12Handle);
		}

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetNative() const { return m_DX12Handle; };
		inline DX12Texture* GetTexture() const { return m_Texture; }
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DX12Handle;
		DX12Texture* m_Texture;
	};

	inline constexpr D3D12_DEPTH_STENCIL_VIEW_DESC DX12ConvertDepthStencilViewDescription(const RHI::DepthStencilViewDescription& description) {
		D3D12_DEPTH_STENCIL_VIEW_DESC dx12_depth_stencil_view_desc = {};
		dx12_depth_stencil_view_desc.Format = DX12ConvertFormat(description.Format);
		switch (description.ViewDimension) {
		case RHI::DepthStencilViewViewDimension::TEXTURE_1D:
			dx12_depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			dx12_depth_stencil_view_desc.Texture1D.MipSlice = description.Texture1D.MipSlice;
			break;
		case RHI::DepthStencilViewViewDimension::TEXTURE_1D_ARRAY:
			dx12_depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			dx12_depth_stencil_view_desc.Texture1DArray.MipSlice = description.Texture1DArray.MipSlice;
			dx12_depth_stencil_view_desc.Texture1DArray.FirstArraySlice = description.Texture1DArray.FirstArraySlice;
			dx12_depth_stencil_view_desc.Texture1DArray.ArraySize = description.Texture1DArray.ArraySize;
			break;
		case RHI::DepthStencilViewViewDimension::TEXTURE_2D:
			dx12_depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dx12_depth_stencil_view_desc.Texture2D.MipSlice = description.Texture2D.MipSlice;
			break;
		case RHI::DepthStencilViewViewDimension::TEXTURE_2D_ARRAY:
			dx12_depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			dx12_depth_stencil_view_desc.Texture2DArray.MipSlice = description.Texture2DArray.MipSlice;
			dx12_depth_stencil_view_desc.Texture2DArray.FirstArraySlice = description.Texture2DArray.FirstArraySlice;
			dx12_depth_stencil_view_desc.Texture2DArray.ArraySize = description.Texture2DArray.ArraySize;
			break;
		}
		return dx12_depth_stencil_view_desc;
	}
}
#endif