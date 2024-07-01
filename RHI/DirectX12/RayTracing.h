#ifndef _RHI_DX12_RAYTRACING_H_
#define _RHI_DX12_RAYTRACING_H_
#include "../RHICore.h"
#include "./Headers/dx12.h"
#include "DX12ViewAllocator.h"

namespace RHI::DX12 {
	class DX12RayTracingPipeline : public IRayTracingPipeline {
	public:
		DX12RayTracingPipeline(ID3D12StateObject* dx12_state_object) :
			m_DX12StateObject(dx12_state_object)
		{
			m_DX12StateObject->QueryInterface(IID_PPV_ARGS(&m_DX12StateObjectProperties));
		}

		ID3D12StateObject* GetNative() const {
			return m_DX12StateObject;
		}

		void* GetShaderIdentifier(const std::string& name) const override {
			return m_DX12StateObjectProperties->GetShaderIdentifier(std::wstring(name.begin(), name.end()).c_str());
		}
	private:
		ID3D12StateObject* m_DX12StateObject;
		ID3D12StateObjectProperties* m_DX12StateObjectProperties;
	};

	class DX12RayTracingBottomLevelAccelerationStructure : public IRayTracingBottomLevelAccelerationStructure {
	public:
		DX12RayTracingBottomLevelAccelerationStructure(RHI::Buffer buffer) : IRayTracingBottomLevelAccelerationStructure(buffer) {}
	private:
		
	};

	class DX12RayTracingTopLevelAccelerationStructure : public IRayTracingTopLevelAccelerationStructure {
	public:
		DX12RayTracingTopLevelAccelerationStructure(RHI::Buffer buffer, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE cpu_descriptor_handle) : IRayTracingTopLevelAccelerationStructure(buffer),
			m_ViewAllocator(view_allocator),
			m_CPUDescriptorHandle(cpu_descriptor_handle)
		{
			
		}

		~DX12RayTracingTopLevelAccelerationStructure() override {
			m_ViewAllocator.FreeHandle(m_CPUDescriptorHandle);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetNative() const {
			return m_CPUDescriptorHandle;
		}
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptorHandle;
	};

	class DX12RayTracingInstanceBuffer : public IRayTracingInstanceBuffer {
	public:
		DX12RayTracingInstanceBuffer (RHI::Buffer buffer, uint64_t capacity) : IRayTracingInstanceBuffer(buffer, capacity)
		{

		}

		void WriteInstance(uint64_t offset, RayTracingInstance& instance) {
			D3D12_RAYTRACING_INSTANCE_DESC dx12_ray_tracing_instance_desc = {};
			//dx12_ray_tracing_instance_desc.Transform[0][0] = dx12_ray_tracing_instance_desc.Transform[1][1] = dx12_ray_tracing_instance_desc.Transform[2][2] = 1.0f;
			
			for (uint32_t j = 0; j < 3; ++j) {
				for (uint32_t k = 0; k < 4; ++k) {
					dx12_ray_tracing_instance_desc.Transform[j][k] = instance.Transform[j][k];
				}
			}
			dx12_ray_tracing_instance_desc.InstanceID = instance.InstanceID;
			dx12_ray_tracing_instance_desc.InstanceMask = 0xFF;
			dx12_ray_tracing_instance_desc.InstanceContributionToHitGroupIndex = instance.InstanceContributionToHitGroupIndex;
			dx12_ray_tracing_instance_desc.AccelerationStructure = static_cast<DX12Buffer*>(instance.BottomLevelAccelerationStructure->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			dx12_ray_tracing_instance_desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			m_Buffer->CopyData(offset * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &dx12_ray_tracing_instance_desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
		}

		void WriteInstances(uint64_t offset, std::vector<RayTracingInstance>& instances) {
			std::vector<D3D12_RAYTRACING_INSTANCE_DESC> dx12_ray_tracing_instance_descs = std::vector<D3D12_RAYTRACING_INSTANCE_DESC>(instances.size());
		}
	};
}
#endif