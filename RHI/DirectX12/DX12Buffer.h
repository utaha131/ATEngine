#ifndef _RHI_DX12_BUFFER_H_
#define _RHI_DX12_BUFFER_H_
#include "../IBuffer.h"
#include "./Headers/dx12.h"

namespace RHI::DX12 {
	class DX12Buffer : public RHI::IBuffer {
	public:
		DX12Buffer(const BufferDescription& description, ID3D12Resource* dx12_resource);
		~DX12Buffer() override;

		void Map() override;
		void CopyData(uint64_t offset, const void* data, uint64_t size) override;
		void Unmap() override;

		inline ID3D12Resource* GetNative() const { return m_DX12Resource; };

	private:
		ID3D12Resource* m_DX12Resource;
	};

	inline constexpr D3D12_RESOURCE_STATES DX12ConvertBufferState(BufferState buffer_state) {
		switch (buffer_state) {
		case BufferState::COMMON:
			return D3D12_RESOURCE_STATE_COMMON;

		case BufferState::UNORDERED_ACCESS:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

		case BufferState::COPY_DEST:
			return D3D12_RESOURCE_STATE_COPY_DEST;

		case BufferState::COPY_SOURCE:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
#undef GENERIC_READ
		case BufferState::GENERIC_READ:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
			//Buffer Exclusives.
		case BufferState::VERTEX_BUFFER:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		case BufferState::CONSTANT_BUFFER:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		case BufferState::INDEX_BUFFER:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;

		case BufferState::NON_PIXEL_SHADER_RESOURCE:
			return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

		case BufferState::PIXEL_SHADER_RESOURCE:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;



		case BufferState::RAYTRACING_ACCELERATION_STRUCTURE:
			return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
		}
	}
	inline D3D12_RESOURCE_DESC DX12ConvertBufferDescription(const BufferDescription& description) {
		CD3DX12_RESOURCE_DESC dx12_buffer_description = CD3DX12_RESOURCE_DESC::Buffer(description.Size);
		if (has_flag(description.UsageFlags, BufferUsageFlag::UNORDERED_ACCESS)) {
			dx12_buffer_description.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}
		return dx12_buffer_description;
	}
}
#endif