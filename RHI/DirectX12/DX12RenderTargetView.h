#ifndef _RHI_DX12_RENDER_TARGET_VIEW_H_
#define _RHI_DX12_RENDER_TARGET_VIEW_H_
#include "../IRenderTargetView.h"
#include "./Headers/dx12.h"
#include "DX12ViewAllocator.h"
#include "DX12Graphics.h"
#include "DX12Texture.h"

namespace RHI::DX12 {
	class DX12RenderTargetView : public RHI::IRenderTargetView {
	public:
		DX12RenderTargetView(const RHI::RenderTargetViewDescription& description, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle, DX12Texture* texture) : RHI::IRenderTargetView(description), 
			m_ViewAllocator(view_allocator), 
			m_DX12Handle(dx12_handle), 
			m_Texture(texture) 
		{
		
		}
		~DX12RenderTargetView() override {
			m_ViewAllocator.FreeHandle(m_DX12Handle);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetNative() const { return m_DX12Handle; };
		DX12Texture* GetTexture() const { return m_Texture; }
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DX12Handle;
		DX12Texture* m_Texture;
	};

	inline constexpr D3D12_RENDER_TARGET_VIEW_DESC DX12ConvertRenderTargetViewDescription(const RHI::RenderTargetViewDescription& description) {
		D3D12_RENDER_TARGET_VIEW_DESC dx12_render_target_view_description = {};
		dx12_render_target_view_description.Format = DX12ConvertFormat(description.Format);
		switch (description.ViewDimension) {
		case RHI::RenderTargetViewViewDimension::BUFFER:
			dx12_render_target_view_description.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
			dx12_render_target_view_description.Buffer.FirstElement = description.Buffer.FirstElement;
			dx12_render_target_view_description.Buffer.NumElements = description.Buffer.NumberOfElements;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_1D:
			dx12_render_target_view_description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			dx12_render_target_view_description.Texture1D.MipSlice = dx12_render_target_view_description.Texture1D.MipSlice;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_1D_ARRAY:
			dx12_render_target_view_description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			dx12_render_target_view_description.Texture1DArray.MipSlice = description.Texture1DArray.MipSlice;
			dx12_render_target_view_description.Texture1DArray.FirstArraySlice = description.Texture1DArray.FirstArraySlice;
			dx12_render_target_view_description.Texture1DArray.ArraySize = description.Texture1DArray.ArraySize;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_2D:
			dx12_render_target_view_description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			dx12_render_target_view_description.Texture2D.MipSlice = description.Texture2D.MipSlice;
			dx12_render_target_view_description.Texture2D.PlaneSlice = description.Texture2D.PlaneSlice;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_2D_ARRAY:
			dx12_render_target_view_description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			dx12_render_target_view_description.Texture2DArray.MipSlice = description.Texture2DArray.MipSlice;
			dx12_render_target_view_description.Texture2DArray.FirstArraySlice = description.Texture2DArray.FirstArraySlice;
			dx12_render_target_view_description.Texture2DArray.ArraySize = description.Texture2DArray.ArraySize;
			dx12_render_target_view_description.Texture2DArray.PlaneSlice = description.Texture2DArray.PlaneSlice;
			break;
		case RHI::RenderTargetViewViewDimension::TEXTURE_3D:
			dx12_render_target_view_description.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			dx12_render_target_view_description.Texture3D.MipSlice = description.Texture3D.MipSlice;
			dx12_render_target_view_description.Texture3D.FirstWSlice = description.Texture3D.FirstWSlice;
			dx12_render_target_view_description.Texture3D.WSize = description.Texture3D.WSize;
			break;
		}
		return dx12_render_target_view_description;
	}
}
#endif