#include "Renderer.h"
#include "../GPUResourceManager.h"

namespace AT {
	class PathTracer : public Renderer {
	public:
		PathTracer(AT::GPUResourceManager& resource_manager, RHI::ShaderResourceView instance_info_srv) {
			RHI::BufferDescription buffer_description;
			buffer_description.Size = 4096;
			buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
			m_STBBuffer = resource_manager.CreateUploadBuffer(buffer_description);

			m_InstanceInfoSRV = instance_info_srv;
		}

		void RenderLogic(FrameParameters& frame_parameters, Scene* scene) {
			scene->GenerateRenderData(frame_parameters.RenderData);
		}

		void GPUExecution(FrameParameters& frame_parameters, FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, AT::GPUPipelineStateManager& pipeline_state_manager, AT::GPURootSignatureManager& root_signature_manager, RHI::Texture swap_chain_image) override {
			/*RHI::BufferDescription buffer_description;
			buffer_description.Size = 4096;
			buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
			RHI::Buffer sbt_buffer;
			graph_builder.GetDevice()->CreateCommittedBuffer(RHI::ResourceHeapType::UPLOAD, RHI::BufferState::COMMON, buffer_description, sbt_buffer);*/
			uint32_t width = 1280, height = 720;
			AT::FrameGraphTextureRef depth_texture = graph_builder.CreateTexture({ .Format = RHI::Format::D32_FLOAT_S8X24_UINT, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1, .Clear_Value = RHI::TextureClearValue{.DepthAndStencil = {.Depth = 0.0f }} });
			AT::FrameGraphDSVRef depth_texture_dsv = graph_builder.CreateDSV(depth_texture, { .FirstSliceIndex = 0, .ArraySize = 1 });
			AT::FrameGraphTextureRef base_color_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM_SRGB, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::RenderFeatures::DepthPrePass("DepthPrepass", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, depth_texture_dsv);
			AT::FrameGraphTextureRef normal_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_UNORM, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphTextureRef surface_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphRTVRef base_color_texture_rtv = graph_builder.CreateRTV(base_color_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::FrameGraphRTVRef normal_texture_rtv = graph_builder.CreateRTV(normal_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::FrameGraphRTVRef surface_texture_rtv = graph_builder.CreateRTV(surface_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::GBuffer("Main_GBuffer", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, depth_texture_dsv, base_color_texture_rtv, normal_texture_rtv, surface_texture_rtv);

			AT::FrameGraphSRVRef base_color_texture_srv = graph_builder.CreateSRV(base_color_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef normal_texture_srv = graph_builder.CreateSRV(normal_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef surface_texture_srv = graph_builder.CreateSRV(surface_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef depth_texture_srv = graph_builder.CreateSRV(depth_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			FrameGraphTextureRef output = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			FrameGraphUAVRef uav = graph_builder.CreateUAV(output, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::RayTraceTest("RayTrace", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer->GetRHIHandle(), m_InstanceInfoSRV,
				base_color_texture_srv,
				normal_texture_srv,
				surface_texture_srv,
				depth_texture_srv,
				uav);
			FrameGraphTextureRef sc_texture = graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::COMMON);
			FrameGraphSRVRef output_srv = graph_builder.CreateSRV(output, {.FirstSliceIndex = 0, .ArraySize = 1, .MostDetailedMip = 0, .MipLevels = 1});
			FrameGraphRTVRef sc_rtv = graph_builder.CreateRTV(sc_texture, { .FirstSliceIndex = 0, .ArraySize =  1, .MipSlice = 0 });
			AT::RenderFeatures::ToneMapping("ToneMapping", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, output_srv, sc_rtv);
			graph_builder.Compile();
			graph_builder.Execute();
		}
	private:
		AT::GPUBufferPtr m_STBBuffer;
		RHI::ShaderResourceView m_InstanceInfoSRV;
	};
}