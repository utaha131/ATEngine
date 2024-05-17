#ifndef _RHI_I_TEXTURE_H_
#define _RHI_I_TEXTURE_H_
#include "RHICore.h"

namespace RHI {
	/*inline RHI_TEXTURE_USAGE_FLAG operator | (RHI_TEXTURE_USAGE_FLAG flag1, RHI_TEXTURE_USAGE_FLAG flag2) {
		return static_cast<RHI_TEXTURE_USAGE_FLAG>(static_cast<int>(flag1) | static_cast<int>(flag2));
	}*/

	typedef struct TextureDescription {
		TextureType TextureType;
		RHI::Format Format;
		uint32_t Width;
		uint32_t Height;
		uint16_t DepthOrArray;
		uint16_t MipLevels;
		RHI::TextureUsageFlag UsageFlags;
	} TextureDescription;

	class ITexture {
	public:
		virtual ~ITexture() = default;
		TextureDescription GetDescription() const { return m_Description; }
	protected:
		TextureDescription m_Description;
		ITexture(const TextureDescription& description) :
			m_Description(description)
		{};
	};
}
#endif