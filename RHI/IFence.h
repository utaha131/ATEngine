#ifndef _RHI_I_FENCE_H_
#define _RHI_I_FENCE_H_
#include "RHICore.h"

namespace RHI {
	class IFence {
	public:
		virtual ~IFence() {}
		virtual void Signal(uint64_t value) const = 0;
		virtual uint64_t GetCompletedValue() const = 0;
	};
}
#endif