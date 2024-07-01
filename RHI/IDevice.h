#ifndef _RHI_I_DEVICE_H_
#define _RHI_I_DEVICE_H_
#include "IResourceHeap.h"
#include "IBuffer.h"
#include "ITexture.h"
#include "ISwapChain.h"
#include "IRootSignature.h"
#include "IShader.h"
#include "IRenderPass.h"
#include "IFrameBuffer.h"
#include "IPipelineState.h"
#include "IConstantBufferView.h"
#include "IDepthStencilView.h"
#include "IRenderTargetView.h"
#include "ISampler.h"
#include "IShaderResourceView.h"
#include "IUnorderedAccessView.h"
#include "ICommandAllocator.h"
#include "ICommandList.h"
#include "IDescriptorTable.h"
#include "RHICore.h"

namespace RHI {
	class IDevice {
	public:
		virtual ~IDevice() = default;
		//Object Creation Interface.
		virtual RHI::Result CreateResourceHeap(const RHI::ResourceHeapDescription& description, RHI::ResourceHeap& resource_heap) const = 0;
		virtual RHI::Result CreateBuffer(RHI::ResourceHeap resource_heap, uint64_t offset, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const = 0;
		virtual RHI::Result CreateCommittedBuffer(RHI::ResourceHeapType resource_heap_type, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const = 0;
		virtual RHI::Result CreateTexture(RHI::ResourceHeap resource_heap, uint64_t offset, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const = 0;
		virtual RHI::Result CreateCommittedTexture(RHI::ResourceHeapType resource_heap_type, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const = 0;
		virtual RHI::Result CreateRootSignature(const RHI::RootSignatureDescription& description, RHI::RootSignature& root_signature) = 0;
		virtual RHI::Result CreateShader(const RHI::ShaderDescription& description, RHI::Shader& shader) const = 0;
		virtual RHI::Result CreateRenderPass(const RHI::RenderPassDescription& description, RHI::RenderPass& render_pass) const = 0;
		virtual RHI::Result CreateFence(uint64_t initial_value, RHI::Fence& fence) const = 0;
		virtual RHI::Result CreateFrameBuffer(const RHI::FrameBufferDescription& description, RHI::FrameBuffer& frame_buffer) const = 0;
		virtual RHI::Result CreateGraphicsPipelineState(const RHI::GraphicsPipelineStateDescription& description, RHI::PipelineState& pipeline_state) const = 0;
		virtual RHI::Result CreateComputePipelineState(const RHI::ComputePipelineStateDescription& description, RHI::PipelineState& pipeline_state) const = 0;
		virtual RHI::Result CreateConstantBufferView(const RHI::ConstantBufferViewDescription& description, RHI::ConstantBufferView& constant_buffer_view) = 0;
		virtual RHI::Result CreateDepthStencilView(const RHI::Texture texture, const RHI::DepthStencilViewDescription& description, RHI::DepthStencilView& depth_stencil_view) = 0;
		virtual RHI::Result CreateRenderTargetView(const RHI::Buffer buffer, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) = 0;
		virtual RHI::Result CreateRenderTargetView(const RHI::Texture texture, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) = 0;
		virtual RHI::Result CreateSampler(const RHI::SamplerDescription& description, RHI::Sampler& sampler) = 0;
		virtual RHI::Result CreateShaderResourceView(const RHI::Buffer buffer, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) = 0;
		virtual RHI::Result CreateShaderResourceView(const RHI::Texture texture, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) = 0;
		virtual RHI::Result CreateUnorderedAccessView(const RHI::Buffer buffer, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) = 0;
		virtual RHI::Result CreateUnorderedAccessView(const RHI::Texture texture, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) = 0;
		virtual RHI::Result CreateCommandAllocator(RHI::CommandType command_type, RHI::CommandAllocator& command_allocator) const = 0;
		virtual RHI::Result CreateCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, RHI::CommandList& command_list) const = 0;

		//Execution Interface.
		virtual void ExecuteCommandList(RHI::CommandType command_type, uint32_t command_list_count, const RHI::CommandList* p_command_lists) = 0;
		virtual void SignalQueue(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) = 0;
		virtual void HostWait(RHI::Fence fence, uint64_t fence_value) = 0;
		virtual void QueueWait(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) = 0;

		//Other Interfaces.
		virtual RHI::AllocationInfo GetResourceAllocationInfo(const RHI::BufferDescription& buffer_description) const = 0;
		virtual RHI::AllocationInfo GetResourceAllocationInfo(const RHI::TextureDescription& texture_description) const = 0;
		virtual uint32_t GetTexturePitchAlignment() const = 0;

		virtual void AllocateDescriptorTable(const RHI::RootSignature root_signature, uint32_t parameter_index, RHI::DescriptorTable& table) = 0;
		virtual void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ConstantBufferView* p_cbv) = 0;
		virtual void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::Sampler* p_sampler) = 0;
		virtual void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ShaderResourceView* p_srv) = 0;
		virtual void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::UnorderedAccessView* p_uav) = 0;

		//Test Ray Tracing Features.
		virtual void CreateRayTracingPipelineState(const RayTracingPipelineStateDescription& description, IRayTracingPipeline*& pipeline) const = 0;
		virtual void CreateRayTracingBottomLevelAccelerationStructure(const RHI::Buffer buffer, IRayTracingBottomLevelAccelerationStructure*& bottom_level_acceleration_structure) const = 0;
		virtual void CreateRayTracingTopLevelAccelerationStructure(const RHI::Buffer buffer, IRayTracingTopLevelAccelerationStructure*& top_level_acceleration_structure) const = 0;
		virtual void CreateRayTracingInstanceBuffer(RHI::Buffer buffer, uint64_t capacity, IRayTracingInstanceBuffer*& instance_buffer) const = 0;
		virtual void GetRayTracingBottomLevelAccelerationStructureMemoryInfo(const RayTracingBottomLevelAccelerationStructureDescription bottom_level_acceleration_structure_description, RayTracingAccelerationStructureMemoryInfo& memory_info) const = 0;
		virtual void GetRayTracingTopLevelAccelerationStructureMemoryInfo(const RayTracingTopLevelAccelerationStructureDescription top_level_acceleration_structure_description, RayTracingAccelerationStructureMemoryInfo& memory_info) const = 0;
		virtual void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, IRayTracingTopLevelAccelerationStructure** p_top_level_acceleration_structures) = 0;
	};
}
#endif