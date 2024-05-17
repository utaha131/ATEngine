#ifndef _RHI_I_SAMPLER_H_
#define _RHI_I_SAMPLER_H_
#include "RHICore.h"

namespace RHI {
	typedef struct SamplerDescription {
		RHI::Filter Filter;
		RHI::TextureAddressMode AddressU;
		RHI::TextureAddressMode AddressV;
		RHI::TextureAddressMode AddressW;
		float MipLODBias;
		uint32_t MaxAnisotropy;
		RHI::ComparisonFunction ComparisonFunction;
		float Border_Color[4];
		float MinLOD;
		float MaxLOD;
	} SamplerDescription;

	class ISampler {
	public:
		virtual ~ISampler() = default;
		const SamplerDescription& GetDescription() const { return m_Description; }
	protected:
		ISampler(const SamplerDescription& description) :
			m_Description(description) {}
		SamplerDescription m_Description;
	};
}
#endif