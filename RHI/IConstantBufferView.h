#ifndef _RHI_I_CONSTANT_BUFFER_VIEW_H_
#define _RHI_I_CONSTANT_BUFFER_VIEW_H_
#include "RHICore.h"

namespace RHI {
	typedef struct ConstantBufferViewDescription {
		RHI::Buffer Buffer;
		uint32_t Size;
	} ConstantBufferViewDescription;

	class IConstantBufferView {
	public:
		virtual ~IConstantBufferView() = default;
		const ConstantBufferViewDescription& GetDescription() const { return m_Description; };
	protected:
		IConstantBufferView(const ConstantBufferViewDescription& description) :
			m_Description(description) {}
		ConstantBufferViewDescription m_Description;
	};
}
#endif