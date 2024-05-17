#include "DX12ResourceHeap.h"

namespace RHI::DX12 {
	DX12ResourceHeap::DX12ResourceHeap(const RHI::ResourceHeapDescription& description, ID3D12Heap* dx12_resource_heap) :
		RHI::IResourceHeap(description),
		m_DX12ResourceHeap(dx12_resource_heap)
	{

	}

	DX12ResourceHeap::~DX12ResourceHeap() {
		m_DX12ResourceHeap->Release();
	}
}