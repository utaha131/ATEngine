#ifndef _AT_RENDER_SYSTEM_FRAME_GRAPH_RESOURCES_H_
#define _AT_RENDER_SYSTEM_FRAME_GRAPH_RESOURCES_H_
#include "../RHI/RHI.h"
#include <vector>
#include <unordered_set>

namespace AT {
	typedef class FrameGraphTexture* FrameGraphTextureRef;
	typedef class FrameGraphSRV* FrameGraphSRVRef;
	typedef class FrameGraphRTV* FrameGraphRTVRef;
	typedef class FrameGraphDSV* FrameGraphDSVRef;
	typedef class FrameGraphUAV* FrameGraphUAVRef;

	struct FrameGraphTextureDescription {
		RHI::Format Format;
		uint32_t Width;
		uint32_t Height;
		uint32_t ArraySize;
		uint16_t MipLevels = 1;
		std::optional<RHI::TextureClearValue> Clear_Value;
		bool operator==(const FrameGraphTextureDescription& text) const {
			return (Format == text.Format) && (Width == text.Width) && (Height == text.Height) && (ArraySize == text.ArraySize);
		}
	};

	struct FrameGraphTexture {
		FrameGraphTextureDescription Description;
		RHI::TextureUsageFlag UsageFlags;
		int64_t LastAccessedRenderPassID = -1;
		RHI::TextureState FinalState = RHI::TextureState::CREATED;
		~FrameGraphTexture() {

		}
		RHI::Texture RHIHandle;
	};

	struct FrameGraphSRVDescription {
		uint32_t FirstSliceIndex = 0u;
		uint32_t ArraySize = 1u;
		uint32_t MipLevels = 1u;
	};

