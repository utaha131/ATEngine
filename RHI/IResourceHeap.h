#ifndef _RHI_I_RESOURCE_HEAP_H_
#define _RHI_I_RESOURCE_HEAP_H_
#include "RHICore.h"

namespace RHI {
	enum class ResourceHeapType{
		DEFAULT,
		UPLOAD,
		READ_BACK
	};

	typedef struct ResourceHeapDescription {
		ResourceHeapType  ResourceHeapType;
		uint64_t Size;
	} ResourceheapDescription;

	class IResourceHeap {
	public:
		virtual ~IResourceHeap() = default;
		const ResourceHeapDescription& GetDescription() const { return m_Description; }
	protected:
		ResourceHeapDescription m_Description;
		IResourceHeap(const ResourceHeapDescription& description) :
			m_Description(description)
		{}
	};
}
#endif