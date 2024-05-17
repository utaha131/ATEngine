#ifndef _RHI_DX12_UNORDERED_ACCESS_VIEW_H_
#define _RHI_DX12_UNORDERED_ACCESS_VIEW_H_
#include "../IUnorderedAccessView.h"
#include "./Headers/dx12.h"
#include "DX12ViewAllocator.h"
#include "DX12Graphics.h"

namespace RHI::DX12 {
	class DX12UnorderedAccessView : public RHI::IUnorderedAccessView {
	public:
		DX12UnorderedAccessView(const RHI::UnorderedAccessViewDescription& description, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle) :
			IUnorderedAccessView(description),
			m_ViewAllocator(view_allocator),
			m_DX12Handle(dx12_handle)
		{

		}
		~DX12UnorderedAccessView() override {
			m_ViewAllocator.FreeHandle(m_DX12Handle);
		}

		inline D3D12_CPU_DESCRIPTOR_HANDLE GetNative() const { return m_DX12Handle; }
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DX12Handle;
	};

	inline D3D12_UNORDERED_ACCESS_VIEW_DESC DX12ConvertUnorderedAccessViewDescription(const RHI::UnorderedAccessViewDescription& description) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC dx12_unordered_access_view_description;
		dx12_unordered_access_view_description.Format = DX12ConvertFormat(description.Format);
		switch (description.ViewDimension) {
		case RHI::UnorderedAccessViewViewDimension::BUFFER:
			dx12_unordered_access_view_description.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			dx12_unordered_access_view_description.Buffer.CounterOffsetInBytes = description.Buffer.CounterOffset;
			dx12_unordered_access_view_description.Buffer.FirstElement = description.Buffer.FirstElement;
			dx12_unordered_access_view_description.Buffer.NumElements = description.Buffer.NumberOfElements;
			dx12_unordered_access_view_description.Buffer.StructureByteStride = description.Buffer.ElementStride;
			dx12_unordered_access_view_description.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_1D:
			dx12_unordered_access_view_description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			dx12_unordered_access_view_description.Texture1D.MipSlice = description.Texture1D.MipSlice;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_1D_ARRAY:
			dx12_unordered_access_view_description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			dx12_unordered_access_view_description.Texture1DArray.MipSlice = description.Texture1DArray.MipSlice;
			dx12_unordered_access_view_description.Texture1DArray.FirstArraySlice = description.Texture1DArray.FirstArraySlice;
			dx12_unordered_access_view_description.Texture1DArray.ArraySize = description.Texture1DArray.ArraySize;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_2D:
			dx12_unordered_access_view_description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			dx12_unordered_access_view_description.Texture2D.MipSlice = description.Texture2D.MipSlice;
			dx12_unordered_access_view_description.Texture2D.PlaneSlice = description.Texture2D.PlaneSlice;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_2D_ARRAY:
			dx12_unordered_access_view_description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			dx12_unordered_access_view_description.Texture2DArray.MipSlice = description.Texture2DArray.MipSlice;
			dx12_unordered_access_view_description.Texture2DArray.FirstArraySlice = description.Texture2DArray.FirstArraySlice;
			dx12_unordered_access_view_description.Texture2DArray.ArraySize = description.Texture2DArray.ArraySize;
			dx12_unordered_access_view_description.Texture2DArray.PlaneSlice = description.Texture2DArray.PlaneSlice;
			break;
		case RHI::UnorderedAccessViewViewDimension::TEXTURE_3D:
			dx12_unordered_access_view_description.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			dx12_unordered_access_view_description.Texture3D.MipSlice = description.Texture3D.MipSlice;
			dx12_unordered_access_view_description.Texture3D.FirstWSlice = description.Texture3D.FirstWSlice;
			dx12_unordered_access_view_description.Texture3D.WSize = description.Texture3D.WSize;
			break;
		}
		return dx12_unordered_access_view_description;
	}
}
#endif