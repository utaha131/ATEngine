#ifndef _AT_RENDER_SYSTEM_DEFERRED_RENDERER_H_
#define _AT_RENDER_SYSTEM_DEFERRED_RENDERER_H_
#include "Renderer.h"
#include "../RenderTechniques.h"
#include "../GPUResourceManager.h"
#include "../GPUConstantBuffer.h"
#include <random>

namespace AT {
	class DeferredRenderer : public Renderer {
	public:
		DeferredRenderer(AT::GPUResourceManager& resource_manager) {
			GPUResourceUploadBatch upload_batch;
			GPUResourceTransitionBatch transition_batch;
			//Create Null Texture2D
			{
				RHI::TextureDescription texture_description;
				texture_description.Format = RHI::Format::R8G8B8A8_UNORM;
				texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
				texture_description.Width = 1;
				texture_description.Height = 1;
				texture_description.DepthOrArray = 1;
				texture_description.MipLevels = 1;
				texture_description.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE;
				m_NullTexture =  resource_manager.CreateTexture(texture_description);
				transition_batch.AddTextureTransition(m_NullTexture, RHI::TextureState::PIXEL_SHADER_RESOURCE);
			}
			//Create Null TextureCube
			{
				{
					RHI::TextureDescription texture_description;
					texture_description.Format = RHI::Format::R8G8B8A8_UNORM;
					texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
					texture_description.Width = 1;
					texture_description.Height = 1;
					texture_description.DepthOrArray = 6;
					texture_description.MipLevels = 1;
					texture_description.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE;
					m_NullCubeTexture = resource_manager.CreateTexture(texture_description);
					transition_batch.AddTextureTransition(m_NullCubeTexture, RHI::TextureState::PIXEL_SHADER_RESOURCE);
				}
			}
			//Create SSAO Resources.
			{
				const float noise_size = 4;
				std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
				std::default_random_engine random_engine;
				RHI::TextureDescription test_noise_texture_description = {
					.TextureType = RHI::TextureType::TEXTURE_2D,
					.Format = RHI::Format::R32G32B32A32_FLOAT,
					.Width = (uint32_t)noise_size,
					.Height = (uint32_t)noise_size,
					.DepthOrArray = 1,
					.MipLevels = 1,
					.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE,
				};
				uint32_t pitch = ceil(noise_size * 4.0f * sizeof(float) / resource_manager.GetDevice()->GetTexturePitchAlignment()) * resource_manager.GetDevice()->GetTexturePitchAlignment();
				std::vector<float> noise_data = std::vector<float>((pitch / sizeof(float)) * noise_size);
				for (uint32_t i = 0; i < noise_size; ++i) {
					for (uint32_t j = 0; j < noise_size; ++j) {
						unsigned int index = i * (pitch / sizeof(float)) + j * 4;
						DirectX::XMVECTOR xm_noise_vector = DirectX::XMVectorSet(
							random_floats(random_engine) * 2.0f - 1.0f,
							random_floats(random_engine) * 2.0f - 1.0f,
							0.0f,
							0.0f
						);
						xm_noise_vector = DirectX::XMVector3Normalize(xm_noise_vector);
						DirectX::XMFLOAT4 noise_vector;
						DirectX::XMStoreFloat4(&noise_vector, xm_noise_vector);
						noise_data[index] = noise_vector.x;
						noise_data[index + 1] = noise_vector.y;
						noise_data[index + 2] = 0.0f;
						noise_data[index + 3] = 0.0f;
					}
				}

				RHI::TextureDescription noise_texture_description = {
					.TextureType = RHI::TextureType::TEXTURE_2D,
					.Format = RHI::Format::R32G32B32A32_FLOAT,
					.Width = (uint32_t)noise_size,
					.Height = (uint32_t)noise_size,
					.DepthOrArray = 1,
					.MipLevels = 1,
					.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE,
				};
				m_SSAOResources.NoiseTexture = resource_manager.CreateTexture(noise_texture_description);

				RHI::BufferDescription staging_buffer_description;
				staging_buffer_description.Size = noise_data.size() * sizeof(float);
				staging_buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
				GPUBufferPtr staging_buffer = resource_manager.CreateUploadBuffer(staging_buffer_description);
				staging_buffer->GetRHIHandle()->Map();
				staging_buffer->GetRHIHandle()->CopyData(0, noise_data.data(), noise_data.size() * sizeof(float));
				upload_batch.AddTextureUpload(m_SSAOResources.NoiseTexture, staging_buffer);
				transition_batch.AddTextureTransition(m_SSAOResources.NoiseTexture, RHI::TextureState::COMMON);
				// noise_data.data(), noise_data.size() * sizeof(float)
				for (uint32_t i = 0; i < 64; ++i) {

					DirectX::XMFLOAT3 sample = DirectX::XMFLOAT3();
					DirectX::XMVECTOR sample_vector = DirectX::XMVectorSet(
						random_floats(random_engine) * 2.0f - 1.0f,
						random_floats(random_engine) * 2.0f - 1.0f,
						random_floats(random_engine),
						0.0f
					);

					sample_vector = DirectX::XMVector3Normalize(sample_vector);
					sample_vector = DirectX::XMVectorScale(sample_vector, random_floats(random_engine));
					float scale = (float)i / 64.0f;
					scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
					sample_vector = DirectX::XMVectorScale(sample_vector, scale);
					DirectX::XMStoreFloat3(&sample, sample_vector);
					m_SSAOResources.KernelData.emplace_back(DirectX::XMFLOAT4A(sample.x, sample.y, sample.z, 0.0f));
				}
			
				
			}
			//Load Skybox 
			{
				std::vector<std::string> sky_box_sources = {
					"./Assets/Skyboxes/right.jpg",
					"./Assets/Skyboxes/left.jpg",
					"./Assets/Skyboxes/top.jpg",
					"./Assets/Skyboxes/bottom.jpg",
					"./Assets/Skyboxes/front.jpg",
					"./Assets/Skyboxes/back.jpg",
				};

				RHI::TextureDescription skybox_description;
				uint32_t skybox_size = 2048;
				skybox_description.Format = RHI::Format::R8G8B8A8_UNORM;
				skybox_description.TextureType = RHI::TextureType::TEXTURE_2D;
				skybox_description.Width = skybox_size;
				skybox_description.Height = skybox_size;
				skybox_description.DepthOrArray = 6;
				skybox_description.MipLevels = 1;
				skybox_description.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE;
				m_Skybox = resource_manager.CreateTexture(skybox_description);
				transition_batch.AddTextureTransition(m_Skybox, RHI::TextureState::COMMON);
				AT::GPUBufferPtr staging_texture;
				RHI::BufferDescription buffer_description;
				buffer_description.Size = ((skybox_size + 255) & ~255) * skybox_size * 4 * 6;
				buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
				staging_texture = resource_manager.CreateUploadBuffer(buffer_description);
				staging_texture->GetRHIHandle()->Map();
				for (uint32_t i = 0; i < sky_box_sources.size(); ++i) {
					AT::Image image = AT::Image(sky_box_sources[i], false);
					staging_texture->GetRHIHandle()->CopyData(((skybox_size + 255) & ~255) * skybox_size * 4 * i, image.GetBytes(), image.GetDataSize());
				}
				upload_batch.AddTextureUpload(m_Skybox, staging_texture);
			}
			resource_manager.ExecuteTransitions(transition_batch);
			resource_manager.UploadBatches(upload_batch);
			resource_manager.WaitForIdle();
			{
				GPUResourceTransitionBatch final_transition_batch;;
				final_transition_batch.AddTextureTransition(m_Skybox, RHI::TextureState::PIXEL_SHADER_RESOURCE);
				final_transition_batch.AddTextureTransition(m_SSAOResources.NoiseTexture, RHI::TextureState::PIXEL_SHADER_RESOURCE);
				resource_manager.ExecuteTransitions(final_transition_batch);
			}
			resource_manager.WaitForIdle();
		}
		void RenderLogic(FrameParameters& frame_parameters, Scene* scene) override {
			//frame_parameters.Render_Info = scene->GenerateRenderInfo();
			scene->GenerateRenderData(frame_parameters.RenderData);
		}

