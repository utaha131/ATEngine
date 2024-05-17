#ifndef _RHI_DX12_ROOT_SIGNATURE_H_
#define _RHI_DX12_ROOT_SIGNATURE_H_
#include "../IRootSignature.h"
#include "Headers/dx12.h"
#include "DX12Graphics.h"

namespace RHI::DX12 {
	class DX12RootSignature : public RHI::IRootSignature {
	public:
		DX12RootSignature(const RHI::RootSignatureDescription& description, ID3D12RootSignature* dx12_root_signature);
		~DX12RootSignature() override;

		inline ID3D12RootSignature* GetNative() const {
			return m_DX12RootSignature;
		};
	private:
		ID3D12RootSignature* m_DX12RootSignature;
	};

	inline constexpr D3D12_DESCRIPTOR_RANGE_TYPE DX12ConvertDescriptorRangeType(RHI::DescriptorRangeType range_type) {
		switch (range_type) {												 
		case RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW:
			return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		case RHI::DescriptorRangeType::SAMPLER_VIEW:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		case RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_BUFFER:
		case RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_BUFFER:
		case RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE:
			return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		}
	};

	inline constexpr D3D12_SHADER_VISIBILITY DX12ConvertShaderVisibility(RHI::ShaderVisibility shader_visibility) {
		switch (shader_visibility) {
		case RHI::ShaderVisibility::ALL:
			return D3D12_SHADER_VISIBILITY_ALL;
		case RHI::ShaderVisibility::VERTEX:
			return D3D12_SHADER_VISIBILITY_VERTEX;
		case RHI::ShaderVisibility::HULL:
			return D3D12_SHADER_VISIBILITY_HULL;
		case RHI::ShaderVisibility::DOMAIN:
			return D3D12_SHADER_VISIBILITY_DOMAIN;
		case RHI::ShaderVisibility::GEOMETRY:
			return D3D12_SHADER_VISIBILITY_GEOMETRY;
		case RHI::ShaderVisibility::PIXEL:
			return D3D12_SHADER_VISIBILITY_PIXEL;
		}
	}

	inline constexpr D3D12_STATIC_BORDER_COLOR DX12ConvertStaticSamplerBorderColor(RHI::StaticSamplerDescription::StaticBorderColor border_color) {
		switch (border_color) {
		case RHI::StaticSamplerDescription::StaticBorderColor::TRANSPARENT_BLACK:
			return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		case RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_BLACK:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		case RHI::StaticSamplerDescription::StaticBorderColor::OPAQUE_WHITE:
			return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		}
	};

	inline D3D12_STATIC_SAMPLER_DESC DX12ConvertStaticSamplerDescription(const RHI::StaticSamplerDescription& static_sampler_description) {
		D3D12_STATIC_SAMPLER_DESC dx12_static_sampler_description;
		dx12_static_sampler_description.Filter = DX12ConvertFilter(static_sampler_description.Filter);
		dx12_static_sampler_description.AddressU = DX12ConvertTextureAddressMode(static_sampler_description.AddressU);
		dx12_static_sampler_description.AddressV = DX12ConvertTextureAddressMode(static_sampler_description.AddressV);
		dx12_static_sampler_description.AddressW = DX12ConvertTextureAddressMode(static_sampler_description.AddressW);
		dx12_static_sampler_description.MipLODBias = static_sampler_description.MipLODBias;
		dx12_static_sampler_description.MaxAnisotropy = static_sampler_description.MaxAnisotropy;
		dx12_static_sampler_description.ComparisonFunc = DX12ConvertComparisonFunction(static_sampler_description.ComparisonFunction);
		dx12_static_sampler_description.BorderColor = DX12ConvertStaticSamplerBorderColor(static_sampler_description.BorderColor);
		dx12_static_sampler_description.MinLOD = static_sampler_description.MinLOD;
		dx12_static_sampler_description.MaxLOD = static_sampler_description.MaxLOD;
		dx12_static_sampler_description.ShaderRegister = static_sampler_description.ShaderRegister;
		dx12_static_sampler_description.RegisterSpace = static_sampler_description.RegisterSpace;
		dx12_static_sampler_description.ShaderVisibility = DX12ConvertShaderVisibility(static_sampler_description.ShaderVisibility);
		return dx12_static_sampler_description;
	}
}
#endif