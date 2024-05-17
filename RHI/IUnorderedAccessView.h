#ifndef _RHI_I_UNORDERED_ACCESS_VIEW_H_
#define _RHI_I_UNORDERED_ACCESS_VIEW_H_
#include "RHICore.h"

namespace RHI {
	typedef struct UnorderedAccessViewBuffer {
		uint64_t FirstElement;
		uint32_t NumberOfElements;
		uint32_t ElementStride;
		uint64_t CounterOffset;
	} UnorderedAccessView_Buffer;

	typedef struct UnorderedAccessViewTexture1D {
		uint32_t MipSlice;
	} UnorderedAccessViewTexture1D;

	typedef struct UnorderedAccessViewTexture1DArray {
		uint32_t MipSlice;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	} UnorderedAccessViewTexture1DArray;

	typedef struct UnorderedAccessViewTexture2D {
		uint32_t MipSlice;
		uint32_t PlaneSlice;
	} UnorderedAccessViewTexture2D;

	typedef struct UnorderedAccessViewTexture2DArray {
		uint32_t MipSlice;
		uint32_t PlaneSlice;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	} UnorderedAccessViewTexture2DArray;

	typedef struct UnorderedAccessViewTexture3D {
		uint32_t MipSlice;
		uint32_t FirstWSlice;
		uint32_t WSize;
	} UnorderedAccessViewTexture3D;

	enum class UnorderedAccessViewViewDimension {
		BUFFER,
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_3D,
	};

	typedef struct UnorderedAccessViewDescription {
		RHI::Format Format;
		UnorderedAccessViewViewDimension ViewDimension;
		union {
			UnorderedAccessView_Buffer Buffer;
			UnorderedAccessViewTexture1D Texture1D;
			UnorderedAccessViewTexture1DArray Texture1DArray;
			UnorderedAccessViewTexture2D Texture2D;
			UnorderedAccessViewTexture2DArray Texture2DArray;
			UnorderedAccessViewTexture3D Texture3D;
		};
	} UnorderedAccessViewDescription;

	class IUnorderedAccessView {
	public:
		virtual ~IUnorderedAccessView() = default;
		const UnorderedAccessViewDescription& GetDescription() const { return m_Description; }
	protected:
		IUnorderedAccessView(const UnorderedAccessViewDescription& description) :
			m_Description(description) {}
		UnorderedAccessViewDescription m_Description;
	};
}
#endif