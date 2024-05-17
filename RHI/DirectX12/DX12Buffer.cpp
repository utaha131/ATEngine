#include "DX12Buffer.h"

namespace RHI::DX12 {
	DX12Buffer::DX12Buffer(const RHI::BufferDescription& description, ID3D12Resource* dx12_resource) :
		RHI::IBuffer(description),
		m_DX12Resource(dx12_resource)
	{

	}

	DX12Buffer::~DX12Buffer() {
		Unmap();
		m_DX12Resource->Release();
	}

	void DX12Buffer::Map() {
		if (!m_Mapped) {
			HRESULT result = m_DX12Resource->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData));
			assert(!FAILED(result), "Buffer Map Failed.");
			m_Mapped = true;
		}
	}

	void DX12Buffer::CopyData(uint64_t offset, const void* data, uint64_t size) {
		if (m_Mapped) {
			memcpy(&m_MappedData[offset], data, size);
		}
	}

	void DX12Buffer::Unmap() {
		if (m_Mapped) {
			m_DX12Resource->Unmap(0, nullptr);
			m_Mapped = false;
			m_MappedData = nullptr;
		}
	}
}