#ifndef _RHI_I_SHADER_RESOURCE_VIEW_H_
#define _RHI_I_SHADER_RESOURCE_VIEW_H_
#include "RHICore.h"

namespace RHI {
	typedef struct ShaderResourceViewBuffer {
		uint64_t FirstElement;
		uint32_t NumberOfElements;
		uint32_t ElementStride;
	};

	typedef struct ShaderResourceViewTexture1D {
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		float ResourceMinLODClamp;
	};

	typedef struct ShaderResourceViewTexture1DArray {
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		float ResourceMinLODClamp;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	};

	typedef struct ShaderResourceViewTexture2D {
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		uint32_t PlaneSlice;
		float ResourceMinLODClamp;
	};

	typedef struct ShaderResourceViewTexture2DArray {
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		uint32_t PlaneSlice;
		float ResourceMinLODClamp;
		uint32_t FirstArraySlice;
		uint32_t ArraySize;
	};

	typedef struct ShaderResourceViewTexture3D {
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		float ResourceMinLODClamp;
	};

	typedef struct ShaderResourceViewTextureCube {
		uint32_t MostDetailedMip;
		uint32_t MipLevels;
		float ResourceMinLODClamp;
	};

	enum class ShaderResourceViewViewDimension {
		BUFFER,
		TEXTURE_1D,
		TEXTURE_1D_ARRAY,
		TEXTURE_2D,
		TEXTURE_2D_ARRAY,
		TEXTURE_3D,
		TEXTURE_CUBE
	};

	typedef struct ShaderResourceViewDescription {
		RHI::Format Format;
		ShaderResourceViewViewDimension ViewDimension;
		union {
			ShaderResourceViewBuffer Buffer;
			ShaderResourceViewTexture1D Texture1D;
			ShaderResourceViewTexture1DArray Texture1DArray;
			ShaderResourceViewTexture2D Texture2D;
			ShaderResourceViewTexture2DArray Texture2DArray;
			ShaderResourceViewTexture3D Texture3D;
			ShaderResourceViewTextureCube TextureCube;
		};
	} ShaderResourceViewDescription;

	class IShaderResourceView {
	public:
		virtual ~IShaderResourceView() = default;
		const ShaderResourceViewDescription& GetDescription() const { return m_Description; }
	protected:
		IShaderResourceView(const ShaderResourceViewDescription& description) :
			m_Description(description) {}
		ShaderResourceViewDescription m_Description;
	};
}
#endif