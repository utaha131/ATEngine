#ifndef _RHI_DX12_DEVICE_H_
#define _RHI_DX12_DEVICE_H_
#include "../IDevice.h"

#include "./Headers/dx12.h"
#include "DX12Graphics.h"
#include "DX12ResourceHeap.h"
#include "DX12Buffer.h"
#include "DX12Texture.h"
#include "DX12RootSignature.h"
#include "DX12Shader.h"
#include "DX12PipelineState.h"
#include "DX12ConstantBufferView.h"
#include "DX12DepthStencilView.h"
#include "DX12RenderTargetView.h"
#include "DX12Sampler.h"
#include "DX12ShaderResourceView.h"
#include "DX12UnorderedAccessView.h"
#include "DX12CommandAllocator.h"
#include "DX12CommandList.h"
#include <vector>
#include "DX12ViewAllocator.h"
#include "DX12DescriptorHeap.h"
#include "DX12Fence.h"

//RT
#include "RayTracing.h"

namespace RHI::DX12 {
	class DX12Device : public RHI::IDevice {
		friend class DX12RenderBackend;
	public:
		DX12Device(ID3D12Device5* dx12_device);
		~DX12Device() override;
		//Object Creation Interface.
		RHI::Result CreateResourceHeap(const RHI::ResourceHeapDescription& description, RHI::ResourceHeap& resource_heap) const override;
		RHI::Result CreateBuffer(RHI::ResourceHeap resource_heap, uint64_t offset, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const override;
		RHI::Result CreateCommittedBuffer(RHI::ResourceHeapType resource_heap_type, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const override;
		RHI::Result CreateTexture(RHI::ResourceHeap resource_heap, uint64_t offset, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const override;
		RHI::Result CreateCommittedTexture(RHI::ResourceHeapType resource_heap_type, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const override;
		RHI::Result CreateRootSignature(const RHI::RootSignatureDescription& description, RHI::RootSignature& root_signature) override;
		RHI::Result CreateShader(const RHI::ShaderDescription& description, RHI::Shader& shader) const override;
		RHI::Result CreateRenderPass(const RHI::RenderPassDescription& description, RHI::RenderPass& render_pass) const override;
		RHI::Result CreateFence(uint64_t initial_value, RHI::Fence& fence) const override;
		RHI::Result CreateFrameBuffer(const RHI::FrameBufferDescription& description, RHI::FrameBuffer& frame_buffer) const override;
		RHI::Result CreateGraphicsPipelineState(const RHI::GraphicsPipelineStateDescription& description, RHI::PipelineState& pipeline_state) const override;
		RHI::Result CreateComputePipelineState(const RHI::ComputePipelineStateDescription& description, RHI::PipelineState& pipeline_state) const override;
		RHI::Result CreateConstantBufferView(const RHI::ConstantBufferViewDescription& description, RHI::ConstantBufferView& constant_buffer_view) override;
		RHI::Result CreateDepthStencilView(const RHI::Texture texture, const RHI::DepthStencilViewDescription& description, RHI::DepthStencilView& depth_stencil_view) override;
		RHI::Result CreateRenderTargetView(const RHI::Buffer buffer, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) override;
		RHI::Result CreateRenderTargetView(const RHI::Texture texture, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) override;
		RHI::Result CreateSampler(const RHI::SamplerDescription& description, RHI::Sampler& sampler) override;
		RHI::Result CreateShaderResourceView(const RHI::Buffer buffer, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) override;
		RHI::Result CreateShaderResourceView(const RHI::Texture texture, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) override;
		RHI::Result CreateUnorderedAccessView(const RHI::Buffer buffer, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) override;
		RHI::Result CreateUnorderedAccessView(const RHI::Texture texture, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) override;
		RHI::Result CreateCommandAllocator(RHI::CommandType command_type, RHI::CommandAllocator& command_allocator) const override;
		RHI::Result CreateCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, RHI::CommandList& command_list) const override;

		//Execution Interface.
		void ExecuteCommandList(RHI::CommandType command_type, uint32_t command_list_count, const RHI::CommandList* p_command_lists) override;
		void SignalQueue(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) override;
		void HostWait(RHI::Fence fence, uint64_t fence_value) override;
		void QueueWait(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) override;

		//Other Interfaces.
		RHI::AllocationInfo GetResourceAllocationInfo(const RHI::BufferDescription& buffer_description) const override;
		RHI::AllocationInfo GetResourceAllocationInfo(const RHI::TextureDescription& texture_description) const override;
		uint32_t GetTexturePitchAlignment() const override;

		void AllocateDescriptorTable(const RHI::RootSignature root_signature, uint32_t parameter_index, RHI::DescriptorTable& table) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ConstantBufferView* p_cbv) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::Sampler* p_sampler) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ShaderResourceView* p_srv) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::UnorderedAccessView* p_uav) override;

