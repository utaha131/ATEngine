#ifndef _RHI_I_COMMAND_ALLOCATOR_H_
#define _RHI_I_COMMAND_ALLOCATOR_H_
#include "RHICore.h"

namespace RHI {
	class ICommandAllocator {
	public:
		virtual ~ICommandAllocator() = default;
		virtual void Reset() = 0;
	protected:
		RHI::CommandType m_CommandType;
		ICommandAllocator(RHI::CommandType command_type) :
			m_CommandType(command_type) {};
	};
}
#endif