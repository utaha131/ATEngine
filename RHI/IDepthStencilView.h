#ifndef _RHI_I_DEPTH_STENCIL_VIEW_H_
#define _RHI_I_DEPTH_STENCIL_VIEW_H_
#include "RHICore.h"

namespace RHI {
	struct DepthStencilViewTexture1D {
		uint32_t MipSlice;
	};

	struct DepthStencilViewTexture1DArray {
		uint32_t MipSlice;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	};

	struct DepthStencilViewTexture2D {
		uint32_t MipSlice;
	};

	struct DepthStencilViewTexture2DArray {
		uint32_t MipSlice;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	};
	enum class DepthStencilViewViewDimension {
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY
	};
	typedef struct DepthStencilViewDescription {
		RHI::Format Format;
		DepthStencilViewViewDimension ViewDimension;
		union {
			DepthStencilViewTexture1D Texture1D;
			DepthStencilViewTexture1DArray Texture1DArray;
			DepthStencilViewTexture2D Texture2D;
			DepthStencilViewTexture2DArray Texture2DArray;
		};
	} DepthStencilViewDescription;

	class IDepthStencilView {
	public:
		virtual ~IDepthStencilView() = default;
		const DepthStencilViewDescription& GetDescription() const { return m_Description; }
	protected:
		IDepthStencilView(const DepthStencilViewDescription& description) :
			m_Description(description) {}
		DepthStencilViewDescription m_Description;
	};
}
#endif