		//RT
		void CreateRayTracingPipelineState(const RayTracingPipelineStateDescription& description, IRayTracingPipeline*& pipeline) const override {
			std::vector<D3D12_STATE_SUBOBJECT> dx12_subobjects;

			//Create Shader Libraries.
			std::vector<std::vector<D3D12_EXPORT_DESC>> dx12_export_descs = std::vector<std::vector<D3D12_EXPORT_DESC>>(description.ShaderLibraries.size());
			std::vector<std::wstring*> export_names = std::vector<std::wstring*>();
			for (uint32_t i = 0u; i < description.ShaderLibraries.size(); ++i) {
				dx12_export_descs[i] = std::vector<D3D12_EXPORT_DESC>(description.ShaderLibraries[i].m_ExportNames.size());
				for (uint32_t j = 0u; j < description.ShaderLibraries[i].m_ExportNames.size(); ++j) {
					dx12_export_descs[i][j].Name = export_names.emplace_back(new std::wstring(description.ShaderLibraries[i].m_ExportNames[j].begin(), description.ShaderLibraries[i].m_ExportNames[j].end()))->c_str();
					dx12_export_descs[i][j].ExportToRename = dx12_export_descs[i][j].Name;
					dx12_export_descs[i][j].Flags = D3D12_EXPORT_FLAG_NONE;
				}
				D3D12_DXIL_LIBRARY_DESC dx12_library_desc;
				dx12_library_desc.DXILLibrary = static_cast<DX12Shader*>(description.ShaderLibraries[i].m_Shader)->GetByteCode();
				dx12_library_desc.NumExports = dx12_export_descs[i].size();
				dx12_library_desc.pExports = dx12_export_descs[i].data();

				D3D12_STATE_SUBOBJECT dx12_subobject;
				dx12_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
				dx12_subobject.pDesc = &dx12_library_desc;
				dx12_subobjects.emplace_back(dx12_subobject);
			}
			
			//Create Hit Groups.
			std::vector<D3D12_HIT_GROUP_DESC> dx12_hit_group_descs = std::vector<D3D12_HIT_GROUP_DESC>(description.HitGroups.size());
			std::vector<std::wstring*> import_names;
			for (uint32_t i = 0u; i < description.HitGroups.size(); ++i) {
				dx12_hit_group_descs[i].HitGroupExport = export_names.emplace_back(new std::wstring(description.HitGroups[i].Name.begin(), description.HitGroups[i].Name.end()))->c_str();
				switch (description.HitGroups[i].Type) {
				case RayTracingHitGroupType::TRIANGLES:
					dx12_hit_group_descs[i].Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
					break;
				case RayTracingHitGroupType::PROCEDURAL_PRIMITIVE:
					dx12_hit_group_descs[i].Type = D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;
					break;
				}

				if (description.HitGroups[i].AnyHitShaderImport != "") {
					dx12_hit_group_descs[i].AnyHitShaderImport = import_names.emplace_back(new std::wstring(description.HitGroups[i].AnyHitShaderImport.begin(), description.HitGroups[i].AnyHitShaderImport.end()))->c_str();
				}
				if (description.HitGroups[i].ClosestHitShaderImport != "") {
					dx12_hit_group_descs[i].ClosestHitShaderImport = import_names.emplace_back(new std::wstring(description.HitGroups[i].ClosestHitShaderImport.begin(), description.HitGroups[i].ClosestHitShaderImport.end()))->c_str();
				}
				if (description.HitGroups[i].IntersectionShaderImport != "") {
					dx12_hit_group_descs[i].IntersectionShaderImport = import_names.emplace_back(new std::wstring(description.HitGroups[i].IntersectionShaderImport.begin(), description.HitGroups[i].IntersectionShaderImport.end()))->c_str();
				}

				D3D12_STATE_SUBOBJECT dx12_subobject;
				dx12_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
				dx12_subobject.pDesc = &dx12_hit_group_descs[i];
				dx12_subobjects.emplace_back(dx12_subobject);
			}
			
			//Create Shader Configuration.
			uint32_t shader_config_index = dx12_subobjects.size();
			D3D12_RAYTRACING_SHADER_CONFIG dx12_shader_config;
			dx12_shader_config.MaxAttributeSizeInBytes = description.ShaderConfiguration.MaxAttributeSizeInBytes;
			dx12_shader_config.MaxPayloadSizeInBytes = description.ShaderConfiguration.MaxPayloadSizeInBytes;
			{
				D3D12_STATE_SUBOBJECT dx12_subobject;
				dx12_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
				dx12_subobject.pDesc = &dx12_shader_config;
				dx12_subobjects.emplace_back(dx12_subobject);
			}

			//Create Global Root Signature.
			if (description.GlobalRootSignature != RHI_NULL_HANDLE) {
				D3D12_GLOBAL_ROOT_SIGNATURE dx12_global_root_signature;
				dx12_global_root_signature.pGlobalRootSignature = static_cast<DX12RootSignature*>(description.GlobalRootSignature)->GetNative();
				{
					D3D12_STATE_SUBOBJECT dx12_subobject;
					dx12_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
					dx12_subobject.pDesc = &dx12_global_root_signature;
					dx12_subobjects.emplace_back(dx12_subobject);
				}
			}

			std::vector<D3D12_LOCAL_ROOT_SIGNATURE> dx12_local_root_signatures = std::vector<D3D12_LOCAL_ROOT_SIGNATURE>(description.LocalRootSignatures.size());
			//Create Local Root Signatures.
			uint32_t local_root_signature_start_index = dx12_subobjects.size();
			for (uint32_t i = 0u; i < description.LocalRootSignatures.size(); ++i) {
				dx12_local_root_signatures[i].pLocalRootSignature = static_cast<DX12RootSignature*>(description.LocalRootSignatures[i])->GetNative();
				D3D12_STATE_SUBOBJECT dx12_subobject;
				dx12_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
				dx12_subobject.pDesc = &dx12_local_root_signatures[i];
			}

			//Create Associations.
			std::vector<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION> dx12_subobject_to_exports_associations = std::vector<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION>(description.ExportAssociations.size());
			std::vector<std::wstring> association_export_names = std::vector<std::wstring>();
			std::vector<std::vector<LPCWSTR>> export_names_for_associations = std::vector<std::vector<LPCWSTR>>(description.ExportAssociations.size());
			for (uint32_t i = 0u; i < description.ExportAssociations.size(); ++i) {
				export_names_for_associations[i] = std::vector<LPCWSTR>(description.ExportAssociations[i].ExportNames.size());
				for (uint32_t j = 0u; j < description.ExportAssociations[i].ExportNames.size(); ++j) {
					export_names_for_associations[i][j] = association_export_names.emplace_back(std::wstring(description.ExportAssociations[i].ExportNames[i].begin(), description.ExportAssociations[i].ExportNames[i].end())).c_str();
				}
				dx12_subobject_to_exports_associations[i].NumExports = export_names_for_associations[i].size();
				dx12_subobject_to_exports_associations[i].pExports = export_names_for_associations[i].data();
				switch (description.ExportAssociations[i].Type) {
				case RayTracingExportAssociationType::SHADER_PAYLOAD:
					dx12_subobject_to_exports_associations[i].pSubobjectToAssociate = &dx12_subobjects[shader_config_index];
					break;
				case RayTracingExportAssociationType::LOCAL_ROOT_SIGNATURE:
					dx12_subobject_to_exports_associations[i].pSubobjectToAssociate = &dx12_subobjects[local_root_signature_start_index + description.ExportAssociations[i].AssociationObjectIndex];
					break;
				}
			}

			D3D12_RAYTRACING_PIPELINE_CONFIG dx12_ray_tracing_pipeline_config;
			dx12_ray_tracing_pipeline_config.MaxTraceRecursionDepth = description.MaxTraceRecursionDepth;
			{
				D3D12_STATE_SUBOBJECT dx12_subobject;
				dx12_subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
				dx12_subobject.pDesc = &dx12_ray_tracing_pipeline_config;
				dx12_subobjects.emplace_back(dx12_subobject);
			}

			D3D12_STATE_OBJECT_DESC dx12_state_object_desc;
			dx12_state_object_desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
			dx12_state_object_desc.NumSubobjects = dx12_subobjects.size();
			dx12_state_object_desc.pSubobjects = dx12_subobjects.data();
			ID3D12StateObject* dx12_state_object;
			m_DX12Device->CreateStateObject(&dx12_state_object_desc, IID_PPV_ARGS(&dx12_state_object));
			pipeline = new DX12RayTracingPipeline(dx12_state_object);
			for (uint32_t i = 0u; i < export_names.size(); ++i) {
				delete export_names[i];
			}
		}

