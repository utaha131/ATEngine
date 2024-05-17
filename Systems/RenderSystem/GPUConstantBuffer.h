#ifndef _AT_RENDER_SYSTEM_GPU_CONSTANT_BUFFER_H_
#define _AT_RENDER_SYSTEM_GPU_CONSTANT_BUFFER_H_
#include "../RHI/RHI.h"

namespace AT {
	class GPUConstantBuffer {
	public:
		GPUConstantBuffer() {

		}

		GPUConstantBuffer(RHI::Device device, RHI::Buffer buffer) :
			m_device(device),
			m_buffer(buffer)
		{
			buffer->Map();
			RHI::ConstantBufferViewDescription cbv_description = {};
			cbv_description.Buffer = m_buffer;
			cbv_description.Size = buffer->GetDescription().Size;//(sizeof(T) + 255) & ~255;
			m_device->CreateConstantBufferView(cbv_description, m_cbv);
		}

		GPUConstantBuffer(const GPUConstantBuffer& other) {
			m_device = other.m_device;
			m_buffer = other.m_buffer;
			m_cbv = other.m_cbv;
		}

		~GPUConstantBuffer() {
			
		}

		void UnMap() {
			m_buffer->Unmap();
		}

		const RHI::ConstantBufferView& GetNative() const {
			return m_cbv;
		}

		uint64_t Size() {
			return m_buffer->GetDescription().Size;
		}

		template<typename T> void WriteData(T& data) {
			m_buffer->CopyData(0, static_cast<void*>(&data), sizeof(T));
		}
	private:
		uint32_t Alignment() {
			//DX12 256
		}
		RHI::Buffer m_buffer;
		RHI::ConstantBufferView m_cbv;
		RHI::Device m_device;
	};
}
#endif