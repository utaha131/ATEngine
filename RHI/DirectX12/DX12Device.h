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

namespace RHI::DX12 {
	class DX12Device : public RHI::IDevice {
		friend class DX12RenderBackend;
	public:
		DX12Device(ID3D12Device* dx12_device);
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
	private:
		ID3D12Device* m_DX12Device;
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