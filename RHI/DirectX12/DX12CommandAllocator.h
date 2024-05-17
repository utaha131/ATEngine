#ifndef _RHI_DX12_COMMAND_ALLOCATOR_H_
#define _RHI_DX12_COMMAND_ALLOCATOR_H_
#include "../ICommandAllocator.h"
#include "./Headers/dx12.h"

namespace RHI::DX12 {
	class DX12CommandAllocator : public RHI::ICommandAllocator {
	public:
		DX12CommandAllocator(RHI::CommandType command_type, ID3D12CommandAllocator* dx12_command_allocator);
		~DX12CommandAllocator() override;

		//TODO maybe remove
		ID3D12CommandAllocator* GetNative() const { return m_DX12CommandAllocator; };

		void Reset() override;
	private:
		ID3D12CommandAllocator* m_DX12CommandAllocator;
	};
}
#endif