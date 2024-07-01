#ifndef _RHI_I_COMMAND_LIST_H_
#define _RHI_I_COMMAND_LIST_H_
#include <vector>
#include <optional>
#include "RHICore.h"

namespace RHI {
	typedef struct BeginRenderPassInfo {
		BeginRenderPassInfo() = default;
		RenderPass RenderPass;
		FrameBuffer FrameBuffer;
		std::vector<TextureClearValue> ClearValues;
		std::optional<TextureClearValue> DepthClearValue;
	} BeginRenderPassInfo;

	enum class CopyLocationType {
		SUBRESOURCE,
		FOOTPRINT
	};

	struct FootPrint {
		Format Format;
		uint64_t Offset;
		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
		uint32_t RowPitch;
	};

	struct TextureCopyLocation {
		Texture Texture;
		CopyLocationType Type;
		union {
			FootPrint Footprint = {};
			Subresource Subresource;
		};
	};

	struct BufferTextureCopyLocation {
		Buffer Buffer;
		CopyLocationType Type;
		union {
			FootPrint Footprint = {};
			Subresource Subresource;
		};
	};

	class ICommandList {
	public:
		virtual ~ICommandList() = default;
		virtual void Reset(const PipelineState pipeline_state) = 0;
		virtual void Close() = 0;
		virtual void ResourceBarrier(uint32_t resource_barrier_count, const ResourceBarrier* resource_barrier) = 0;
		virtual void CopyBuffer(Buffer destination_buffer, Buffer source_buffer) = 0;
		virtual void CopyBufferRegion(Buffer destination_buffer, uint64_t destination_offset, Buffer source_buffer, uint64_t source_offset, uint64_t size) = 0;
		virtual void CopyBufferToTexture(TextureCopyLocation& dest_texture_region, BufferTextureCopyLocation& source_buffer_region) = 0;
		virtual void CopyTexture(Texture destination_texture, Texture source_texture) = 0;
		virtual void CopyTextureRegion(TextureCopyLocation& dest_texture_region, TextureCopyLocation& source_texture_region) = 0;
		virtual void SetGraphicsRootSignature(const RootSignature root_signature) = 0;
		virtual void SetPipelineState(const PipelineState pipeline_state) = 0;
		virtual void SetVertexBuffer(uint32_t start_slot, uint32_t num_views, const VertexBufferView* p_vertex_buffer_views) = 0;
		virtual void SetIndexBuffer(const IndexBufferView& index_buffer_view) = 0;
		virtual void SetViewport(const Viewport& viewport) = 0;
		virtual void SetScissorRect(const Rect& scissor_rect) = 0;
		virtual void SetRootConstants(uint32_t root_parameter_index, uint32_t constants_count, const void* data, uint32_t offset) = 0;
		virtual void SetGraphicsRootConstantBufferView(uint32_t root_parameter_index, ConstantBufferView constant_buffer_View) = 0;
		virtual void SetGraphicsRootShaderResourceView(uint32_t root_parameter_index, ShaderResourceView shader_resource_view) = 0;
		virtual void SetGraphicsRootUnorderedAccessView(uint32_t root_parameter_index, UnorderedAccessView unordered_access_view) = 0;
		virtual void SetPrimitiveTopology(PrimitiveTopology primitive_topology) = 0;
		virtual void Draw(uint32_t vertices, uint32_t instances, uint32_t start_vertex, uint32_t start_instance) = 0;
		virtual void DrawIndexed(uint32_t indices, uint32_t instance_count, uint32_t start_index, uint32_t start_vertex, uint32_t start_instance) = 0;
		virtual void BeginRenderPass(const BeginRenderPassInfo& begin_render_pass_info) = 0;
		virtual void EndRenderPass() = 0;

		virtual void SetComputeRootSignature(const RootSignature root_signature) = 0;
		virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;

		virtual void SetGraphicsRootDescriptorTable(uint32_t root_parameter_index, const DescriptorTable table) = 0;
		virtual void SetComputeRootDescriptorTable(uint32_t root_parameter_index, const DescriptorTable table) = 0;

		//RT
		virtual void SetRayTracingPipeline(IRayTracingPipeline* pipeline) = 0;
		virtual void DispatchRays(const RayTracingDispatchRaysDescription& dispatch_rays_description) = 0;

		virtual void BuildRaytracingBottomLevelAccelerationStructure(const BuildRayTracingBottomLevelAccelerationStructure& build_bottom_level_acceleration_structure) const = 0;
		virtual void BuildRaytracingTopLevelAccelerationStructure(const BuildRayTracingTopLevelAccelerationStructure& build_top_level_acceleration_structure) const = 0;
	protected:
		ICommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator) :
			m_CommandType(command_type),
			m_CommandAllocator(command_allocator)
		{};
		RHI::CommandType m_CommandType;
		RHI::CommandAllocator m_CommandAllocator;
		RootSignature m_CurrentRootSignature = RHI_NULL_HANDLE;
		BeginRenderPassInfo m_CurrentRenderPassInfo = {};
	};
};
#endif