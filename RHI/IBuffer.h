#ifndef _RHI_I_BUFFER_H_
#define _RHI_I_BUFFER_H_
#include "RHICore.h"

namespace RHI {
	//inline RHI_BUFFER_USAGE_FLAG operator | (RHI_BUFFER_USAGE_FLAG flag1, RHI_BUFFER_USAGE_FLAG flag2) {
	//	return static_cast<RHI_BUFFER_USAGE_FLAG>(static_cast<int>(flag1) | static_cast<int>(flag2));
	//}

	typedef struct BufferDescription {
		uint64_t Size;
		BufferUsageFlag UsageFlags;
	};

	class IBuffer {
	public:
		virtual ~IBuffer() = default;
		virtual void Map() = 0;
		virtual void CopyData(uint64_t offset, const void* data, uint64_t size) = 0;
		virtual void Unmap() = 0;
		inline const BufferDescription& GetDescription() const { return m_Description; };
	protected:
		BufferDescription m_Description;
		bool m_Mapped;
		unsigned char* m_MappedData;
		IBuffer(const BufferDescription& description) :
			m_Description(description),
			m_Mapped(false),
			m_MappedData(nullptr) {};
	};
}
#endif