		void CreateRayTracingBottomLevelAccelerationStructure(const RHI::Buffer buffer, IRayTracingBottomLevelAccelerationStructure*& bottom_level_acceleration_structure) const override {
			bottom_level_acceleration_structure = new DX12RayTracingBottomLevelAccelerationStructure(buffer);
		}

		void CreateRayTracingTopLevelAccelerationStructure(const RHI::Buffer buffer, IRayTracingTopLevelAccelerationStructure*& top_level_acceleration_structure) const override {
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_UNKNOWN;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.RaytracingAccelerationStructure.Location = static_cast<RHI::DX12::DX12Buffer*>(buffer)->GetNative()->GetGPUVirtualAddress();
			D3D12_CPU_DESCRIPTOR_HANDLE dx12_srv = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->AllocateHandle();
			m_DX12Device->CreateShaderResourceView(nullptr, &srvDesc, dx12_srv);
			top_level_acceleration_structure = new DX12RayTracingTopLevelAccelerationStructure(buffer, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], dx12_srv);
		}

		void CreateRayTracingInstanceBuffer(RHI::Buffer buffer, uint64_t capacity, IRayTracingInstanceBuffer*& instance_buffer) const override {
			instance_buffer = new DX12RayTracingInstanceBuffer(buffer, capacity);
		}

		void GetRayTracingBottomLevelAccelerationStructureMemoryInfo(const RayTracingBottomLevelAccelerationStructureDescription bottom_level_acceleration_structure_description, RayTracingAccelerationStructureMemoryInfo& memory_info) const override {
			std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> dx12_geometry_descriptions = std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>(bottom_level_acceleration_structure_description.Geometries.size());

			for (uint32_t i = 0u; i < bottom_level_acceleration_structure_description.Geometries.size(); ++i) {
				dx12_geometry_descriptions[i].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

				dx12_geometry_descriptions[i].Triangles.VertexBuffer.StartAddress = static_cast<DX12Buffer*>(bottom_level_acceleration_structure_description.Geometries[i].VertexBuffer.Buffer)->GetNative()->GetGPUVirtualAddress();
				dx12_geometry_descriptions[i].Triangles.VertexBuffer.StrideInBytes = bottom_level_acceleration_structure_description.Geometries[i].VertexBuffer.Stride;
				dx12_geometry_descriptions[i].Triangles.VertexFormat = DX12ConvertFormat(bottom_level_acceleration_structure_description.Geometries[i].VertexBuffer.Format);
				dx12_geometry_descriptions[i].Triangles.VertexCount = bottom_level_acceleration_structure_description.Geometries[i].VertexBuffer.VertexCount;
				
				dx12_geometry_descriptions[i].Triangles.IndexBuffer = static_cast<DX12Buffer*>(bottom_level_acceleration_structure_description.Geometries[i].IndexBuffer.Buffer)->GetNative()->GetGPUVirtualAddress();
				dx12_geometry_descriptions[i].Triangles.IndexFormat = DX12ConvertFormat(bottom_level_acceleration_structure_description.Geometries[i].IndexBuffer.Format);
				dx12_geometry_descriptions[i].Triangles.IndexCount = bottom_level_acceleration_structure_description.Geometries[i].IndexBuffer.IndexCount;

				dx12_geometry_descriptions[i].Triangles.Transform3x4 = 0;

				dx12_geometry_descriptions[i].Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			}

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS dx12_ray_tracing_acceleration_structure_inputs = {};
			dx12_ray_tracing_acceleration_structure_inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			dx12_ray_tracing_acceleration_structure_inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			dx12_ray_tracing_acceleration_structure_inputs.pGeometryDescs = dx12_geometry_descriptions.data();
			dx12_ray_tracing_acceleration_structure_inputs.NumDescs = dx12_geometry_descriptions.size();
			dx12_ray_tracing_acceleration_structure_inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO dx12_prebuild_info = {};
			m_DX12Device->GetRaytracingAccelerationStructurePrebuildInfo(&dx12_ray_tracing_acceleration_structure_inputs, &dx12_prebuild_info);
			memory_info.ScratchDataSize = dx12_prebuild_info.ScratchDataSizeInBytes;
			memory_info.DestinationDataSize = dx12_prebuild_info.ResultDataMaxSizeInBytes;
		}

		void GetRayTracingTopLevelAccelerationStructureMemoryInfo(const RayTracingTopLevelAccelerationStructureDescription top_level_acceleration_structure_description, RayTracingAccelerationStructureMemoryInfo& memory_info) const override {
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS dx12_ray_tracing_acceleration_structure_inputs = {};
			dx12_ray_tracing_acceleration_structure_inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			dx12_ray_tracing_acceleration_structure_inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			dx12_ray_tracing_acceleration_structure_inputs.InstanceDescs = static_cast<RHI::DX12::DX12Buffer*>(top_level_acceleration_structure_description.InstancesBuffer->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			dx12_ray_tracing_acceleration_structure_inputs.NumDescs = top_level_acceleration_structure_description.InstanceCount;
			dx12_ray_tracing_acceleration_structure_inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO dx12_prebuild_info = {};
			m_DX12Device->GetRaytracingAccelerationStructurePrebuildInfo(&dx12_ray_tracing_acceleration_structure_inputs, &dx12_prebuild_info);
			memory_info.ScratchDataSize = dx12_prebuild_info.ScratchDataSizeInBytes;
			memory_info.DestinationDataSize = dx12_prebuild_info.ResultDataMaxSizeInBytes;
		}

		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, IRayTracingTopLevelAccelerationStructure** p_top_level_acceleration_structures) override {
			CD3DX12_CPU_DESCRIPTOR_HANDLE dx12_destination_cpu_handle{ static_cast<DX12DescriptorTable*>(descriptor_table)->GetCPUHandle() };
			dx12_destination_cpu_handle.Offset(offset, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			for (uint32_t i = 0u; i < count; ++i) {
				m_DX12Device->CopyDescriptorsSimple(1u, dx12_destination_cpu_handle, static_cast<const DX12RayTracingTopLevelAccelerationStructure*>(p_top_level_acceleration_structures[i])->GetNative(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				dx12_destination_cpu_handle.Offset(1, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			}
		}

		ID3D12Device5* m_DX12Device;
	//private:
		
		ID3D12CommandQueue* m_DX12CommandQueues[(uint32_t)RHI::CommandType::NUM_TYPES];

		DX12DescriptorHeap* m_GPUDescriptorHeap;
		DX12DescriptorHeap* m_GPUSamplerDescriptorHeap;
		DX12ViewAllocator* m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

		Microsoft::WRL::ComPtr<IDxcLibrary> m_DXLibrary;
		Microsoft::WRL::ComPtr<IDxcCompiler> m_DXCompiler;
		Microsoft::WRL::ComPtr<IDxcUtils> m_DXUtils;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> m_DXIncludeHandler;
	};
}
#endif