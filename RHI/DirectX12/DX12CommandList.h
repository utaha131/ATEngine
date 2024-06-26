#ifndef _RHI_DX12_COMMAND_LIST_H_
#define _RHI_DX12_COMMAND_LIST_H_
#include <math.h>
#include <array>
#include "../RHICore.h"
#include "../ICommandList.h"
#include "./Headers/dx12.h"
#include "DX12CommandAllocator.h"
#include "DX12RootSignature.h"
#include "DX12PipelineState.h"
#include "DX12Buffer.h"
#include "DX12Texture.h"
#include "DX12FrameBuffer.h"
#include "DX12DepthStencilView.h"
#include "DX12RenderTargetView.h"
#include "DX12Graphics.h"
#include "DX12DescriptorHeap.h"

#include "RayTracing.h"

namespace RHI::DX12 {
	class DX12CommandList : public RHI::ICommandList {
	public:
		DX12CommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, ID3D12GraphicsCommandList4* dx12_command_list, std::array<ID3D12DescriptorHeap*, 2> dx12_descriptor_heaps);
		~DX12CommandList() override;

		void Reset(const RHI::PipelineState pipeline_state) override;
		void Close() override;
		void ResourceBarrier(uint32_t resource_barrier_count, const RHI::ResourceBarrier* p_resource_barrier) override;
		void CopyBuffer(RHI::Buffer destination_buffer, RHI::Buffer source_buffer) override;
		void CopyBufferRegion(RHI::Buffer destination_buffer, uint64_t destination_offset, RHI::Buffer source_buffer, uint64_t source_offset, uint64_t size) override;
		void CopyBufferToTexture(RHI::TextureCopyLocation& dest_texture_region, RHI::BufferTextureCopyLocation& source_buffer_region) override;
		void CopyTexture(RHI::Texture destination_texture, RHI::Texture source_texture) override;
		void CopyTextureRegion(RHI::TextureCopyLocation& dest_texture_region, RHI::TextureCopyLocation& source_texture_region) override;
		void SetGraphicsRootSignature(const RHI::RootSignature root_signature) override;
		void SetComputeRootSignature(const RHI::RootSignature root_signature) override;
		void SetPipelineState(const RHI::PipelineState pipeline_state) override;
		void SetVertexBuffer(uint32_t start_slot, uint32_t num_views, const RHI::VertexBufferView* p_vertex_buffer_views) override;
		void SetIndexBuffer(const RHI::IndexBufferView& index_buffer_view) override;
		void SetViewport(const RHI::Viewport& viewport) override;
		void SetScissorRect(const RHI::Rect& scissor_rect) override;
		void SetRootConstants(uint32_t root_parameter_index, uint32_t constants_count, const void* p_data, uint32_t offset) override;
		void SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, const RHI::DescriptorTable table) override;
		void SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, RHI::ConstantBufferView constant_buffer_View) override;
		void SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, RHI::ShaderResourceView shader_resource_view) override;
		void SetGraphicsRootUnorderedAccessView(uint32_t root_parameter_index, RHI::UnorderedAccessView unordered_access_view) override;
		void SetComputeRootDescriptorTable(uint32_t root_parameter_index, const RHI::DescriptorTable table) override;
		void SetPrimitiveTopology(RHI::PrimitiveTopology primitive_topology) override;
		void Draw(uint32_t vertices, uint32_t instances, uint32_t start_vertex, uint32_t start_instance) override;
		void DrawIndexed(uint32_t indices, uint32_t instance_count, uint32_t start_index, uint32_t start_vertex, uint32_t start_instance) override;
		void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;
		void BeginRenderPass(const RHI::BeginRenderPassInfo& begin_render_pass_info) override;
		void EndRenderPass() override;

		ID3D12GraphicsCommandList* GetNative() const { return m_DX12CommandList; };

		//void Reset(const RHI::PipelineState pipeline_state) override;
		//void Close() override;
		//void ResourceBarrier(RHI_UINT32 resource_barrier_count, const RHI_ResourceBarrier* resource_barrier) override;

		//void CopyBuffer(RHI_Buffer destination_buffer, RHI_Buffer source_buffer) override {
		//	m_native->CopyResource(static_cast<DX12Buffer*>(destination_buffer)->GetNative(), static_cast<DX12Buffer*>(source_buffer)->GetNative());
		//}

		//void CopyBufferRegion(RHI_Buffer destination_buffer, RHI_UINT64 destination_offset, RHI_Buffer source_buffer, RHI_UINT64 source_offset, RHI_UINT64 size) override {
		//	m_native->CopyBufferRegion(static_cast<DX12Buffer*>(destination_buffer)->GetNative(), destination_offset, static_cast<DX12Buffer*>(source_buffer)->GetNative(), source_offset, size);
		//}

		//void CopyBufferToTexture(RHI_TextureCopyLocation& dest_texture_region, RHI_BufferTextureCopyLocation& source_buffer_region) override {
		//	m_native->CopyTextureRegion(&DX12ConvertCopyLocation(dest_texture_region), 0, 0, 0, &DX12ConvertCopyLocation(source_buffer_region), nullptr);
		//}

		//void CopyTexture(RHI_Texture destination_texture, RHI_Texture source_texture) override {
		//	m_native->CopyResource(static_cast<DX12Texture*>(destination_texture)->GetNative(), static_cast<DX12Texture*>(source_texture)->GetNative());
		//}

		//void CopyTextureRegion(RHI_TextureCopyLocation& dest_texture_region, RHI_TextureCopyLocation& source_texture_region) override {
		//	m_native->CopyTextureRegion(&DX12ConvertCopyLocation(dest_texture_region), 0, 0, 0, &DX12ConvertCopyLocation(source_texture_region), nullptr);
		//}

		//void SetGraphicsRootSignature(const RHI_RootSignature root_signature) override;
		//void SetPipelineState(const RHI_PipelineState pipeline_state) override;
		//void SetVertexBuffer(RHI_UINT32 start_slot, RHI_UINT32 num_views, const RHI_VertexBufferView* p_vertex_buffer_views) override;
		//void SetIndexBuffer(const RHI_IndexBufferView& index_buffer_view) override;
		//void SetViewport(const RHI_Viewport& viewport) override;
		//void SetScissorRect(const RHI_Rect& scissor_rect) override;
		//void SetRootConstants(RHI_UINT32 root_parameter_index, RHI_UINT32 constants_count, const RHI_DATA p_data, RHI_UINT32 offset) override;
		//void SetGraphicsRootConstantBufferView(RHI_UINT32 root_parameter_index, RHI_ConstantBufferView constant_buffer_View) override;
		//void SetGraphicsRootShaderResourceView(RHI_UINT32 root_parameter_index, RHI_ShaderResourceView shader_resource_view) override;
		//void SetGraphicsRootUnorderedAccessView(RHI_UINT32 root_parameter_index, RHI_UnorderedAccessView unordered_access_view) override;
		//void SetPrimitiveTopology(RHI_PRIMITIVE_TOPOLOGY primitive_topology) override;
		//void Draw(RHI_UINT32 vertices, RHI_UINT32 instances, RHI_UINT32 start_vertex, RHI_UINT32 start_instance) override;
		//void DrawIndexed(RHI_UINT32 indices, RHI_UINT32 instance_count, RHI_UINT32 start_index, RHI_UINT32 start_vertex, RHI_UINT32 start_instance) override;
		//void BeginRenderPass(const RHI_BeginRenderPassInfo& begin_render_pass_info) override;
		//void EndRenderPass() override;

		//void SetComputeRootSignature(const RHI_RootSignature root_signature) {
		//	m_native->SetComputeRootSignature(static_cast<DX12RootSignature*>(root_signature)->GetNative());
		//}

		//void Dispatch(RHI_UINT32 x, RHI_UINT32 y, RHI_UINT32 z) {
		//	m_native->Dispatch(x, y, z);
		//}

		//void SetGraphicsRootDescriptorTable(RHI_UINT32 root_parameter_index, const RHI_DescriptorTable table) override {
		//	//CD3DX12_GPU_DESCRIPTOR_HANDLE handle{ m_gpu_descriptor_heaps[0]->GetGPUDescriptorHandleForHeapStart() };
		//	//m_native->SetGraphicsRootDescriptorTable(index, handle.Offset(table_id, 32));
		//	const DX12DescriptorTable* native_table = static_cast<const DX12DescriptorTable*>(table);
		//	m_native->SetGraphicsRootDescriptorTable(root_parameter_index, native_table->GetGPUHandle());
		//}

		//void SetComputeRootDescriptorTable(RHI_UINT32 root_parameter_index, const RHI_DescriptorTable table) override {
		//	//CD3DX12_GPU_DESCRIPTOR_HANDLE handle{ m_gpu_descriptor_heaps[0]->GetGPUDescriptorHandleForHeapStart() };
		//	//m_native->SetGraphicsRootDescriptorTable(index, handle.Offset(table_id, 32));
		//	const DX12DescriptorTable* native_table = static_cast<const DX12DescriptorTable*>(table);
		//	m_native->SetComputeRootDescriptorTable(root_parameter_index, native_table->GetGPUHandle());
		//}

		void SetRayTracingPipeline(IRayTracingPipeline* pipeline) override {
			m_DX12CommandList->SetPipelineState1(static_cast<DX12RayTracingPipeline*>(pipeline)->GetNative());
		}

		void DispatchRays(const RayTracingDispatchRaysDescription& dispatch_rays_description) override {
			D3D12_DISPATCH_RAYS_DESC dx12_dispatch_rays_desc = {};
			dx12_dispatch_rays_desc.Width = dispatch_rays_description.Width;
			dx12_dispatch_rays_desc.Height = dispatch_rays_description.Height;
			dx12_dispatch_rays_desc.Depth = dispatch_rays_description.Depth;
			//dx12_dispatch_rays_desc.CallableShaderTable;
			dx12_dispatch_rays_desc.RayGenerationShaderRecord.StartAddress = static_cast<RHI::DX12::DX12Buffer*>(dispatch_rays_description.RayGenerationShaderRecord.Buffer)->GetNative()->GetGPUVirtualAddress() + dispatch_rays_description.RayGenerationShaderRecord.Offset;
			dx12_dispatch_rays_desc.RayGenerationShaderRecord.SizeInBytes = dispatch_rays_description.RayGenerationShaderRecord.Size;
			
			dx12_dispatch_rays_desc.HitGroupTable.StartAddress = static_cast<RHI::DX12::DX12Buffer*>(dispatch_rays_description.HitGroupTable.Buffer)->GetNative()->GetGPUVirtualAddress() + dispatch_rays_description.HitGroupTable.Offset;
			dx12_dispatch_rays_desc.HitGroupTable.SizeInBytes = dispatch_rays_description.HitGroupTable.Size;
			dx12_dispatch_rays_desc.HitGroupTable.StrideInBytes = dispatch_rays_description.HitGroupTable.Stride;

			dx12_dispatch_rays_desc.MissShaderTable.StartAddress = static_cast<RHI::DX12::DX12Buffer*>(dispatch_rays_description.MissShaderTable.Buffer)->GetNative()->GetGPUVirtualAddress() + dispatch_rays_description.MissShaderTable.Offset;
			dx12_dispatch_rays_desc.MissShaderTable.SizeInBytes = dispatch_rays_description.MissShaderTable.Size;
			dx12_dispatch_rays_desc.MissShaderTable.StrideInBytes = dispatch_rays_description.MissShaderTable.Stride;
			m_DX12CommandList->DispatchRays(&dx12_dispatch_rays_desc);
		}

		void BuildRayTracingAccelerationStructure(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& desc) {
			m_DX12CommandList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
		}

		void BuildRaytracingBottomLevelAccelerationStructure(const BuildRayTracingBottomLevelAccelerationStructure& build_bottom_level_acceleration_structure) const override {
			std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> dx12_geometry_descriptions = std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>(build_bottom_level_acceleration_structure.description->Geometries.size());

			for (uint32_t i = 0u; i < build_bottom_level_acceleration_structure.description->Geometries.size(); ++i) {
				dx12_geometry_descriptions[i].Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

				dx12_geometry_descriptions[i].Triangles.VertexBuffer.StartAddress = static_cast<DX12Buffer*>(build_bottom_level_acceleration_structure.description->Geometries[i].VertexBuffer.Buffer)->GetNative()->GetGPUVirtualAddress();
				dx12_geometry_descriptions[i].Triangles.VertexBuffer.StrideInBytes = build_bottom_level_acceleration_structure.description->Geometries[i].VertexBuffer.Stride;
				dx12_geometry_descriptions[i].Triangles.VertexFormat = DX12ConvertFormat(build_bottom_level_acceleration_structure.description->Geometries[i].VertexBuffer.Format);
				dx12_geometry_descriptions[i].Triangles.VertexCount = build_bottom_level_acceleration_structure.description->Geometries[i].VertexBuffer.VertexCount;

				dx12_geometry_descriptions[i].Triangles.IndexBuffer = static_cast<DX12Buffer*>(build_bottom_level_acceleration_structure.description->Geometries[i].IndexBuffer.Buffer)->GetNative()->GetGPUVirtualAddress();
				dx12_geometry_descriptions[i].Triangles.IndexFormat = DX12ConvertFormat(build_bottom_level_acceleration_structure.description->Geometries[i].IndexBuffer.Format);
				dx12_geometry_descriptions[i].Triangles.IndexCount = build_bottom_level_acceleration_structure.description->Geometries[i].IndexBuffer.IndexCount;

				dx12_geometry_descriptions[i].Triangles.Transform3x4 = 0;

				dx12_geometry_descriptions[i].Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			}

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS dx12_ray_tracing_acceleration_structure_inputs = {};
			dx12_ray_tracing_acceleration_structure_inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			dx12_ray_tracing_acceleration_structure_inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			dx12_ray_tracing_acceleration_structure_inputs.pGeometryDescs = dx12_geometry_descriptions.data();
			dx12_ray_tracing_acceleration_structure_inputs.NumDescs = dx12_geometry_descriptions.size();
			dx12_ray_tracing_acceleration_structure_inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC dx12_build_ray_tracing_acceleration_structure_description = {};
			dx12_build_ray_tracing_acceleration_structure_description.Inputs = dx12_ray_tracing_acceleration_structure_inputs;
			dx12_build_ray_tracing_acceleration_structure_description.ScratchAccelerationStructureData = static_cast<DX12Buffer*>(build_bottom_level_acceleration_structure.ScratchBottomLevelAccelerationStructure->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			dx12_build_ray_tracing_acceleration_structure_description.DestAccelerationStructureData = static_cast<DX12Buffer*>(build_bottom_level_acceleration_structure.DestinationBottomLevelAccelerationStructure->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			m_DX12CommandList->BuildRaytracingAccelerationStructure(&dx12_build_ray_tracing_acceleration_structure_description, 0, nullptr);
		}

		void BuildRaytracingTopLevelAccelerationStructure(const BuildRayTracingTopLevelAccelerationStructure& build_top_level_acceleration_structure) const override {
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS dx12_ray_tracing_acceleration_structure_inputs = {};
			dx12_ray_tracing_acceleration_structure_inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			dx12_ray_tracing_acceleration_structure_inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			dx12_ray_tracing_acceleration_structure_inputs.InstanceDescs = static_cast<RHI::DX12::DX12Buffer*>(build_top_level_acceleration_structure.description->InstancesBuffer->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			dx12_ray_tracing_acceleration_structure_inputs.NumDescs = build_top_level_acceleration_structure.description->InstanceCount;
			dx12_ray_tracing_acceleration_structure_inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC dx12_build_ray_tracing_acceleration_structure_description = {};
			dx12_build_ray_tracing_acceleration_structure_description.Inputs = dx12_ray_tracing_acceleration_structure_inputs;
			dx12_build_ray_tracing_acceleration_structure_description.ScratchAccelerationStructureData = static_cast<DX12Buffer*>(build_top_level_acceleration_structure.ScratchTopLevelAccelerationStructure->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			dx12_build_ray_tracing_acceleration_structure_description.DestAccelerationStructureData = static_cast<DX12Buffer*>(build_top_level_acceleration_structure.DestinationTopLevelAccelerationStructure->GetBuffer())->GetNative()->GetGPUVirtualAddress();
			m_DX12CommandList->BuildRaytracingAccelerationStructure(&dx12_build_ray_tracing_acceleration_structure_description, 0, nullptr);
		}
	private:
		ID3D12DescriptorHeap* m_DX12DescriptorHeaps[2]; // 0 : CBV_SRV_UAV, 1 : SAMPLERs.
		ID3D12GraphicsCommandList4* m_DX12CommandList;
	};

	inline D3D12_TEXTURE_COPY_LOCATION DX12ConvertCopyLocation(const RHI::TextureCopyLocation& location) {
		D3D12_TEXTURE_COPY_LOCATION dx12_location;
		dx12_location.pResource = static_cast<DX12Texture*>(location.Texture)->GetNative();
		if (location.Type == RHI::CopyLocationType::FOOTPRINT) {
			dx12_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dx12_location.PlacedFootprint.Footprint.Format = DX12ConvertFormat(location.Footprint.Format);
			dx12_location.PlacedFootprint.Offset = location.Footprint.Offset;
			dx12_location.PlacedFootprint.Footprint.Width = location.Footprint.Width;
			dx12_location.PlacedFootprint.Footprint.Height = location.Footprint.Height;
			dx12_location.PlacedFootprint.Footprint.Depth = location.Footprint.Depth;
			dx12_location.PlacedFootprint.Footprint.RowPitch = location.Footprint.RowPitch;
		}
		else {
			dx12_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dx12_location.SubresourceIndex = location.Subresource.MipSlice + (location.Subresource.ArraySlice * location.Subresource.MipLevels);
		}
		return dx12_location;
	}

	inline D3D12_TEXTURE_COPY_LOCATION DX12ConvertCopyLocation(const RHI::BufferTextureCopyLocation& location) {
		D3D12_TEXTURE_COPY_LOCATION dx12_location;
		dx12_location.pResource = static_cast<DX12Buffer*>(location.Buffer)->GetNative();
		if (location.Type == RHI::CopyLocationType::FOOTPRINT) {
			dx12_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dx12_location.PlacedFootprint.Footprint.Format = DX12ConvertFormat(location.Footprint.Format);
			dx12_location.PlacedFootprint.Offset = location.Footprint.Offset;
			dx12_location.PlacedFootprint.Footprint.Width = location.Footprint.Width;
			dx12_location.PlacedFootprint.Footprint.Height = location.Footprint.Height;
			dx12_location.PlacedFootprint.Footprint.Depth = location.Footprint.Depth;
			dx12_location.PlacedFootprint.Footprint.RowPitch = location.Footprint.RowPitch;
		}
		else {
			dx12_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dx12_location.SubresourceIndex = location.Subresource.MipSlice + (location.Subresource.ArraySlice * location.Subresource.MipLevels);
		}
		return dx12_location;
	}

	inline D3D12_RESOURCE_BARRIER DX12ConvertResourceBarrier(const RHI::ResourceBarrier& resource_barrier) {
		switch (resource_barrier.ResourceBarrierType) {
		case RHI::ResourceBarrierType::ALIASING_BARRIER_BUFFER:
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Aliasing.pResourceBefore = static_cast<DX12Buffer*>(resource_barrier.AliasingBarrierBuffer.BeforeBuffer)->GetNative();
			barrier.Aliasing.pResourceAfter = static_cast<DX12Buffer*>(resource_barrier.AliasingBarrierBuffer.AfterBuffer)->GetNative();
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			return barrier;
		}
		break;
		case RHI::ResourceBarrierType::ALIASING_BARRIER_TEXTURE:
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_ALIASING;
			barrier.Aliasing.pResourceBefore = static_cast<DX12Texture*>(resource_barrier.AliasingBarrierTexture.BeforeTexture)->GetNative();
			barrier.Aliasing.pResourceAfter = static_cast<DX12Texture*>(resource_barrier.AliasingBarrierTexture.AfterTexture)->GetNative();
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			return barrier;
		}
		break;
		case RHI::ResourceBarrierType::TRANSITION_BARRIER_BUFFER:
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = static_cast<DX12Buffer*>(resource_barrier.TransitionBarrierBuffer.Buffer)->GetNative();
			barrier.Transition.Subresource = RHI_SUBRESOURCE_INDEX_ALL;
			barrier.Transition.StateBefore = DX12ConvertBufferState(resource_barrier.TransitionBarrierBuffer.InitialState);
			barrier.Transition.StateAfter = DX12ConvertBufferState(resource_barrier.TransitionBarrierBuffer.FinalState);
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			return barrier;
		}
		break;
		case RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE:
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Transition.pResource = static_cast<DX12Texture*>(resource_barrier.TransitionBarrierTexture.Texture)->GetNative();
			if (resource_barrier.TransitionBarrierTexture.Subresource == RHI::SUBRESOURCE_ALL) {
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			}
			else {
				barrier.Transition.Subresource = resource_barrier.TransitionBarrierTexture.Subresource.MipSlice + (resource_barrier.TransitionBarrierTexture.Subresource.ArraySlice * resource_barrier.TransitionBarrierTexture.Subresource.MipLevels);
			}
			barrier.Transition.StateBefore = DX12ConvertTextureState(resource_barrier.TransitionBarrierTexture.InitialState);
			barrier.Transition.StateAfter = DX12ConvertTextureState(resource_barrier.TransitionBarrierTexture.FinalState);
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			return barrier;
		}
		break;
		case RHI::ResourceBarrierType::UNORDERED_ACCESS_BARRIER_BUFFER:
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.UAV.pResource = static_cast<DX12Buffer*>(resource_barrier.UnorderedAccessBarrierBuffer.Buffer)->GetNative();
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			return barrier;
		}
		break;
		case RHI::ResourceBarrierType::UNORDERED_ACCESS_BARRIER_TEXTURE:
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barrier.UAV.pResource = static_cast<DX12Texture*>(resource_barrier.UnorderedAccessBarrierTexture.Texture)->GetNative();
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			return barrier;
		}
		break;
		}
	}
}
#endif