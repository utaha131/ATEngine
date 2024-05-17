#ifndef _RHI_DX12_TEXTURE_H_
#define _RHI_DX12_TEXTURE_H_
#include "../ITexture.h"
#include "DX12Graphics.h"
#include "./Headers/d3dx12.h"

namespace RHI::DX12 {
	class DX12Texture : public RHI::ITexture {
	public:
		DX12Texture(const TextureDescription& description, ID3D12Resource* dx12_resource);
		~DX12Texture() override;
		inline ID3D12Resource* GetNative() const { return m_DX12Resource; };
	private:
		ID3D12Resource* m_DX12Resource;
	};

	inline constexpr D3D12_RESOURCE_STATES DX12ConvertTextureState(TextureState texture_state) {
		switch (texture_state) {
		case TextureState::CREATED:
			return D3D12_RESOURCE_STATE_COMMON;

		case TextureState::COMMON:
			return D3D12_RESOURCE_STATE_COMMON;

		case TextureState::UNORDERED_ACCESS:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		case TextureState::COPY_DEST:
			return D3D12_RESOURCE_STATE_COPY_DEST;

		case TextureState::COPY_SOURCE:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;

#undef GENERIC_READ
		case TextureState::GENERIC_READ:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
			//Texture Exclusive.
		case TextureState::RENDER_TARGET:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;

		case TextureState::DEPTH_WRITE:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;

		case TextureState::DEPTH_READ:
			return D3D12_RESOURCE_STATE_DEPTH_READ;

		case TextureState::PRESENT:
			return D3D12_RESOURCE_STATE_PRESENT;

		case TextureState::NON_PIXEL_SHADER_RESOURCE:
			return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		case TextureState::PIXEL_SHADER_RESOURCE:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		}
	}

	inline D3D12_RESOURCE_DESC DX12ConvertTextureDescription(const RHI::TextureDescription& description) {
		CD3DX12_RESOURCE_DESC dx12_texture_description;
		switch (description.TextureType) {
		case RHI::TextureType::TEXTURE_1D:
			dx12_texture_description = CD3DX12_RESOURCE_DESC::Tex1D(DX12ConvertFormat(description.Format), description.Width, description.DepthOrArray, description.MipLevels);
			break;
		case RHI::TextureType::TEXTURE_2D:
			dx12_texture_description = CD3DX12_RESOURCE_DESC::Tex2D(DX12ConvertFormat(description.Format), description.Width, description.Height, description.DepthOrArray, description.MipLevels);
			break;
		case RHI::TextureType::TEXTURE_3D:
			dx12_texture_description = CD3DX12_RESOURCE_DESC::Tex3D(DX12ConvertFormat(description.Format), description.Width, description.Height, description.DepthOrArray, description.MipLevels);
			break;
		}

		if (has_flag(description.UsageFlags, TextureUsageFlag::DEPTH_STENCIL)) {
			dx12_texture_description.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		}

		if (has_flag(description.UsageFlags, TextureUsageFlag::RENDER_TARGET)) {
			dx12_texture_description.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		}

		if (has_flag(description.UsageFlags, TextureUsageFlag::UNORDERED_ACCESS)) {
			dx12_texture_description.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		return dx12_texture_description;
	}
}
#endif