#ifndef _RHI_I_RENDER_TARGET_VIEW_H_
#define _RHI_I_RENDER_TARGET_VIEW_H_
#include "RHICore.h"

namespace RHI {
	typedef struct RenderTargetViewBuffer {
		uint64_t FirstElement;
		uint32_t NumberOfElements;
	};

	typedef struct RenderTargetViewTexture1D {
		uint32_t MipSlice;
	};

	typedef struct RenderTargetViewTexture1DArray {
		uint32_t MipSlice;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	};

	typedef struct RenderTargetViewTexture2D {
		uint32_t MipSlice;
		uint32_t PlaneSlice;
	};

	typedef struct RenderTargetViewTexture2DArray {
		uint32_t MipSlice;
		uint32_t PlaneSlice;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	};

	typedef struct RenderTargetViewTexture3D {
		uint32_t MipSlice;
		uint32_t FirstWSlice;
		uint32_t WSize;
	};

	enum class RenderTargetViewViewDimension {
		BUFFER,
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_3D,
	};

	typedef struct RenderTargetViewDescription {
		RHI::Format Format;
		RenderTargetViewViewDimension ViewDimension;
		union {
			RenderTargetViewBuffer Buffer;
			RenderTargetViewTexture1D Texture1D;
			RenderTargetViewTexture1DArray Texture1DArray;
			RenderTargetViewTexture2D Texture2D;
			RenderTargetViewTexture2DArray Texture2DArray;
			RenderTargetViewTexture3D Texture3D;
		};
	} RenderTargetViewDescription;

	class IRenderTargetView {
	public:
		virtual ~IRenderTargetView() = default;
		inline const RenderTargetViewDescription& GetDescription() const { return m_Description; }
	protected:
		IRenderTargetView(const RenderTargetViewDescription& description) :
			m_Description(description) {}
		RenderTargetViewDescription m_Description;
	};
}
#endif