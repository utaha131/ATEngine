#include "Renderer.h"
#include "../GPUResourceManager.h"

namespace AT {
	class PathTracer : public Renderer {
	public:
		PathTracer(AT::GPUResourceManager& resource_manager, RHI::ShaderResourceView instance_info_srv) {
			RHI::BufferDescription buffer_description;
			buffer_description.Size = 512;
			buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
			/*m_STBBuffer = resource_manager.CreateUploadBuffer(buffer_description);

			m_STBBuffer2 = resource_manager.CreateUploadBuffer(buffer_description);

			m_STBBuffer3 = resource_manager.CreateUploadBuffer(buffer_description);

			m_STBBuffer4 = resource_manager.CreateUploadBuffer(buffer_description);*/
			for (uint32_t i = 0u; i < 8u; ++i) {
				m_STBBuffer[i] = resource_manager.CreateUploadBuffer(buffer_description);
			}

			RHI::TextureDescription texture_description;
			texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
			texture_description.Format = RHI::Format::R32G32B32A32_FLOAT;
			texture_description.Width = 1280;
			texture_description.Height = 720;
			texture_description.DepthOrArray = 1;
			texture_description.MipLevels = 1;
			texture_description.UsageFlags = RHI::TextureUsageFlag::UNORDERED_ACCESS | RHI::TextureUsageFlag::SHADER_RESOURCE;

			for (uint32_t i = 0u; i < 2; ++i) {
				m_ReservoirTextures[i] = resource_manager.CreateTexture(texture_description);

				for (uint32_t j = 0u; j < 4; ++j) {
					m_GI_ReservoirTextures[j][i] = resource_manager.CreateTexture(texture_description);
				}
			}

			m_InstanceInfoSRV = instance_info_srv;
		}

		void RenderLogic(FrameParameters& frame_parameters, Scene* scene) {
			scene->GenerateRenderData(frame_parameters.RenderData);
		}

