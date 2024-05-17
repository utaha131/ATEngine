#ifndef _RHI_DX12_CONSTANT_BUFFER_VIEW_H_
#define _RHI_DX12_CONSTANT_BUFFER_VIEW_H_
#include "../IConstantBufferView.h"
#include "./Headers/dx12.h"
#include "DX12ViewAllocator.h"
#include "DX12Buffer.h"

namespace RHI::DX12 {
	class DX12ConstantBufferView : public RHI::IConstantBufferView {
	public:
		DX12ConstantBufferView(const RHI::ConstantBufferViewDescription& description, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle) :
			RHI::IConstantBufferView(description),
			m_ViewAllocator(view_allocator),
			m_DX12Handle(dx12_handle)
		{

		}

		~DX12ConstantBufferView() override {
			m_ViewAllocator.FreeHandle(m_DX12Handle);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE GetNative() { return m_DX12Handle; }
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DX12Handle;
	};

	inline constexpr D3D12_CONSTANT_BUFFER_VIEW_DESC DX12ConvertConstantBufferViewDescription(const RHI::ConstantBufferViewDescription& description) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC dx12_constant_buffer_view_description;
		dx12_constant_buffer_view_description.BufferLocation = static_cast<DX12Buffer*>(description.Buffer)->GetNative()->GetGPUVirtualAddress();
		dx12_constant_buffer_view_description.SizeInBytes = description.Size;
		return dx12_constant_buffer_view_description;
	}
}
#endif