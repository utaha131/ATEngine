#ifndef _AT_RENDER_SYSTEM_GPU_CONSTANT_BUFFER_H_
#define _AT_RENDER_SYSTEM_GPU_CONSTANT_BUFFER_H_
#include "../RHI/RHI.h"

namespace AT {
	class GPUConstantBuffer {
	public:
		GPUConstantBuffer() {

		}

		GPUConstantBuffer(RHI::Device device, RHI::Buffer buffer) :
			m_Device(device),
			m_Buffer(buffer)
		{
			buffer->Map();
			RHI::ConstantBufferViewDescription cbv_description = {};
			cbv_description.Buffer = m_Buffer;
			cbv_description.Size = buffer->GetDescription().Size;//(sizeof(T) + 255) & ~255;
			m_Device->CreateConstantBufferView(cbv_description, m_CBV);
		}

		GPUConstantBuffer(const GPUConstantBuffer& other) {
			m_Device = other.m_Device;
			m_Buffer = other.m_Buffer;
			m_CBV = other.m_CBV;
		}

		~GPUConstantBuffer() {
			
		}

		void UnMap() {
			m_Buffer->Unmap();
		}

		const RHI::ConstantBufferView& GetNative() const {
			return m_CBV;
		}

		uint64_t Size() {
			return m_Buffer->GetDescription().Size;
		}

		template<typename T> void WriteData(T& data) {
			m_Buffer->CopyData(0, static_cast<void*>(&data), sizeof(T));
		}
	private:
		uint32_t Alignment() {
			//DX12 256
		}
		RHI::Buffer m_Buffer;
		RHI::ConstantBufferView m_CBV;
		RHI::Device m_Device;
	};
}
#endif