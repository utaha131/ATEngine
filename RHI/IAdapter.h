#ifndef _RHI_I_ADAPTER_H_
#define _RHI_I_ADAPTER_H_
#include "RHICore.h"

namespace RHI {
	enum class Vendor {
		NVIDIA,
		AMD,
		INTEL
	};


	class IAdapter {
	public:
		virtual ~IAdapter() = default;
		inline uint64_t GetVRAM() const { return m_VRam; }
		inline RHI::Vendor GetVendor() const { return m_Vendor; }
	protected:
		uint64_t m_VRam;
		RHI::Vendor m_Vendor;
		IAdapter(uint64_t vram, RHI::Vendor vendor) :
			m_VRam(vram),
			m_Vendor(vendor) {};
	};
}
#endif