	inline RHI::ShaderResourceViewDescription FrameGraphConvertSRVDescription(const FrameGraphSRVDescription& description, RHI::Format format) {
		RHI::ShaderResourceViewDescription rhi_srv_description;
		rhi_srv_description.Format = format;
		if (description.FirstSliceIndex == 0u && description.ArraySize == 1u) {
			rhi_srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::TEXTURE_2D;
			rhi_srv_description.Texture2D.MostDetailedMip = 0u;
			rhi_srv_description.Texture2D.MipLevels = description.MipLevels;
			rhi_srv_description.Texture2D.PlaneSlice = 0u;
			rhi_srv_description.Texture2D.ResourceMinLODClamp = 0.0f;
		}
		else if (description.FirstSliceIndex == 0u && description.ArraySize == 6) { //Handle Cube Texture.
			rhi_srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::TEXTURE_CUBE;
			rhi_srv_description.TextureCube.MostDetailedMip = 0u;
			rhi_srv_description.TextureCube.MipLevels = description.MipLevels;
			rhi_srv_description.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		else {
			rhi_srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::TEXTURE_2D_ARRAY;
			rhi_srv_description.Texture2DArray.FirstArraySlice = description.FirstSliceIndex;
			rhi_srv_description.Texture2DArray.ArraySize = description.ArraySize;
			rhi_srv_description.Texture2DArray.MostDetailedMip = 0u;
			rhi_srv_description.Texture2DArray.MipLevels = description.MipLevels;
			rhi_srv_description.Texture2DArray.PlaneSlice = 0u;
			rhi_srv_description.Texture2DArray.ResourceMinLODClamp = 0.0f;
		}
		return rhi_srv_description;
	}
		 
	struct FrameGraphSRV {
		FrameGraphSRVDescription Description;
		FrameGraphTextureRef TextureRef;
		~FrameGraphSRV() {
			delete RHIHandle;
		}
		RHI::ShaderResourceView RHIHandle;
	};

	struct FrameGraphRTVDescription {
		uint32_t FirstSliceIndex = 0u;
		uint32_t ArraySize = 1u;
		uint32_t MipSlice = 0u;
	};

	inline RHI::RenderTargetViewDescription FrameGraphConvertRTVDescription(const FrameGraphRTVDescription& description, RHI::Format format) {
		RHI::RenderTargetViewDescription rhi_rtv_description;
		rhi_rtv_description.Format = format;
		if (description.FirstSliceIndex == 0u && description.ArraySize == 1u) {
			rhi_rtv_description.ViewDimension = RHI::RenderTargetViewViewDimension::TEXTURE_2D;
			rhi_rtv_description.Texture2D.MipSlice = description.MipSlice;
			rhi_rtv_description.Texture2D.PlaneSlice = 0u;
		}
		else {
			rhi_rtv_description.ViewDimension = RHI::RenderTargetViewViewDimension::TEXTURE_2D_ARRAY;
			rhi_rtv_description.Texture2DArray.FirstArraySlice = description.FirstSliceIndex;
			rhi_rtv_description.Texture2DArray.ArraySize = description.ArraySize;
			rhi_rtv_description.Texture2DArray.MipSlice = description.MipSlice;
			rhi_rtv_description.Texture2DArray.PlaneSlice = 0u;
		}
		return rhi_rtv_description;
	}

	struct FrameGraphRTV {
		FrameGraphRTVDescription Description;
		FrameGraphTextureRef TextureRef;
		~FrameGraphRTV() {
			delete RHIHandle;
		}
		RHI::RenderTargetView RHIHandle;
	};

	struct FrameGraphDSVDescription {
		uint32_t FirstSliceIndex = 0u;
		uint32_t ArraySize = 1u;
	};

	inline RHI::DepthStencilViewDescription FrameGraphConvertDSVDescription(const FrameGraphDSVDescription& description, RHI::Format format) {
		RHI::DepthStencilViewDescription rhi_dsv_description;
		rhi_dsv_description.Format = format;
		if (description.FirstSliceIndex == 0u && description.ArraySize == 1u) {
			rhi_dsv_description.ViewDimension = RHI::DepthStencilViewViewDimension::TEXTURE_2D;
			rhi_dsv_description.Texture2D.MipSlice = 0u;
		}
		else {
			rhi_dsv_description.ViewDimension = RHI::DepthStencilViewViewDimension::TEXTURE_2D_ARRAY;
			rhi_dsv_description.Texture2DArray.MipSlice = 0u;
			rhi_dsv_description.Texture2DArray.FirstArraySlice = description.FirstSliceIndex;
			rhi_dsv_description.Texture2DArray.ArraySize = description.ArraySize;
		}
		return rhi_dsv_description;
	}

	struct FrameGraphDSV {
		FrameGraphDSVDescription Description;
		FrameGraphTextureRef TextureRef;
		~FrameGraphDSV() {
			delete RHIHandle;
		}
		RHI::DepthStencilView RHIHandle;
	};

	struct FrameGraphUAVDescription {
		uint32_t FirstSliceIndex;
		uint32_t ArraySize;
		uint32_t MipSlice;
	};

	inline RHI::UnorderedAccessViewDescription FrameGraphConvertUAVDescription(const FrameGraphUAVDescription& description, RHI::Format format) {
		RHI::UnorderedAccessViewDescription rhi_uav_description;
		rhi_uav_description.Format = format;
		if (description.FirstSliceIndex == 0u && description.ArraySize == 1u) {
			rhi_uav_description.ViewDimension = RHI::UnorderedAccessViewViewDimension::TEXTURE_2D;
			rhi_uav_description.Texture2D.MipSlice = description.MipSlice;
			rhi_uav_description.Texture2D.PlaneSlice = 0u;
		} else {
			rhi_uav_description.ViewDimension = RHI::UnorderedAccessViewViewDimension::TEXTURE_2D_ARRAY;
			rhi_uav_description.Texture2DArray.MipSlice = description.MipSlice;
			rhi_uav_description.Texture2DArray.FirstArraySlice = description.FirstSliceIndex;
			rhi_uav_description.Texture2DArray.ArraySize = description.ArraySize;
			rhi_uav_description.Texture2DArray.PlaneSlice = 0u;
		}
		return rhi_uav_description;
	}

	struct FrameGraphUAV {
		FrameGraphUAVDescription Description;
		FrameGraphTextureRef TextureRef;
		~FrameGraphUAV() {
			delete RHIHandle;
		}
		RHI::UnorderedAccessView RHIHandle;
	};
}
#endif