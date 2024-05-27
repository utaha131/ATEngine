#include "VKCommandList.h"
namespace RHI::VK {
	VKCommandList::VKCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, VkCommandBuffer vk_command_buffer) :
		RHI::ICommandList(command_type, command_allocator),
		m_VKCommandBuffer(vk_command_buffer)
	{

	}

	VKCommandList::~VKCommandList() {
		
	}
	
	void VKCommandList::Reset(const RHI::PipelineState pipeline_state) {
		VkCommandBufferBeginInfo vk_command_buffer_begin_info = {};
		vk_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vk_command_buffer_begin_info.flags = 0;
		vk_command_buffer_begin_info.pInheritanceInfo = VK_NULL_HANDLE;
		vkBeginCommandBuffer(m_VKCommandBuffer, &vk_command_buffer_begin_info);
		if (pipeline_state != RHI_NULL_HANDLE) {
			vkCmdBindPipeline(m_VKCommandBuffer, static_cast<VKPipelineState*>(pipeline_state)->GetBindPoint(), static_cast<const VKPipelineState*>(pipeline_state)->GetNative());
		}
	}

	void VKCommandList::Close() {
		vkEndCommandBuffer(m_VKCommandBuffer);
	}

	void VKCommandList::ResourceBarrier(uint32_t resource_barrier_count, const RHI::ResourceBarrier* p_resource_barrier) {
		std::vector<VkMemoryBarrier> vk_memory_barriers;
		std::vector<VkImageMemoryBarrier> vk_image_memory_barriers;
		std::vector<VkBufferMemoryBarrier> vk_buffer_memory_barriers;
		std::vector<VkMemoryBarrier>;
		for (uint32_t i = 0u; i < resource_barrier_count; ++i) {
			switch (p_resource_barrier[i].ResourceBarrierType) {
			case RHI::ResourceBarrierType::ALIASING_BARRIER_BUFFER:
			{

			}
				break;
			case RHI::ResourceBarrierType::ALIASING_BARRIER_TEXTURE:
			{

			}
				break;
			case RHI::ResourceBarrierType::TRANSITION_BARRIER_BUFFER:
			{
				VkBufferMemoryBarrier vk_buffer_memory_barrier;
				vk_buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				vk_buffer_memory_barrier.pNext = VK_NULL_HANDLE;
				vk_buffer_memory_barrier.buffer = static_cast<VKBuffer*>(p_resource_barrier[i].TransitionBarrierBuffer.Buffer)->GetNative();
				vk_buffer_memory_barrier.offset = 0ull;
				vk_buffer_memory_barrier.size = p_resource_barrier[i].TransitionBarrierBuffer.Buffer->GetDescription().Size;
				vk_buffer_memory_barrier.srcAccessMask = VKConvertBufferStateToAccessFlag(p_resource_barrier[i].TransitionBarrierBuffer.InitialState);
				vk_buffer_memory_barrier.dstAccessMask = VKConvertBufferStateToAccessFlag(p_resource_barrier[i].TransitionBarrierBuffer.FinalState);
				vk_buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				vk_buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			}
				break;
			case RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE:
			{
				VkImageMemoryBarrier vk_image_memory_barrier = {};
				vk_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				vk_image_memory_barrier.pNext = VK_NULL_HANDLE;
				vk_image_memory_barrier.image = static_cast<VKTexture*>(p_resource_barrier[i].TransitionBarrierTexture.Texture)->GetNative();
				if (p_resource_barrier[i].TransitionBarrierTexture.Subresource == RHI::SUBRESOURCE_ALL) {
					vk_image_memory_barrier.subresourceRange.baseArrayLayer = 0;
					vk_image_memory_barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
					vk_image_memory_barrier.subresourceRange.baseMipLevel = 0;
					vk_image_memory_barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
				}
				else {
					vk_image_memory_barrier.subresourceRange.baseArrayLayer = p_resource_barrier[i].TransitionBarrierTexture.Subresource.ArraySlice;
					vk_image_memory_barrier.subresourceRange.layerCount = 1;
					vk_image_memory_barrier.subresourceRange.baseMipLevel = p_resource_barrier[i].TransitionBarrierTexture.Subresource.MipSlice;
					vk_image_memory_barrier.subresourceRange.levelCount = 1;
				}

				vk_image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				if (p_resource_barrier[i].TransitionBarrierTexture.Texture->GetDescription().Format == RHI::Format::D32_FLOAT_S8X24_UINT) {
					vk_image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
				}
				if (p_resource_barrier[i].TransitionBarrierTexture.Texture->GetDescription().Format == RHI::Format::D16_UNORM) {
					vk_image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				}

				vk_image_memory_barrier.oldLayout = VKConvertResourceStateToImageLayout(p_resource_barrier[i].TransitionBarrierTexture.InitialState);
				vk_image_memory_barrier.newLayout = VKConvertResourceStateToImageLayout(p_resource_barrier[i].TransitionBarrierTexture.FinalState);
				vk_image_memory_barrier.srcAccessMask = VKConvertTextureStateToAccessFlag(p_resource_barrier[i].TransitionBarrierTexture.InitialState);
				vk_image_memory_barrier.dstAccessMask = VKConvertTextureStateToAccessFlag(p_resource_barrier[i].TransitionBarrierTexture.FinalState);
				vk_image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				vk_image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				vk_image_memory_barriers.emplace_back(vk_image_memory_barrier);
			}
				break;
			case RHI::ResourceBarrierType::UNORDERED_ACCESS_BARRIER_BUFFER:
			{
				/*VkBufferMemoryBarrier vk_buffer_memory_barrier = {};
				vk_buffer_memory_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
				vk_buffer_memory_barrier.pNext = VK_NULL_HANDLE;
				vk_buffer_memory_barrier.buffer = static_cast<VKBuffer*>(p_resource_barrier[i].TransitionBarrierBuffer.Buffer)->GetNative();
				vk_buffer_memory_barrier.offset = 0ull;
				vk_buffer_memory_barrier.size = p_resource_barrier[i].TransitionBarrierBuffer.Buffer->GetDescription().Size;
				vk_buffer_memory_barrier.srcAccessMask = VKConvertBufferStateToAccessFlag(p_resource_barrier[i].TransitionBarrierBuffer.InitialState);
				vk_buffer_memory_barrier.dstAccessMask = VKConvertBufferStateToAccessFlag(p_resource_barrier[i].TransitionBarrierBuffer.FinalState);
				vk_buffer_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				vk_buffer_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				vk_buffer_memory_barriers.emplace_back(vk_buffer_memory_barrier);*/
			}
				break;
			case RHI::ResourceBarrierType::UNORDERED_ACCESS_BARRIER_TEXTURE:
			{
				VkMemoryBarrier vk_memory_barrier = {};
				vk_memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
				vk_memory_barrier.pNext = VK_NULL_HANDLE;
				vk_memory_barrier.srcAccessMask = (VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
				vk_memory_barriers.emplace_back(vk_memory_barrier);
				
			}
				break;
			}
		}
		if (vk_memory_barriers.size() > 0 || vk_image_memory_barriers.size() > 0 || vk_buffer_memory_barriers.size() > 0) {
			//OutputDebugString(L"Executing Barriers.\n");
			vkCmdPipelineBarrier(m_VKCommandBuffer, 
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
				0, 
				vk_memory_barriers.size(), vk_memory_barriers.data(),
				vk_buffer_memory_barriers.size(), vk_buffer_memory_barriers.data(),
				vk_image_memory_barriers.size(), vk_image_memory_barriers.data());
		}
	}

	void VKCommandList::CopyBuffer(RHI::Buffer destination_buffer, RHI::Buffer source_buffer) {
		VkBufferCopy vk_buffer_copy;
		vk_buffer_copy.dstOffset = 0;
		vk_buffer_copy.srcOffset = 0;
		vk_buffer_copy.size = source_buffer->GetDescription().Size;
		vkCmdCopyBuffer(m_VKCommandBuffer, static_cast<VKBuffer*>(source_buffer)->GetNative(), static_cast<VKBuffer*>(destination_buffer)->GetNative(), 1, &vk_buffer_copy);
	}

	void VKCommandList::CopyBufferRegion(RHI::Buffer destination_buffer, uint64_t destination_offset, RHI::Buffer source_buffer, uint64_t source_offset, uint64_t size) {
		VkBufferCopy vk_buffer_copy;
		vk_buffer_copy.dstOffset = destination_offset;
		vk_buffer_copy.srcOffset = source_offset;
		vk_buffer_copy.size = size;
		vkCmdCopyBuffer(m_VKCommandBuffer, static_cast<VKBuffer*>(source_buffer)->GetNative(), static_cast<VKBuffer*>(destination_buffer)->GetNative(), 1, &vk_buffer_copy);
	}

	void VKCommandList::CopyBufferToTexture(RHI::TextureCopyLocation& dest_texture_region, RHI::BufferTextureCopyLocation& source_buffer_region) {
		VkBufferImageCopy vk_buffer_image_copy = VkConvertBufferTextureCopyLocationToBufferImageCopy(source_buffer_region, dest_texture_region);
		vkCmdCopyBufferToImage(m_VKCommandBuffer, static_cast<VKBuffer*>(source_buffer_region.Buffer)->GetNative(), static_cast<VKTexture*>(dest_texture_region.Texture)->GetNative(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_buffer_image_copy);
	}

	void VKCommandList::CopyTexture(RHI::Texture destination_texture, RHI::Texture source_texture) {
		VkImageCopy image_copy;
		image_copy.srcOffset.x = 0;
		image_copy.srcOffset.y = 0;
		image_copy.srcOffset.z = 0;
		image_copy.srcSubresource.baseArrayLayer = 0;
		image_copy.srcSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
		image_copy.srcSubresource.mipLevel = 0;
		image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		image_copy.dstOffset.x = 0;
		image_copy.dstOffset.y = 0;
		image_copy.dstOffset.z = 0;
		image_copy.dstSubresource.baseArrayLayer = 0;
		image_copy.dstSubresource.layerCount = VK_REMAINING_ARRAY_LAYERS;
		image_copy.dstSubresource.mipLevel = 0;
		image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		image_copy.extent.width = source_texture->GetDescription().Width;
		image_copy.extent.height = source_texture->GetDescription().Height;
		image_copy.extent.depth = source_texture->GetDescription().DepthOrArray;
		vkCmdCopyImage(m_VKCommandBuffer, static_cast<VKTexture*>(source_texture)->GetNative(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, static_cast<VKTexture*>(destination_texture)->GetNative(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
	}

	void VKCommandList::CopyTextureRegion(RHI::TextureCopyLocation& dest_texture_region, RHI::TextureCopyLocation& source_texture_region) {
		VkImageCopy image_copy = VKConvertTextureCopyLocationToImageCopy(source_texture_region, dest_texture_region);
		vkCmdCopyImage(m_VKCommandBuffer, static_cast<VKTexture*>(source_texture_region.Texture)->GetNative(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, static_cast<VKTexture*>(dest_texture_region.Texture)->GetNative(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy);
	}

	void VKCommandList::SetGraphicsRootSignature(const RHI::RootSignature root_signature) {
		m_CurrentRootSignature = root_signature;
	}

	void VKCommandList::SetComputeRootSignature(const RHI::RootSignature root_signature) {
		m_CurrentRootSignature = root_signature;
	}

	void VKCommandList::SetPipelineState(const RHI::PipelineState pipeline_state) {
		vkCmdBindPipeline(m_VKCommandBuffer, static_cast<const VKPipelineState*>(pipeline_state)->GetBindPoint(), static_cast<const VKPipelineState*>(pipeline_state)->GetNative());
	}

	void VKCommandList::SetVertexBuffer(uint32_t start_slot, uint32_t num_views, const RHI::VertexBufferView* p_vertex_buffer_views) {
		std::vector<VkBuffer> buffers = std::vector<VkBuffer>(num_views);
		std::vector<VkDeviceSize> offsets = std::vector<VkDeviceSize>(num_views, 0);
		for (uint32_t i = 0; i < num_views; ++i) {
			buffers[i] = static_cast<const VKBuffer*>(p_vertex_buffer_views[i].Buffer)->GetNative();
		}
		vkCmdBindVertexBuffers(m_VKCommandBuffer, start_slot, num_views, buffers.data(), offsets.data());
	}

	void VKCommandList::SetIndexBuffer(const RHI::IndexBufferView& index_buffer_view) {
		vkCmdBindIndexBuffer(m_VKCommandBuffer, static_cast<const VKBuffer*>(index_buffer_view.Buffer)->GetNative(), 0, VKConvertIndexBufferFormat(index_buffer_view.Format));
	}

	void VKCommandList::SetViewport(const RHI::Viewport& viewport) {
		VkViewport vk_viewport = {};
		vk_viewport.x = viewport.TopLeftX;
		vk_viewport.y = viewport.Height - viewport.TopLeftY;
		vk_viewport.width = viewport.Width;
		vk_viewport.height = -viewport.Height;
		vk_viewport.minDepth = viewport.MinDepth;
		vk_viewport.maxDepth = viewport.MaxDepth;
		vkCmdSetViewport(m_VKCommandBuffer, 0u, 1u, &vk_viewport);
	}

	void VKCommandList::SetScissorRect(const RHI::Rect& scissor_rect) {
		VkRect2D vk_rect = {};
		vk_rect.offset.x = scissor_rect.Left;
		vk_rect.offset.y = scissor_rect.Top;
		vk_rect.extent.width = static_cast<uint32_t>(scissor_rect.Right);
		vk_rect.extent.height = static_cast<uint32_t>(scissor_rect.Bottom);
		vkCmdSetScissor(m_VKCommandBuffer, 0, 1, &vk_rect);
	}

	void VKCommandList::SetRootConstants(uint32_t root_parameter_index, uint32_t constants_count, const void* p_data, uint32_t offset) {
		vkCmdPushConstants(m_VKCommandBuffer, static_cast<VKRootSignature*>(m_CurrentRootSignature)->GetNative(), VK_SHADER_STAGE_ALL, 4 * offset, constants_count * 4, p_data);
	}

	void VKCommandList::SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, const RHI::DescriptorTable table) {
		VkDescriptorSet vk_descriptor_set = static_cast<const VKDescriptorTable*>(table)->GetNative();
		vkCmdBindDescriptorSets(m_VKCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VKRootSignature*>(m_CurrentRootSignature)->GetNative(), root_parameter_index, 1, &vk_descriptor_set, 0, VK_NULL_HANDLE);
	}

	void VKCommandList::SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, RHI::ConstantBufferView constant_buffer_View) {
	}

	void VKCommandList::SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, RHI::ShaderResourceView shader_resource_view) {
	}

	void VKCommandList::SetGraphicsRootUnorderedAccessView(uint32_t root_parameter_index, RHI::UnorderedAccessView unordered_access_view) {
	}

	void VKCommandList::SetComputeRootDescriptorTable(uint32_t root_parameter_index, const RHI::DescriptorTable table) {
		VkDescriptorSet vk_descriptor_set = static_cast<const VKDescriptorTable*>(table)->GetNative();
		vkCmdBindDescriptorSets(m_VKCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<VKRootSignature*>(m_CurrentRootSignature)->GetNative(), root_parameter_index, 1, &vk_descriptor_set, 0, VK_NULL_HANDLE);
	}

	void VKCommandList::SetPrimitiveTopology(RHI::PrimitiveTopology primitive_topology) {
		//TODO Enable Dynamic Primitive Topology Vulkan.
	}

	void VKCommandList::Draw(uint32_t vertices, uint32_t instances, uint32_t start_vertex, uint32_t start_instance) {
		vkCmdDraw(m_VKCommandBuffer, vertices, instances, start_vertex, start_instance);
	}

	void VKCommandList::DrawIndexed(uint32_t indices, uint32_t instance_count, uint32_t start_index, uint32_t start_vertex, uint32_t start_instance) {
		vkCmdDrawIndexed(m_VKCommandBuffer, indices, instance_count, start_index, start_vertex, start_instance);
	}

	void VKCommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
		vkCmdDispatch(m_VKCommandBuffer, x, y, z);
	}

	void VKCommandList::BeginRenderPass(const RHI::BeginRenderPassInfo& begin_render_pass_info) {
		m_CurrentRenderPassInfo = begin_render_pass_info;
		
		VkRenderingInfoKHR rendering_info = {};
		rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		rendering_info.renderArea.extent = { begin_render_pass_info.FrameBuffer->GetDescription().Width, begin_render_pass_info.FrameBuffer->GetDescription().Height };
		rendering_info.renderArea.offset = { 0, 0 };
		rendering_info.layerCount = 1;
		
		std::vector<VkRenderingAttachmentInfoKHR> vk_rendering_attachment_infos = std::vector<VkRenderingAttachmentInfoKHR>(begin_render_pass_info.RenderPass->GetDescription().Attachments.size(), VkRenderingAttachmentInfoKHR{});
		for (uint32_t i = 0u; i < begin_render_pass_info.RenderPass->GetDescription().Attachments.size(); ++i) {
			vk_rendering_attachment_infos[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			vk_rendering_attachment_infos[i].imageView = static_cast<const VKRenderTargetView*>(begin_render_pass_info.FrameBuffer->GetDescription().RenderTargetViews[i])->GetNativeImageView();
			vk_rendering_attachment_infos[i].imageLayout = VKConvertResourceStateToImageLayout(begin_render_pass_info.RenderPass->GetDescription().Attachments[i].InitialState);
			vk_rendering_attachment_infos[i].loadOp = ConvertLoadOp(begin_render_pass_info.RenderPass->GetDescription().Attachments[i].LoadOp);
			vk_rendering_attachment_infos[i].storeOp = ConvertStoreOp(begin_render_pass_info.RenderPass->GetDescription().Attachments[i].StoreOp);
			vk_rendering_attachment_infos[i].clearValue = VKConvertClearValue(begin_render_pass_info.ClearValues[i]);
		}
		rendering_info.colorAttachmentCount = vk_rendering_attachment_infos.size();
		rendering_info.pColorAttachments = vk_rendering_attachment_infos.data();
		
		VkRenderingAttachmentInfoKHR depth_vk_rendering_attachment_info = {};
		if (begin_render_pass_info.RenderPass->GetDescription().DepthAttachment.has_value()) {
			depth_vk_rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
			depth_vk_rendering_attachment_info.imageView = static_cast<const VKDepthStencilView*>(begin_render_pass_info.FrameBuffer->GetDescription().DepthStencilView.value())->GetNative();
			depth_vk_rendering_attachment_info.imageLayout = VKConvertResourceStateToImageLayout(begin_render_pass_info.RenderPass->GetDescription().DepthAttachment.value().InitialState);
			depth_vk_rendering_attachment_info.loadOp = ConvertLoadOp(begin_render_pass_info.RenderPass->GetDescription().DepthAttachment.value().LoadOp);
			depth_vk_rendering_attachment_info.storeOp = ConvertStoreOp(begin_render_pass_info.RenderPass->GetDescription().DepthAttachment.value().StoreOp);
			depth_vk_rendering_attachment_info.clearValue = VKConvertClearValue(begin_render_pass_info.DepthClearValue.value());
			rendering_info.pDepthAttachment = &depth_vk_rendering_attachment_info;
			rendering_info.pStencilAttachment;
		}
		
		vkCmdBeginRendering(m_VKCommandBuffer, &rendering_info);
	}

	void VKCommandList::EndRenderPass() {
		vkCmdEndRendering(m_VKCommandBuffer);
		std::vector<RHI::ResourceBarrier> resource_barriers = std::vector<RHI::ResourceBarrier>();
		resource_barriers.reserve(m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments.size() + 1);
		for (uint32_t i = 0u; i < m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments.size(); ++i) {
			if (m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].InitialState != m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].FinalState) {
				RHI::ResourceBarrier barrier;
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = static_cast<VKRenderTargetView*>(m_CurrentRenderPassInfo.FrameBuffer->GetDescription().RenderTargetViews[i])->GetTexture();
				barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
				barrier.TransitionBarrierTexture.InitialState = m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].InitialState;
				barrier.TransitionBarrierTexture.FinalState = m_CurrentRenderPassInfo.RenderPass->GetDescription().Attachments[i].FinalState;
				resource_barriers.emplace_back(barrier);
			}
		}
		
		if (m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.has_value()) {
			if (m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().InitialState != m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().FinalState) {
				RHI::ResourceBarrier barrier;
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = static_cast<VKDepthStencilView*>(m_CurrentRenderPassInfo.FrameBuffer->GetDescription().DepthStencilView.value())->GetTexture();
				barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
				barrier.TransitionBarrierTexture.InitialState = m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().InitialState;
				barrier.TransitionBarrierTexture.FinalState = m_CurrentRenderPassInfo.RenderPass->GetDescription().DepthAttachment.value().FinalState;
				resource_barriers.emplace_back(barrier);
			}
		}
		
		ResourceBarrier(resource_barriers.size(), resource_barriers.data());
	}
}