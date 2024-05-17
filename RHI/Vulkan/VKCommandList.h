#ifndef _RHI_VK_COMMAND_LIST_H_
#define _RHI_K_COMMAND_LIST_H_
#include <assert.h>
#include "../ICommandList.h"
#include <vulkan/vulkan.h>
#include "VKFrameBuffer.h"
#include "VKPipelineState.h"
#include "VKBuffer.h"
#include "VKTexture.h"
#include "VKPipelineState.h"
#include "VKRenderTargetView.h"
#include "VKDepthStencilView.h"
#include "VKGraphics.h"
#include "VKDescriptorHeap.h"

namespace RHI::VK {
	class VKCommandList : public ICommandList {
	public:
		VKCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, VkCommandBuffer vk_command_buffer);
		~VKCommandList() override;

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

		inline VkCommandBuffer GetNative() { return m_VKCommandBuffer; }
	private:
		VkCommandBuffer m_VKCommandBuffer;
	};

	inline VkBufferImageCopy VkConvertBufferTextureCopyLocationToBufferImageCopy(const RHI::BufferTextureCopyLocation& buffer_texture_copy_location, const RHI::TextureCopyLocation& texture_copy_location) {
		VkBufferImageCopy vk_buffer_image_copy;

		if (buffer_texture_copy_location.Type == RHI::CopyLocationType::FOOTPRINT) {
			vk_buffer_image_copy.bufferOffset = buffer_texture_copy_location.Footprint.Offset;
			vk_buffer_image_copy.bufferRowLength = 0;
			vk_buffer_image_copy.bufferImageHeight = 0;
		}
		else {
			vk_buffer_image_copy.bufferOffset = 0;
			vk_buffer_image_copy.bufferRowLength = 0;
			vk_buffer_image_copy.bufferImageHeight = 0;
		}

		if (texture_copy_location.Type == RHI::CopyLocationType::FOOTPRINT) {
			vk_buffer_image_copy.imageOffset.x = 0;
			vk_buffer_image_copy.imageOffset.y = 0;
			vk_buffer_image_copy.imageOffset.z = 0;
			vk_buffer_image_copy.imageSubresource.mipLevel = 0;
			vk_buffer_image_copy.imageSubresource.baseArrayLayer = 0;
			vk_buffer_image_copy.imageSubresource.layerCount = 1; texture_copy_location.Footprint.Depth;
			vk_buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else {
			vk_buffer_image_copy.imageOffset.x = 0;
			vk_buffer_image_copy.imageOffset.y = 0;
			vk_buffer_image_copy.imageOffset.z = 0;
			vk_buffer_image_copy.imageSubresource.mipLevel = texture_copy_location.Subresource.MipSlice;
			vk_buffer_image_copy.imageSubresource.baseArrayLayer = texture_copy_location.Subresource.ArraySlice;
			vk_buffer_image_copy.imageSubresource.layerCount = 1; texture_copy_location.Footprint.Depth;
			vk_buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		vk_buffer_image_copy.imageExtent.width = buffer_texture_copy_location.Footprint.Width;
		vk_buffer_image_copy.imageExtent.height = buffer_texture_copy_location.Footprint.Height;
		vk_buffer_image_copy.imageExtent.depth = buffer_texture_copy_location.Footprint.Depth;

		return vk_buffer_image_copy;
	}

	inline VkImageCopy VKConvertTextureCopyLocationToImageCopy(const RHI::TextureCopyLocation& source_texture_copy_location, const RHI::TextureCopyLocation& destination_texture_copy_location) {
		VkImageCopy vk_image_copy;
		if (source_texture_copy_location.Type == RHI::CopyLocationType::FOOTPRINT) {
			vk_image_copy.srcOffset.x = 0;
			vk_image_copy.srcOffset.y = 0;
			vk_image_copy.srcOffset.z = 0;
			vk_image_copy.srcSubresource.mipLevel = 0;
			vk_image_copy.srcSubresource.baseArrayLayer = 0;
			vk_image_copy.srcSubresource.layerCount = source_texture_copy_location.Footprint.Depth;
			vk_image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else {
			vk_image_copy.srcOffset.x = 0;
			vk_image_copy.srcOffset.y = 0;
			vk_image_copy.srcOffset.z = 0;
			vk_image_copy.srcSubresource.baseArrayLayer = source_texture_copy_location.Subresource.ArraySlice;
			vk_image_copy.srcSubresource.layerCount = 1;
			vk_image_copy.srcSubresource.mipLevel = source_texture_copy_location.Subresource.MipSlice;
			vk_image_copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		if (destination_texture_copy_location.Type == RHI::CopyLocationType::FOOTPRINT) {
			vk_image_copy.dstOffset.x = 0;
			vk_image_copy.dstOffset.y = 0;
			vk_image_copy.dstOffset.z = 0;
			vk_image_copy.dstSubresource.mipLevel = 0;
			vk_image_copy.dstSubresource.baseArrayLayer = 0;
			vk_image_copy.dstSubresource.layerCount = source_texture_copy_location.Footprint.Depth;
			vk_image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		else {
			vk_image_copy.dstOffset.x = 0;
			vk_image_copy.dstOffset.y = 0;
			vk_image_copy.dstOffset.z = 0;
			vk_image_copy.dstSubresource.baseArrayLayer = destination_texture_copy_location.Subresource.ArraySlice;
			vk_image_copy.dstSubresource.layerCount = 1;
			vk_image_copy.dstSubresource.mipLevel = destination_texture_copy_location.Subresource.MipSlice;
			vk_image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		vk_image_copy.extent.width = source_texture_copy_location.Texture->GetDescription().Width >> source_texture_copy_location.Subresource.MipSlice;
		vk_image_copy.extent.height = source_texture_copy_location.Texture->GetDescription().Height >> source_texture_copy_location.Subresource.MipSlice;
		vk_image_copy.extent.depth = source_texture_copy_location.Texture->GetDescription().DepthOrArray;
		return vk_image_copy;
	}
}
#endif