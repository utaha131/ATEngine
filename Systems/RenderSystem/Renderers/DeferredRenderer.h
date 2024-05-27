#ifndef _AT_RENDER_SYSTEM_DEFERRED_RENDERER_H_
#define _AT_RENDER_SYSTEM_DEFERRED_RENDERER_H_
#include "Renderer.h"
//#include "../RenderTechniques.h"
#include "../RenderFeatures.h"
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
			scene->GenerateRenderData(frame_parameters.RenderData);
		}

		void GPUExecution(FrameParameters& frame_parameters, FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, AT::GPUPipelineStateManager& pipeline_state_manager, AT::GPURootSignatureManager& root_signature_manager, RHI::Texture swap_chain_image) override {
			const uint32_t width = 1280, height = 720;
			AT::FrameGraphTextureRef brdf_lut_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16_UNORM, .Width = 512, .Height = 512, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphRTVRef brdf_lut_texture_rtv = graph_builder.CreateRTV(brdf_lut_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::IntegrateBRDF("Main_BRDF_Integration", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 512, 512, frame_parameters.RenderData, brdf_lut_texture_rtv);
			AT::FrameGraphTextureRef depth_texture = graph_builder.CreateTexture({ .Format = RHI::Format::D32_FLOAT_S8X24_UINT, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1, .Clear_Value = RHI::TextureClearValue{.DepthAndStencil = {.Depth = 0.0f }} });
			AT::FrameGraphDSVRef depth_texture_dsv = graph_builder.CreateDSV(depth_texture, { .FirstSliceIndex = 0, .ArraySize = 1 });
			AT::RenderFeatures::DepthPrePass("Main_Depth_PrePass", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, depth_texture_dsv);
			AT::FrameGraphTextureRef skybox_texture = graph_builder.RegisterReadTexture(m_Skybox->GetRHIHandle(), RHI::TextureState::PIXEL_SHADER_RESOURCE);
			AT::FrameGraphSRVRef skybox_texture_srv = graph_builder.CreateSRV(skybox_texture, { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 1 });
			AT::FrameGraphTextureRef scene_color_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphRTVRef scene_color_texture_rtv = graph_builder.CreateRTV(scene_color_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::Skybox("Main_Skybox", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, skybox_texture_srv, scene_color_texture_rtv);
			AT::FrameGraphTextureRef skybox_irradiance_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 128, .Height = 128, .ArraySize = 6, .MipLevels = 1 });
			AT::RenderFeatures::IrradianceMap("Main_Skybox_Irradiance", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 128, 128, frame_parameters.RenderData, skybox_texture_srv, skybox_irradiance_texture);
			AT::FrameGraphTextureRef base_color_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM_SRGB, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphTextureRef normal_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_UNORM, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphTextureRef surface_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphRTVRef base_color_texture_rtv = graph_builder.CreateRTV(base_color_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::FrameGraphRTVRef normal_texture_rtv = graph_builder.CreateRTV(normal_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::FrameGraphRTVRef surface_texture_rtv = graph_builder.CreateRTV(surface_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::GBuffer("Main_GBuffer", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, depth_texture_dsv, base_color_texture_rtv, normal_texture_rtv, surface_texture_rtv);
			AT::FrameGraphSRVRef null_texture_srv = graph_builder.CreateSRV(graph_builder.RegisterReadTexture(m_NullTexture->GetRHIHandle(), RHI::TextureState::PIXEL_SHADER_RESOURCE), { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef null_cube_texture_srv = graph_builder.CreateSRV(graph_builder.RegisterReadTexture(m_NullCubeTexture->GetRHIHandle(), RHI::TextureState::PIXEL_SHADER_RESOURCE), { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 1 });
			std::array<AT::FrameGraphSRVRef, 5> csm_srvs;
			std::array<AT::FrameGraphSRVRef, 5> osm_srvs;
			uint32_t light_count = (std::min)(frame_parameters.RenderData.Lights.size(), 5ull);
			for (uint32_t i = 0u; i < light_count; ++i) {
				switch (frame_parameters.RenderData.Lights[i].Type) {
				case AT::LIGHT_TYPE_DIRECTIONAL:
				{
					AT::FrameGraphTextureRef csm_texture = graph_builder.CreateTexture({ .Format = RHI::Format::D16_UNORM, .Width = 4096, .Height = 4096, .ArraySize = 4, .MipLevels = 1, .Clear_Value = RHI::TextureClearValue{.DepthAndStencil= {.Depth = 0.0f}} });
					AT::RenderFeatures::CascadedShadowMapping("Main_CSM", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 4096, 4096, frame_parameters.RenderData, i, csm_texture);
					csm_srvs[i] = graph_builder.CreateSRV(csm_texture, { .FirstSliceIndex = 0, .ArraySize = 4, .MipLevels = 1 });
					osm_srvs[i] = null_cube_texture_srv;
				}
				break;
				case AT::LIGHT_TYPE_POINT:
				{
					AT::FrameGraphTextureRef osm_texture = graph_builder.CreateTexture({ .Format = RHI::Format::D16_UNORM, .Width = 1024, .Height = 1024, .ArraySize = 6, .MipLevels = 1, .Clear_Value = RHI::TextureClearValue{.DepthAndStencil = {.Depth = 1.0f}} });;
					AT::RenderFeatures::OmnidirectionalShadowMapping("Main_OSM", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1024, 1024, frame_parameters.RenderData, i, osm_texture);
					osm_srvs[i] = graph_builder.CreateSRV(osm_texture, { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 1 });
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
			AT::FrameGraphSRVRef base_color_texture_srv = graph_builder.CreateSRV(base_color_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef normal_texture_srv = graph_builder.CreateSRV(normal_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef surface_texture_srv = graph_builder.CreateSRV(surface_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef depth_texture_srv = graph_builder.CreateSRV(depth_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::RenderFeatures::Deferred("Main_Deferred_Lighting", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, base_color_texture_srv, normal_texture_srv, surface_texture_srv, depth_texture_srv, csm_srvs, osm_srvs, scene_color_texture_rtv);
			AT::FrameGraphTextureRef ssao_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8_UNORM, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphRTVRef ssao_texture_rtv = graph_builder.CreateRTV(ssao_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::SSAO("Main_SSAO", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, 4, m_SSAOResources.NoiseTexture->GetRHIHandle(), m_SSAOResources.KernelData, depth_texture_srv, normal_texture_srv, ssao_texture_rtv);
			AT::FrameGraphTextureRef composition1 = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef scene_color_texture_srv = graph_builder.CreateSRV(scene_color_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef ssao_texture_srv = graph_builder.CreateSRV(ssao_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef skybox_irradiance_texture_srv = graph_builder.CreateSRV(skybox_irradiance_texture, { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 1 });
			AT::FrameGraphUAVRef composition1_uav = graph_builder.CreateUAV(composition1, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::RenderFeatures::DiffuseIndirectPass("Main_Diffuse_Indirect", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, frame_parameters.RenderData, width, height, scene_color_texture_srv, base_color_texture_srv, normal_texture_srv, surface_texture_srv, ssao_texture_srv, skybox_irradiance_texture_srv, composition1_uav);
			AT::FrameGraphTextureRef ssr_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphSRVRef composition1_srv = graph_builder.CreateSRV(composition1, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::RenderFeatures::SSR("Main_SSR", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, composition1_srv, depth_texture_srv, normal_texture_srv, surface_texture_srv, ssr_texture);
			
			AT::FrameGraphSRVRef reflection_probe_texture_srv = null_cube_texture_srv;
			if (frame_parameters.RenderData.ReflectionProbe.has_value()) {
				AT::FrameGraphTextureRef reflection_probe_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 128, .Height = 128, .ArraySize = 6, .MipLevels = 8 });
				AT::RenderFeatures::ReflectionProbe("Main_Reflection_Probe", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 128, 128, frame_parameters.RenderData, frame_parameters.ReflectionProbeRenderData, csm_srvs, osm_srvs, m_SSAOResources.NoiseTexture->GetRHIHandle(), m_SSAOResources.KernelData, skybox_texture_srv, skybox_irradiance_texture_srv, reflection_probe_texture);
				reflection_probe_texture_srv = graph_builder.CreateSRV(reflection_probe_texture, { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 8 });
			}
			AT::FrameGraphSRVRef ssr_texture_srv = graph_builder.CreateSRV(ssr_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphTextureRef composition_final = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 1, .MipLevels = 1 });
			AT::FrameGraphUAVRef composition_final_uav = graph_builder.CreateUAV(composition_final, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::FrameGraphSRVRef brdf_lut_texture_srv = graph_builder.CreateSRV(brdf_lut_texture, { .FirstSliceIndex = 0, .ArraySize = 1, . MipLevels = 1 });
			AT::RenderFeatures::ReflectionPass("Main_Reflection", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, frame_parameters.RenderData, composition1_srv, base_color_texture_srv, normal_texture_srv, surface_texture_srv, depth_texture_srv, ssao_texture_srv, ssr_texture_srv, brdf_lut_texture_srv, reflection_probe_texture_srv, composition_final_uav);
			AT::FrameGraphTextureRef final_image = graph_builder.RegisterWriteTexture(swap_chain_image);
			AT::FrameGraphRTVRef final_image_rtv = graph_builder.CreateRTV(final_image, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			AT::FrameGraphSRVRef composition_final_srv = graph_builder.CreateSRV(composition_final, {.FirstSliceIndex = 0, .ArraySize = 1, .MipLevels = 1});
			AT::RenderFeatures::ToneMapping("Main_Tone_Mapping", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, 1280, 720, frame_parameters.RenderData, composition_final_srv, final_image_rtv);
			
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