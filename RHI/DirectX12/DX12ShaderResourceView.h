#ifndef _RHI_DX12_SHADER_RESOURCE_VIEW_H_
#define _RHI_DX12_SHADER_RESOURCE_VIEW_H_
#include "../IShaderResourceView.h"
#include "./Headers/dx12.h"
#include "DX12ViewAllocator.h"
#include "DX12Graphics.h"

namespace RHI::DX12 {
	class DX12ShaderResourceView : public RHI::IShaderResourceView {
	public:
		DX12ShaderResourceView(const RHI::ShaderResourceViewDescription& description, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle) :
			RHI::IShaderResourceView(description),
			m_ViewAllocator(view_allocator),
			m_DX12Handle(dx12_handle)
		{

		}
		~DX12ShaderResourceView() override {
			m_ViewAllocator.FreeHandle(m_DX12Handle);
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetNative() const { return m_DX12Handle; }
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DX12Handle;
	};

	inline D3D12_SHADER_RESOURCE_VIEW_DESC DX12ConvertShaderResourceViewDescription(const RHI::ShaderResourceViewDescription& description) {
		D3D12_SHADER_RESOURCE_VIEW_DESC dx12_shader_resource_view_description;
		dx12_shader_resource_view_description.Format = DX12ConvertFormat(description.Format);
		if (RHI_Format_Is_Depth(description.Format)) {
			dx12_shader_resource_view_description.Format = DepthToColorFormat(description.Format);
		}
		dx12_shader_resource_view_description.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		switch (description.ViewDimension) {
		case RHI::ShaderResourceViewViewDimension::BUFFER:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			dx12_shader_resource_view_description.Buffer.FirstElement = description.Buffer.FirstElement;
			dx12_shader_resource_view_description.Buffer.NumElements = description.Buffer.NumberOfElements;
			dx12_shader_resource_view_description.Buffer.StructureByteStride = description.Buffer.ElementStride;
			dx12_shader_resource_view_description.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_1D:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			dx12_shader_resource_view_description.Texture1D.MostDetailedMip = description.Texture1D.MostDetailedMip;
			dx12_shader_resource_view_description.Texture1D.MipLevels = description.Texture1D.MipLevels;
			dx12_shader_resource_view_description.Texture1D.ResourceMinLODClamp = description.Texture1D.ResourceMinLODClamp;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_1D_ARRAY:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			dx12_shader_resource_view_description.Texture1DArray.MostDetailedMip = description.Texture1DArray.MostDetailedMip;
			dx12_shader_resource_view_description.Texture1DArray.MipLevels = description.Texture1DArray.MipLevels;
			dx12_shader_resource_view_description.Texture1DArray.FirstArraySlice = description.Texture1DArray.FirstArraySlice;
			dx12_shader_resource_view_description.Texture1DArray.ArraySize = description.Texture1DArray.ArraySize;
			dx12_shader_resource_view_description.Texture1DArray.ResourceMinLODClamp = description.Texture1DArray.ResourceMinLODClamp;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_2D:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			dx12_shader_resource_view_description.Texture2D.MostDetailedMip = description.Texture2D.MostDetailedMip;
			dx12_shader_resource_view_description.Texture2D.MipLevels = description.Texture2D.MipLevels;
			dx12_shader_resource_view_description.Texture2D.PlaneSlice = description.Texture2D.PlaneSlice;
			dx12_shader_resource_view_description.Texture2D.ResourceMinLODClamp = description.Texture2D.ResourceMinLODClamp;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_2D_ARRAY:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			dx12_shader_resource_view_description.Texture2DArray.MostDetailedMip = description.Texture2DArray.MostDetailedMip;
			dx12_shader_resource_view_description.Texture2DArray.MipLevels = description.Texture2DArray.MipLevels;
			dx12_shader_resource_view_description.Texture2DArray.FirstArraySlice = description.Texture2DArray.FirstArraySlice;
			dx12_shader_resource_view_description.Texture2DArray.ArraySize = description.Texture2DArray.ArraySize;
			dx12_shader_resource_view_description.Texture2DArray.PlaneSlice = description.Texture2DArray.PlaneSlice;
			dx12_shader_resource_view_description.Texture2DArray.ResourceMinLODClamp = description.Texture2DArray.ResourceMinLODClamp;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_3D:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			dx12_shader_resource_view_description.Texture3D.MostDetailedMip = description.Texture3D.MostDetailedMip;
			dx12_shader_resource_view_description.Texture3D.MipLevels = description.Texture3D.MipLevels;
			dx12_shader_resource_view_description.Texture3D.ResourceMinLODClamp = description.Texture3D.ResourceMinLODClamp;
			break;
		case RHI::ShaderResourceViewViewDimension::TEXTURE_CUBE:
			dx12_shader_resource_view_description.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			dx12_shader_resource_view_description.TextureCube.MostDetailedMip = description.TextureCube.MostDetailedMip;
			dx12_shader_resource_view_description.TextureCube.MipLevels = description.TextureCube.MipLevels;
			dx12_shader_resource_view_description.TextureCube.ResourceMinLODClamp = description.TextureCube.ResourceMinLODClamp;
			break;
		}
		return dx12_shader_resource_view_description;
	}
}
#endif