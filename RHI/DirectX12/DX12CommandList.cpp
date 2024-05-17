#include "DX12CommandList.h"

namespace RHI::DX12 {
	DX12CommandList::DX12CommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, ID3D12GraphicsCommandList* dx12_command_list, std::array<ID3D12DescriptorHeap*, 2> dx12_descriptor_heaps) :
		RHI::ICommandList(command_type, command_allocator),
		m_DX12CommandList(dx12_command_list)
	{
		m_DX12DescriptorHeaps[0] = dx12_descriptor_heaps[0];
		m_DX12DescriptorHeaps[1] = dx12_descriptor_heaps[1];
	}

	DX12CommandList::~DX12CommandList() {
		m_DX12CommandList->Release();
	}

	void DX12CommandList::Reset(const RHI::PipelineState pipeline_state) {
		ID3D12PipelineState* dx12_pipeline_state = nullptr;
		if (pipeline_state != RHI_NULL_HANDLE) {
			dx12_pipeline_state = static_cast<DX12PipelineState*>(pipeline_state)->GetNative();
		}
		m_DX12CommandList->Reset(static_cast<DX12CommandAllocator*>(m_CommandAllocator)->GetNative(), dx12_pipeline_state);
		if (m_CommandType != RHI::CommandType::COPY) {
			m_DX12CommandList->SetDescriptorHeaps(2, m_DX12DescriptorHeaps);
		}
	}

	void DX12CommandList::Close() {
		m_DX12CommandList->Close();
	}

	void DX12CommandList::ResourceBarrier(uint32_t resource_barrier_count, const RHI::ResourceBarrier* p_resource_barrier) {
		std::vector<D3D12_RESOURCE_BARRIER> dx12_resource_barriers = std::vector<D3D12_RESOURCE_BARRIER>();
		dx12_resource_barriers.reserve(resource_barrier_count);
		for (uint32_t i = 0u; i < resource_barrier_count; ++i) {
			if (p_resource_barrier[i].TransitionBarrierTexture.InitialState == RHI::TextureState::CREATED && p_resource_barrier[i].TransitionBarrierTexture.FinalState == RHI::TextureState::COMMON) {
				continue;
			}
			dx12_resource_barriers.emplace_back(DX12ConvertResourceBarrier(p_resource_barrier[i]));
		}
		if (dx12_resource_barriers.size() > 0) {
			m_DX12CommandList->ResourceBarrier(dx12_resource_barriers.size(), dx12_resource_barriers.data());
		}
	}

	void DX12CommandList::CopyBuffer(RHI::Buffer destination_buffer, RHI::Buffer source_buffer) {
		m_DX12CommandList->CopyResource(static_cast<DX12Buffer*>(destination_buffer)->GetNative(), static_cast<DX12Buffer*>(source_buffer)->GetNative());
	}

	void DX12CommandList::CopyBufferRegion(RHI::Buffer destination_buffer, uint64_t destination_offset, RHI::Buffer source_buffer, uint64_t source_offset, uint64_t size) {
		m_DX12CommandList->CopyBufferRegion(static_cast<DX12Buffer*>(destination_buffer)->GetNative(), destination_offset, static_cast<DX12Buffer*>(source_buffer)->GetNative(), source_offset, size);
	}

	void DX12CommandList::CopyBufferToTexture(RHI::TextureCopyLocation& dest_texture_region, RHI::BufferTextureCopyLocation& source_buffer_region) {
		m_DX12CommandList->CopyTextureRegion(&DX12ConvertCopyLocation(dest_texture_region), 0u, 0u, 0u, &DX12ConvertCopyLocation(source_buffer_region), nullptr);
	}

	void DX12CommandList::CopyTexture(RHI::Texture destination_texture, RHI::Texture source_texture) {
		m_DX12CommandList->CopyResource(static_cast<DX12Texture*>(destination_texture)->GetNative(), static_cast<DX12Texture*>(source_texture)->GetNative());
	}

	void DX12CommandList::CopyTextureRegion(RHI::TextureCopyLocation& dest_texture_region, RHI::TextureCopyLocation& source_texture_region) {
		m_DX12CommandList->CopyTextureRegion(&DX12ConvertCopyLocation(dest_texture_region), 0u, 0u, 0u, &DX12ConvertCopyLocation(source_texture_region), nullptr);
	}

	void DX12CommandList::SetGraphicsRootSignature(const RHI::RootSignature root_signature) {
		m_DX12CommandList->SetGraphicsRootSignature(static_cast<DX12RootSignature*>(root_signature)->GetNative());
	}

	void DX12CommandList::SetComputeRootSignature(const RHI::RootSignature root_signature) {
		m_DX12CommandList->SetComputeRootSignature(static_cast<DX12RootSignature*>(root_signature)->GetNative());
	}

	void DX12CommandList::SetPipelineState(const RHI::PipelineState pipeline_state) {
		m_DX12CommandList->SetPipelineState(static_cast<DX12PipelineState*>(pipeline_state)->GetNative());
	}

	void DX12CommandList::SetVertexBuffer(uint32_t start_slot, uint32_t num_views, const RHI::VertexBufferView* p_vertex_buffer_views) {
		std::vector<D3D12_VERTEX_BUFFER_VIEW> dx12_vertex_buffer_views = std::vector<D3D12_VERTEX_BUFFER_VIEW>(num_views);
		for (uint32_t i = 0u; i < num_views; ++i) {
			dx12_vertex_buffer_views[i].BufferLocation = static_cast<DX12Buffer*>(p_vertex_buffer_views[i].Buffer)->GetNative()->GetGPUVirtualAddress();
			dx12_vertex_buffer_views[i].SizeInBytes = p_vertex_buffer_views[i].Size;
			dx12_vertex_buffer_views[i].StrideInBytes = p_vertex_buffer_views[i].Stride;
		}
		m_DX12CommandList->IASetVertexBuffers(start_slot, num_views, dx12_vertex_buffer_views.data());
	}

	void DX12CommandList::SetIndexBuffer(const RHI::IndexBufferView& index_buffer_view) {
		D3D12_INDEX_BUFFER_VIEW dx12_index_buffer_view;
		dx12_index_buffer_view.Format = DX12ConvertFormat(index_buffer_view.Format);
		dx12_index_buffer_view.BufferLocation = static_cast<DX12Buffer*>(index_buffer_view.Buffer)->GetNative()->GetGPUVirtualAddress();
		dx12_index_buffer_view.SizeInBytes = index_buffer_view.Size;
		m_DX12CommandList->IASetIndexBuffer(&dx12_index_buffer_view);
	}

	void DX12CommandList::SetViewport(const RHI::Viewport& viewport) {
		m_DX12CommandList->RSSetViewports(1u, &DX12ConvertViewport(viewport));
	}

	void DX12CommandList::SetScissorRect(const RHI::Rect& scissor_rect) {
		m_DX12CommandList->RSSetScissorRects(1u, &DX12ConvertRect(scissor_rect));
	}

	void DX12CommandList::SetRootConstants(uint32_t root_parameter_index, uint32_t constants_count, const void* p_data, uint32_t offset) {
		m_DX12CommandList->SetGraphicsRoot32BitConstants(root_parameter_index, constants_count, p_data, offset);
	}

	void DX12CommandList::SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, const RHI::DescriptorTable table) {
		m_DX12CommandList->SetGraphicsRootDescriptorTable(root_parameter_index, static_cast<RHI::DX12::DX12DescriptorTable*>(table)->GetGPUHandle());
	}

	void DX12CommandList::SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, RHI::ConstantBufferView constant_buffer_View) {
		//TODO add support for Root Descriptors.
	}

	void DX12CommandList::SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, RHI::ShaderResourceView shader_resource_view) {
		//TODO add support for Root Descriptors.
	}

	void DX12CommandList::SetGraphicsRootUnorderedAccessView(uint32_t root_parameter_index, RHI::UnorderedAccessView unordered_access_view) {
		//TODO add support for Root Descriptors.
	}

	void DX12CommandList::SetComputeRootDescriptorTable(uint32_t root_parameter_index, const RHI::DescriptorTable table) {
		m_DX12CommandList->SetComputeRootDescriptorTable(root_parameter_index, static_cast<RHI::DX12::DX12DescriptorTable*>(table)->GetGPUHandle());
	}

	void DX12CommandList::SetPrimitiveTopology(RHI::PrimitiveTopology primitive_topology) {
		m_DX12CommandList->IASetPrimitiveTopology(DX12ConvertPrimitiveTopology(primitive_topology));
	}

	void DX12CommandList::Draw(uint32_t vertices, uint32_t instances, uint32_t start_vertex, uint32_t start_instance) {
		m_DX12CommandList->DrawInstanced(vertices, instances, start_vertex, start_instance);
	}

	void DX12CommandList::DrawIndexed(uint32_t indices, uint32_t instance_count, uint32_t start_index, uint32_t start_vertex, uint32_t start_instance) {
		m_DX12CommandList->DrawIndexedInstanced(indices, instance_count, start_index, start_vertex, start_instance);
	}
	
	void DX12CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
		m_DX12CommandList->Dispatch(x, y, z);
	}

	void DX12CommandList::BeginRenderPass(const RHI::BeginRenderPassInfo& begin_render_pass_info) {
		m_CurrentRenderPassInfo = begin_render_pass_info;
		for (uint32_t i = 0u; i < begin_render_pass_info.RenderPass->GetDescription().Attachments.size(); ++i) {
			if (begin_render_pass_info.RenderPass->GetDescription().Attachments[i].LoadOp == RHI::RenderPassAttachment::LoadOperation::CLEAR) {
				m_DX12CommandList->ClearRenderTargetView(static_cast<const DX12RenderTargetView*>(begin_render_pass_info.FrameBuffer->GetDescription().RenderTargetViews[i])->GetNative(), DX12ConvertClearValue(begin_render_pass_info.ClearValues[i], begin_render_pass_info.ClearValues[i].Format).Color, 0u, nullptr);
			}
		}
		D3D12_CPU_DESCRIPTOR_HANDLE* dx12_depth_handle = nullptr;
		if (m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.has_value()) {
			dx12_depth_handle = &static_cast<const DX12DepthStencilView*>(m_CurrentRenderPassInfo.FrameBuffer->GetDescription().DepthStencilView.value())->GetNative();
			if (m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().LoadOp == RHI::RenderPassAttachment::LoadOperation::CLEAR) {
				D3D12_CLEAR_VALUE dx12_clear_value = DX12ConvertClearValue(begin_render_pass_info.DepthClearValue.value(), begin_render_pass_info.DepthClearValue.value().Format);
				m_DX12CommandList->ClearDepthStencilView(*dx12_depth_handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, dx12_clear_value.DepthStencil.Depth, dx12_clear_value.DepthStencil.Stencil, 0u, nullptr);
			}
		}
		m_DX12CommandList->OMSetRenderTargets(begin_render_pass_info.FrameBuffer->GetDescription().RenderTargetViews.size(), static_cast<DX12FrameBuffer*>(begin_render_pass_info.FrameBuffer)->GetRenderTargets(), FALSE, dx12_depth_handle);
	}

	void DX12CommandList::EndRenderPass() {
		std::vector<RHI::ResourceBarrier> barriers = std::vector<RHI::ResourceBarrier>();
		barriers.reserve(m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments.size() + 1);
		for (uint32_t i = 0u; i < m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments.size(); ++i) {
			if (m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].InitialState != m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].FinalState) {
				RHI::ResourceBarrier barrier;
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = static_cast<DX12RenderTargetView*>(m_CurrentRenderPassInfo.FrameBuffer->GetDescription().RenderTargetViews[i])->GetTexture();
				barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
				barrier.TransitionBarrierTexture.InitialState = m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].InitialState;
				barrier.TransitionBarrierTexture.FinalState = m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].FinalState;
				barriers.emplace_back(barrier);
			}
		}
		if (m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.has_value() && m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().InitialState != m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().FinalState) {
			RHI::ResourceBarrier barrier;
			barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
			barrier.TransitionBarrierTexture.Texture = static_cast<DX12DepthStencilView*>(m_CurrentRenderPassInfo.FrameBuffer->GetDescription().DepthStencilView.value())->GetTexture();
			barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
			barrier.TransitionBarrierTexture.InitialState = m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().InitialState;
			barrier.TransitionBarrierTexture.FinalState = m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().FinalState;
			barriers.emplace_back(barrier);
		}
		ResourceBarrier(static_cast<uint32_t>(barriers.size()), barriers.data());
	}
}