		void GPUExecution(FrameParameters& frame_parameters, FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, AT::GPUPipelineStateManager& pipeline_state_manager, AT::GPURootSignatureManager& root_signature_manager, RHI::Texture swap_chain_image) override {
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



			//FrameGraphTextureRef di_reservoir_buffer = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef di_reservoir_buffer_uav = graph_builder.CreateUAV(di_reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			//AT::RenderFeatures::ReSTIRGenerateSamples("ReSTIR_DI_InitialSamples", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[0]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,
			//	di_reservoir_buffer_uav
			//);

			//FrameGraphTextureRef di_previous_reservoir_buffer = graph_builder.RegisterWriteTexture(m_ReservoirTextures[frame_parameters.FrameNumber % 2]->GetRHIHandle());
			//FrameGraphUAVRef di_previous_reservoir_buffer_uav = graph_builder.CreateUAV(di_previous_reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef di_temporal_reservoir_buffer = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef di_temporal_reservoir_buffer_uav = graph_builder.CreateUAV(di_temporal_reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//AT::RenderFeatures::ReSTIRTemporalSamples("ReSTIR_DI_TemporalSamples", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[1]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,
			//	di_previous_reservoir_buffer_uav,
			//	di_reservoir_buffer_uav,
			//	di_temporal_reservoir_buffer_uav
			//);

			//FrameGraphTextureRef di_spatial_reservoir_buffer = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef di_spatial_reservoir_buffer_uav = graph_builder.CreateUAV(di_spatial_reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//AT::RenderFeatures::ReSTIRSpatialSamples("ReSTIR_DI_SpatialSamples", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[2]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,
			//	di_temporal_reservoir_buffer_uav,
			//	di_spatial_reservoir_buffer_uav
			//);

			//FrameGraphTextureRef di_output = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef di_uav = graph_builder.CreateUAV(di_output, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//AT::RenderFeatures::ReSTIRFinalShade("ReSTIR_DI_FinalShading", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[3]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,
			//	di_previous_reservoir_buffer_uav,
			//	di_spatial_reservoir_buffer_uav,
			//	di_uav
			//);


			////ReSTIR GI Test.
			//
			//FrameGraphTextureRef reservoir_buffer = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef reservoir_buffer_uav = graph_builder.CreateUAV(reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef reservoir_buffer1 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef reservoir_buffer1_uav = graph_builder.CreateUAV(reservoir_buffer1, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef reservoir_buffer2 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef reservoir_buffer2_uav = graph_builder.CreateUAV(reservoir_buffer2, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef reservoir_buffer3 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef reservoir_buffer3_uav = graph_builder.CreateUAV(reservoir_buffer3, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//AT::RenderFeatures::ReSTIR_GI_GenerateSamples(
			//	"ReSTIR_GI_InitialSampling", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[4]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,
			//	reservoir_buffer_uav,
			//	reservoir_buffer1_uav,
			//	reservoir_buffer2_uav,
			//	reservoir_buffer3_uav
			//);

			//FrameGraphTextureRef temporal_reservoir_buffer = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef temporal_reservoir_buffer_uav = graph_builder.CreateUAV(temporal_reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef temporal_reservoir_buffer1 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef temporal_reservoir_buffer1_uav = graph_builder.CreateUAV(temporal_reservoir_buffer1, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef temporal_reservoir_buffer2 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef temporal_reservoir_buffer2_uav = graph_builder.CreateUAV(temporal_reservoir_buffer2, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef temporal_reservoir_buffer3 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef temporal_reservoir_buffer3_uav = graph_builder.CreateUAV(temporal_reservoir_buffer3, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef previous_reservoir_buffer[4];
			//FrameGraphUAVRef previous_reservoir_buffer_uav[4];

			//for (uint32_t i = 0u; i < 4u; ++i) {
			//	previous_reservoir_buffer[i] = graph_builder.RegisterWriteTexture(m_GI_ReservoirTextures[i][frame_parameters.RenderData.FrameNumber % 2]->GetRHIHandle());
			//	previous_reservoir_buffer_uav[i] = graph_builder.CreateUAV(previous_reservoir_buffer[i], { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			//}

			//AT::RenderFeatures::ReSTIR_GI_TemporalSampling(
			//	"ReSTIR_GI_TemporalSampling", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[5]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,

			//	temporal_reservoir_buffer_uav,
			//	temporal_reservoir_buffer1_uav,
			//	temporal_reservoir_buffer2_uav,
			//	temporal_reservoir_buffer3_uav,

			//	reservoir_buffer_uav,
			//	reservoir_buffer1_uav,
			//	reservoir_buffer2_uav,
			//	reservoir_buffer3_uav,

			//	previous_reservoir_buffer_uav[0],
			//	previous_reservoir_buffer_uav[1],
			//	previous_reservoir_buffer_uav[2],
			//	previous_reservoir_buffer_uav[3]
			//);

			//FrameGraphTextureRef spatial_reservoir_buffer = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef spatial_reservoir_buffer_uav = graph_builder.CreateUAV(spatial_reservoir_buffer, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef spatial_reservoir_buffer1 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef spatial_reservoir_buffer1_uav = graph_builder.CreateUAV(spatial_reservoir_buffer1, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef spatial_reservoir_buffer2 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef spatial_reservoir_buffer2_uav = graph_builder.CreateUAV(spatial_reservoir_buffer2, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//FrameGraphTextureRef spatial_reservoir_buffer3 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 4, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef spatial_reservoir_buffer3_uav = graph_builder.CreateUAV(spatial_reservoir_buffer3, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });

			//AT::RenderFeatures::ReSTIR_GI_SpatialSampling(
			//	"ReSTIR_GI_SpatialSampling", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[6]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,

			//	spatial_reservoir_buffer_uav,
			//	spatial_reservoir_buffer1_uav,
			//	spatial_reservoir_buffer2_uav,
			//	spatial_reservoir_buffer3_uav,

			//	temporal_reservoir_buffer_uav,
			//	temporal_reservoir_buffer1_uav,
			//	temporal_reservoir_buffer2_uav,
			//	temporal_reservoir_buffer3_uav
			//);
			//

			//FrameGraphTextureRef gi_output = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphUAVRef gi_uav = graph_builder.CreateUAV(gi_output, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });


			//AT::RenderFeatures::ReSTIR_GI_FinalShading(
			//	"ReSTIR_GI_FinalShading", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[7]->GetRHIHandle(), m_InstanceInfoSRV,
			//	base_color_texture_srv,
			//	normal_texture_srv,
			//	surface_texture_srv,
			//	depth_texture_srv,
			//	gi_uav,
			//	spatial_reservoir_buffer_uav,
			//	spatial_reservoir_buffer1_uav,
			//	spatial_reservoir_buffer2_uav,
			//	spatial_reservoir_buffer3_uav,
			//	previous_reservoir_buffer_uav[0],
			//	previous_reservoir_buffer_uav[1],
			//	previous_reservoir_buffer_uav[2],
			//	previous_reservoir_buffer_uav[3]
			//);

			//FrameGraphTextureRef output = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			//FrameGraphRTVRef rtv = graph_builder.CreateRTV(output, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			//FrameGraphSRVRef di_srv = graph_builder.CreateSRV(di_output, { .FirstSliceIndex = 0, .ArraySize = 1, .MostDetailedMip = 0, .MipLevels = 1 });
			//FrameGraphSRVRef gi_srv = graph_builder.CreateSRV(gi_output, { .FirstSliceIndex = 0, .ArraySize = 1, .MostDetailedMip = 0, .MipLevels = 1 });
			//AT::RenderFeatures::ReSTIRComposite("ReSTIRComposite", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, di_srv, gi_srv, rtv);

			//Path Tracer

			FrameGraphTextureRef output = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1, .MipLevels = 1 });//graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::PRESENT);
			FrameGraphUAVRef uav = graph_builder.CreateUAV(output, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });


			AT::RenderFeatures::RayTraceTest("RayTrace", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, m_STBBuffer[0]->GetRHIHandle(), m_InstanceInfoSRV,
				base_color_texture_srv,
				normal_texture_srv,
				surface_texture_srv,
				depth_texture_srv,
				uav
			);

			FrameGraphTextureRef sc_texture = graph_builder.RegisterReadTexture(swap_chain_image, RHI::TextureState::COMMON);
			FrameGraphSRVRef output_srv = graph_builder.CreateSRV(output, {.FirstSliceIndex = 0, .ArraySize = 1, .MostDetailedMip = 0, .MipLevels = 1});
			FrameGraphRTVRef sc_rtv = graph_builder.CreateRTV(sc_texture, { .FirstSliceIndex = 0, .ArraySize =  1, .MipSlice = 0 });
			AT::RenderFeatures::ToneMapping("ToneMapping", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, output_srv, sc_rtv);
			graph_builder.Compile();
			graph_builder.Execute();
		}
	private:
		AT::GPUBufferPtr m_STBBuffer[8];
		AT::GPUTexturePtr m_ReservoirTextures[2];
		AT::GPUTexturePtr m_GI_ReservoirTextures[4][2];
		RHI::ShaderResourceView m_InstanceInfoSRV;
	};
}