		void GPUExecution(FrameParameters& frame_parameters, FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, AT::GPUPipelineStateManager& pipeline_state_manager, AT::GPURootSignatureManager& root_signature_manager, RHI::Texture swap_chain_image) override {
			AT::FrameGraphTextureRef depth_buffer = graph_builder.CreateTexture({
			.Format = RHI::Format::D32_FLOAT_S8X24_UINT,
			.Width = 1280,
			.Height = 720,
			.ArraySize = 1,
				});

			RHI::TextureClearValue clear_value = { .DepthAndStencil = {.Depth = 0.0f, .Stencil = 0 } };
			depth_buffer->Description.Clear_Value = std::optional<RHI::TextureClearValue>(clear_value);

			AT::FrameGraphDSVRef depth_buffer_dsv = graph_builder.CreateDSV(depth_buffer, { .FirstSliceIndex = 0, .ArraySize = 1 });

			AT::RenderTechniques::DepthPass(
				"Main_Depth_Pass",
				graph_builder,
				shader_manager,
				pipeline_state_manager,
				root_signature_manager,
				1280,
				720,
				frame_parameters.RenderData,
				depth_buffer_dsv
			);

			AT::FrameGraphTextureRef output_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .ArraySize = 1 });
			AT::FrameGraphSRVRef skybox_texture = graph_builder.CreateSRV(graph_builder.RegisterReadTexture(m_Skybox->GetRHIHandle(), RHI::TextureState::PIXEL_SHADER_RESOURCE), {.FirstSliceIndex = 0, .ArraySize = 6});
			AT::RenderTechniques::Skybox(
				"Main_Skybox_Pass",
				graph_builder,
				shader_manager,
				pipeline_state_manager,
				root_signature_manager,
				frame_parameters.RenderData,
				1280,
				720,
				skybox_texture,
				graph_builder.CreateRTV(output_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 }));


			AT::FrameGraphTextureRef base_color = graph_builder.CreateTexture({
				.Format = RHI::Format::R8G8B8A8_UNORM_SRGB,
				.Width = 1280,
				.Height = 720,
				.ArraySize = 1
				});
			AT::FrameGraphTextureRef normal = graph_builder.CreateTexture({
				.Format = RHI::Format::R16G16B16A16_UNORM,
				.Width = 1280,
				.Height = 720,
				.ArraySize = 1
				});
			AT::FrameGraphTextureRef surface = graph_builder.CreateTexture({
				.Format = RHI::Format::R8G8B8A8_UNORM,
				.Width = 1280,
				.Height = 720,
				.ArraySize = 1
				});

			AT::RenderTechniques::GBuffer(
				"GBuffer_Pass",
				graph_builder,
				shader_manager,
				pipeline_state_manager,
				root_signature_manager,
				1280,
				720,
				frame_parameters.RenderData,
				graph_builder.CreateRTV(base_color, {}),
				graph_builder.CreateRTV(normal, {}),
				graph_builder.CreateRTV(surface, {}),
				depth_buffer_dsv);

			AT::FrameGraphSRVRef null_texture_srv = graph_builder.CreateSRV(graph_builder.RegisterReadTexture(m_NullTexture->GetRHIHandle(), RHI::TextureState::PIXEL_SHADER_RESOURCE), { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef null_cube_texture_srv = graph_builder.CreateSRV(graph_builder.RegisterReadTexture(m_NullCubeTexture->GetRHIHandle(), RHI::TextureState::PIXEL_SHADER_RESOURCE), { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 1 });
			std::array<AT::FrameGraphSRVRef, 5> csm_srvs;
			std::array<AT::FrameGraphSRVRef, 5> osm_srvs;
			uint32_t light_count = (std::min)(frame_parameters.RenderData.Lights.size(), 5ull);
			for (uint32_t i = 0u; i < light_count; ++i) {
				switch (frame_parameters.RenderData.Lights[i].Type) {
				case AT::LIGHT_TYPE_DIRECTIONAL:
				{
					csm_srvs[i] = graph_builder.CreateSRV(AT::RenderTechniques::CSM("CSM_Light[" + std::to_string(i) + "]", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, frame_parameters.RenderData, i, 4u), { .FirstSliceIndex = 0, .ArraySize = 4, .MipLevels = 1 });
					osm_srvs[i] = null_cube_texture_srv;
				}
				break;
				case AT::LIGHT_TYPE_POINT:
				{
					osm_srvs[i] = graph_builder.CreateSRV(AT::RenderTechniques::OSM("OSM_Light[" + std::to_string(i) + "]", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, frame_parameters.RenderData, i), { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 1 });
					csm_srvs[i] = null_texture_srv;
				}
				break;
				}
			}

			for (uint32_t i = light_count; i < 5u; ++i) {
				osm_srvs[i] = null_cube_texture_srv;
				csm_srvs[i] = null_texture_srv;
				frame_parameters.RenderData.Lights.push_back({});
			}

			AT::FrameGraphTextureRef SSAO_out = graph_builder.CreateTexture({
				.Format = RHI::Format::R8_UNORM,
				.Width = 1280,
				.Height = 720,
				.ArraySize = 1,
				.MipLevels = 1
				});

			AT::FrameGraphSRVRef normal_srv = graph_builder.CreateSRV(normal, {});
			AT::FrameGraphSRVRef depth_buffer_srv = graph_builder.CreateSRV(depth_buffer_dsv->TextureRef, {});

			AT::RenderTechniques::SSAO("Main_SSAO_Pass", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, graph_builder.CreateRTV(SSAO_out, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 }), depth_buffer_srv, normal_srv, 4, m_SSAOResources.NoiseTexture->GetRHIHandle(), m_SSAOResources.KernelData);

			AT::FrameGraphSRVRef prefiltered = null_cube_texture_srv;
			AT::FrameGraphSRVRef brdf_lut = null_texture_srv;

			if (frame_parameters.RenderData.ReflectionProbe.has_value()) {
				AT::RenderTechniques::ReflectionProbeOut output = AT::RenderTechniques::ReflectionProbe("ReflectionProbe", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 128, 128, frame_parameters.RenderData, csm_srvs, osm_srvs, null_texture_srv, null_cube_texture_srv, m_SSAOResources.NoiseTexture->GetRHIHandle(), m_SSAOResources.KernelData, frame_parameters.ReflectionProbeRenderData, skybox_texture);
				prefiltered = graph_builder.CreateSRV(output.prefiltered_map, { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 7 });
				brdf_lut = graph_builder.CreateSRV(output.brdf_lut, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			}

			AT::FrameGraphTextureRef deferred_output = AT::RenderTechniques::Deferred(
				"Main_Deferred_Pass",
				graph_builder,
				shader_manager,
				pipeline_state_manager,
				root_signature_manager,
				1280,
				720,
				frame_parameters.RenderData,
				graph_builder.CreateRTV(output_texture, { .FirstSliceIndex = 0, .ArraySize = 1 }),
				graph_builder.CreateSRV(base_color, {}),
				normal_srv,
				graph_builder.CreateSRV(surface, {}),
				depth_buffer_srv,
				graph_builder.CreateSRV(SSAO_out, {}),
				csm_srvs,
				osm_srvs,
				null_cube_texture_srv,//irradiance,
				prefiltered,
				brdf_lut
			);

			AT::FrameGraphTextureRef fg_swap_chain_image = graph_builder.RegisterWriteTexture(swap_chain_image);
			AT::RenderTechniques::ToneMapping(graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, deferred_output, fg_swap_chain_image);
			graph_builder.Compile();
			graph_builder.Execute();
		}
	private:
		AT::GPUTexturePtr m_Skybox, m_NullTexture, m_NullCubeTexture;
		struct SSAOResources {
			GPUTexturePtr NoiseTexture;
			std::vector<DirectX::XMFLOAT4A> KernelData;
		} m_SSAOResources;
	};
}
#endif