#include "DX12CommandAllocator.h"

namespace RHI::DX12 {
	DX12CommandAllocator::DX12CommandAllocator(RHI::CommandType command_type, ID3D12CommandAllocator* dx12_command_allocator) :
		RHI::ICommandAllocator(command_type),
		m_DX12CommandAllocator(dx12_command_allocator)
	{

	}

	DX12CommandAllocator::~DX12CommandAllocator() {
		m_DX12CommandAllocator->Release();
	}

	void DX12CommandAllocator::Reset() {
		m_DX12CommandAllocator->Reset();
	}
}