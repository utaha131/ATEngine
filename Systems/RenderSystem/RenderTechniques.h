#ifndef _AT_RENDER_SYSTEM_RENDER_TECHNIQUES_H_
#define _AT_RENDER_SYSTEM_RENDER_TECHNIQUES_H_

#include "../ResourceManagers/SceneManager.h"
#include "FrameGraphBuilder.h"
#include "GPUShaderManager.h"
#include "GPUPipelineStateManager.h"
#include "GPURootSignatureManager.h"
#include "ShaderPrograms/DepthPrepassShader.h"
#include "ShaderPrograms/GBufferShader.h"
#include "ShaderPrograms/DeferredShader.h"
#include "ShaderPrograms/CascadedShadowMappingShader.h"
#include "ShaderPrograms/OmnidirectionalShadowMappingShader.h"
#include "ShaderPrograms/ScreenSpaceAmbientOcclusionShader.h"
#include "ShaderPrograms/ScreenSpaceAmbientOcclusionBlurShader.h"
#include "ShaderPrograms/ToneMappingShader.h"
#include "ShaderPrograms/SkyboxShader.h"
#include "ShaderPrograms/IrradianceMap.h"
#include "ShaderPrograms/PreFilterEnviromentMapShader.h"
#include "ShaderPrograms/EnvBRDFShader.h"
#include "ShaderPrograms/GenerateMipChain2DShader.h"
#include "GPUResourceManager.h"
#include <random>

namespace AT {

	RHI::ShaderDescription GLSLDescription(const std::string& path, RHI::ShaderType type) {
		RHI::ShaderDescription description;
		description.SourcePath = path;
		description.ShaderType = type;
		description.EntryPoint = "main";
		return description;
	}

	RHI::ShaderDescription HLSLDescription(const std::string& path, RHI::ShaderType type) {
		RHI::ShaderDescription description;
		description.SourcePath = path;
		description.ShaderType = type;
		switch (type) {
		case RHI::ShaderType::VERTEX:
			description.EntryPoint = "VS";
			break;
		case RHI::ShaderType::PIXEL:
			description.EntryPoint = "PS";
			break;
		}
		return description;
	}

	namespace RenderTechniques {
		inline void DepthPass(const std::string& name,
							  AT::FrameGraphBuilder& graph_builder, 
							  AT::GPUShaderManager& shader_manager,
							  AT::GPUPipelineStateManager& pipeline_state_manager,
							  AT::GPURootSignatureManager& root_signature_manager,
							  uint32_t width,
							  uint32_t height,
							  const AT::RenderData& render_data,
							  FrameGraphDSVRef dsv) {

			AT::DepthPrepassShader shader = AT::DepthPrepassShader(shader_manager.LoadRHIShader("Depth", RHI::ShaderType::VERTEX), 
																	shader_manager.LoadRHIShader("Depth", RHI::ShaderType::PIXEL),
																	root_signature_manager);
			AT::GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder
				.SetInputLayout(AT::VertexFormat::input_layout)
				.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
				.SetDepthFormat(RHI::Format::D32_FLOAT_S8X24_UINT)
				.SetDepthEnabled(true)
				.SetDepthStencilComparisonFunction(RHI::ComparisonFunction::GREATER_EQUAL)
				.SetDepthWriteMask(RHI::DepthWriteMask::ALL)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);

			AT::FrameGraphRenderPassIO IO = {};
			IO.SetDepthStencilBuffer(dsv, RHI::RenderPassAttachment::LoadOperation::CLEAR);

			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			graph_builder.AddRenderPass(name, IO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso);
				command_list->SetGraphicsRootSignature(shader.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = width;
				viewport.Height = height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = height;
				scissor_rect.Right = width;
				command_list->SetScissorRect(scissor_rect);

				AT::DepthPrepassShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::DepthPrepassShader::Parameters>();
				std::vector<AT::DepthPrepassShader::ObjectGroup*> object_groups = graph_builder.AllocateShaderParameterGroup<AT::DepthPrepassShader::ObjectGroup>(shader.GetRootSignature(), 0, render_data.RenderObjects.size());

				for (uint32_t i = 0; i < render_data.RenderObjects.size(); ++i) {
					parameters.object = object_groups[i];
					parameters.object->constant_buffer = graph_builder.AllocateConstantBuffer<AT::DepthPrepassShader::ObjectGroup::Constants>();
					const AT::RenderObject& render_object = render_data.RenderObjects[i];
					const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];
					DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);

					DirectX::XMStoreFloat4x4A(&parameters.object->constants.MVP_Matrix, DirectX::XMMatrixTranspose(model_matrix) * render_data.ViewProjectionMatrix);
					DirectX::XMStoreFloat4x4A(&parameters.object->constants.Normal_Matrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, model_matrix * render_data.ViewMatrix)));
					parameters.object->BaseColor.srv = render_data.Materials[mesh->MaterialID]->BaseColorSRV;
					shader.SetParameters(command_list, &parameters);
					command_list->SetVertexBuffer(0, 1, &mesh->VBView);
					command_list->SetIndexBuffer(mesh->IBView);
					command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
					command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
				}
			});
		}
		inline void GBuffer(const std::string& name,
							AT::FrameGraphBuilder& graph_builder,
							AT::GPUShaderManager& shader_manager,
							AT::GPUPipelineStateManager& pipeline_state_manager,
							AT::GPURootSignatureManager& root_signature_manager,
							uint32_t width,
							uint32_t height,
							AT::RenderData& render_data,
							AT::FrameGraphRTVRef base_color,
							AT::FrameGraphRTVRef normal,
							AT::FrameGraphRTVRef surface,
							AT::FrameGraphDSVRef depth_buffer) {

			AT::GBufferShader shader = AT::GBufferShader(shader_manager.LoadRHIShader("GBuffer", RHI::ShaderType::VERTEX),
														shader_manager.LoadRHIShader("GBuffer", RHI::ShaderType::PIXEL), 
														root_signature_manager);
			GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetInputLayout(AT::VertexFormat::input_layout)
				.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
				.SetRenderTargetFormats({ RHI::Format::R8G8B8A8_UNORM_SRGB, RHI::Format::R16G16B16A16_UNORM, RHI::Format::R8G8B8A8_UNORM })
				.SetDepthFormat(RHI::Format::D32_FLOAT_S8X24_UINT)
				.SetDepthEnabled(true)
				.SetDepthStencilComparisonFunction(RHI::ComparisonFunction::EQUAL)
				.SetDepthWriteMask(RHI::DepthWriteMask::ZERO)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);

			AT::FrameGraphRenderPassIO IO = {};
			IO.AddRenderTarget(base_color, RHI::RenderPassAttachment::LoadOperation::CLEAR);
			IO.AddRenderTarget(normal, RHI::RenderPassAttachment::LoadOperation::CLEAR);
			IO.AddRenderTarget(surface, RHI::RenderPassAttachment::LoadOperation::CLEAR);
			IO.SetDepthStencilBufferAsDependency(depth_buffer, RHI::RenderPassAttachment::LoadOperation::LOAD);
			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			graph_builder.AddRenderPass(name, IO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso);
				command_list->SetGraphicsRootSignature(shader.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = width;
				viewport.Height = height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = height;
				scissor_rect.Right = width;
				command_list->SetScissorRect(scissor_rect);

				AT::GBufferShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::GBufferShader::Parameters>();
				std::vector<AT::GBufferShader::ObjectGroup*> object_groups = graph_builder.AllocateShaderParameterGroup<AT::GBufferShader::ObjectGroup>(shader.GetRootSignature(), 1, render_data.RenderObjects.size());
				std::vector< AT::MaterialManager::MaterialGroup*> material_groups = graph_builder.AllocateShaderParameterGroup<AT::MaterialManager::MaterialGroup>(shader.GetRootSignature(), 0, render_data.RenderObjects.size());
				for (uint32_t i = 0; i < render_data.RenderObjects.size(); ++i) {
					parameters.object = object_groups[i];
					parameters.object->constant_buffer = graph_builder.AllocateConstantBuffer<AT::GBufferShader::ObjectGroup::Constants>();
					parameters.material = material_groups[i];
				
					const AT::RenderObject& render_object = render_data.RenderObjects[i];
					const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];
					DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);
					
					DirectX::XMStoreFloat4x4A(&parameters.object->constants.MVP_Matrix, DirectX::XMMatrixTranspose(model_matrix) * render_data.ViewProjectionMatrix);
					DirectX::XMStoreFloat4x4A(&parameters.object->constants.Normal_Matrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, model_matrix * render_data.ViewMatrix)));

					//CopyDescriptorTable(*parameters.material, materials[mesh->Material_ID]->Descriptor_Table);

					parameters.material->BaseColor.srv = render_data.Materials[mesh->MaterialID]->BaseColorSRV;
					parameters.material->Normals.srv = render_data.Materials[mesh->MaterialID]->NormalSRV;
					parameters.material->Surface.srv = render_data.Materials[mesh->MaterialID]->RoughnessMetalnessSRV;
					parameters.material->constant_buffer = &render_data.Materials[mesh->MaterialID]->FactorConstantBuffer;

					shader.SetParameters(command_list, &parameters);
					
					command_list->SetVertexBuffer(0, 1, &mesh->VBView);
					command_list->SetIndexBuffer(mesh->IBView);
					command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
					command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
				}
			});
		}

		inline FrameGraphTextureRef CSM(const std::string& name,
							AT::FrameGraphBuilder& graph_builder,
							AT::GPUShaderManager& shader_manager,
							AT::GPUPipelineStateManager& pipeline_state_manager,
							AT::GPURootSignatureManager& root_signature_manager,
							AT::RenderData& render_data,
							uint32_t light_index,
							uint32_t splits) {

			const DirectX::XMFLOAT2A partitions[4] = {
				{ 1.0f, 20.0f },
				{ 20.0f, 100.0f},
				{ 100.0f, 300.0f },
				{ 300.0f , 600.0f }
			};

			const float aspect_ratio = 1280.0f / 720.0f;
			const float fov_angle = tan(0.1667f * 3.141592653589793f * 0.5f);

			const DirectX::XMFLOAT4A frustrum_points[8] = {
				//Near Plane
				{ 1.0f, 1.0f, 0.0f, 1.0f },
				{ -1.0f, 1.0f, 0.0f, 1.0f },
				{ 1.0f, -1.0f, 0.0f, 1.0f },
				{ -1.0f, -1.0f, 0.0f, 1.0f },
				//Far Plane
				{ 1.0f, 1.0f, 1.0f, 1.0f },
				{ -1.0f, 1.0f, 1.0f, 1.0f },
				{ 1.0f, -1.0f, 1.0f, 1.0f },
				{ -1.0f, -1.0f, 1.0f, 1.0f },
			};

			uint32_t size = 4096;

			AT::FrameGraphTextureRef csm_shadow_map = graph_builder.CreateTexture({
				.Format = RHI::Format::D16_UNORM,
				.Width = size,
				.Height = size,
				.ArraySize = 4,
			});

			RHI::TextureClearValue clear_value = { .DepthAndStencil = {.Depth = 0.0f, .Stencil = 0 } };
			csm_shadow_map->Description.Clear_Value = std::optional<RHI::TextureClearValue>(clear_value);

			AT::CascadedShadowMappingShader shader = AT::CascadedShadowMappingShader(shader_manager.LoadRHIShader("CascadedShadowMap", RHI::ShaderType::VERTEX),
																					shader_manager.LoadRHIShader("CascadedShadowMap", RHI::ShaderType::PIXEL), 
																					root_signature_manager);
			GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetInputLayout(AT::VertexFormat::input_layout)
				.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
				.SetDepthFormat(RHI::Format::D16_UNORM)
				.SetDepthStencilComparisonFunction(RHI::ComparisonFunction::GREATER_EQUAL)
				.SetDepthEnabled(true)
				.SetDepthWriteMask(RHI::DepthWriteMask::ALL)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);
			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			for (uint32_t i = 0u; i < splits; ++i) {
				float near_distance = partitions[i].x / 1.0f;
				float far_distance = partitions[i].y / 1.0f;

				DirectX::XMMATRIX camera_projection = DirectX::XMMatrixPerspectiveFovRH(0.415f * 3.14159f, 1280.0f / 720.0f, far_distance, near_distance);
				DirectX::XMMATRIX inverse_view_projection = DirectX::XMMatrixInverse(nullptr, render_data.ViewMatrix * camera_projection);

				DirectX::XMVECTOR center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

				DirectX::XMFLOAT4A frustrum_corners[8];

				for (uint32_t j = 0u; j < 8u; ++j) {
					DirectX::XMVECTOR world_space = DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&frustrum_points[j]), inverse_view_projection);
					DirectX::XMFLOAT4A world_space_float4;
					DirectX::XMStoreFloat4A(&world_space_float4, world_space);
					world_space = DirectX::XMVectorScale(world_space, 1.0f / world_space_float4.w);
					DirectX::XMStoreFloat4A(&frustrum_corners[j], world_space);
				

					DirectX::XMFLOAT4 test;
					DirectX::XMStoreFloat4(&test, world_space);

					center = DirectX::XMVectorAdd(center, world_space);
				}
				center = DirectX::XMVectorScale(center, 1.0f / 8.0f);

				float radius = 0.0f;
				{
					DirectX::XMFLOAT4 p1, p2;
					DirectX::XMStoreFloat4(&p1, DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&frustrum_points[0]), inverse_view_projection));
					DirectX::XMStoreFloat4(&p2, DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&frustrum_points[7]), inverse_view_projection));
					float length = std::sqrt(std::pow((p1.x / p1.w) - (p2.x / p2.w), 2.0f) + std::pow((p1.y / p1.w) - (p2.y / p2.w), 2.0f) + std::pow((p1.z / p1.w) - (p2.z / p2.w), 2.0f));
					radius = length / 2.0f;
				}

				float units = size / radius / 2.0f;
				DirectX::XMMATRIX scalar_matrix = DirectX::XMMatrixScaling(units, units, units);
				DirectX::XMVECTOR light_direction = DirectX::XMLoadFloat4A(&render_data.Lights[light_index].PositionOrDirection);
				DirectX::XMMATRIX look_at = DirectX::XMMatrixLookAtRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVector3Normalize(light_direction), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
				look_at = scalar_matrix * look_at;

				DirectX::XMFLOAT4 new_center4;
				DirectX::XMStoreFloat4(&new_center4, center);
				new_center4.w = 1.0f;
				DirectX::XMVECTOR snap_center = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&new_center4), look_at);
				DirectX::XMFLOAT4 snap_center4;
				DirectX::XMStoreFloat4(&snap_center4, snap_center);
				snap_center4.x = std::floor(snap_center4.x / snap_center4.w);
				snap_center4.y = std::floor(snap_center4.y / snap_center4.w);
				snap_center4.z = std::floor(snap_center4.z / snap_center4.w);
				snap_center4.w = 1.0f;
				snap_center = DirectX::XMLoadFloat4(&snap_center4);

				DirectX::XMVECTOR new_center = DirectX::XMVector4Transform(snap_center, DirectX::XMMatrixInverse(nullptr, look_at));
				DirectX::XMMATRIX light_view_matrix = DirectX::XMMatrixLookAtRH(DirectX::XMVectorAdd(new_center, DirectX::XMVectorScale(DirectX::XMVector3Normalize(light_direction), radius * 1.0f)), new_center, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

				//DirectX::XMMATRIX light_view_matrix = DirectX::XMMatrixLookAtRH(DirectX::XMVectorAdd(center, DirectX::XMVectorScale(DirectX::XMVector3Normalize(light_direction), radius * 2.0f)), center, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

				float scale = 5.0f;
				DirectX::XMMATRIX ortho = DirectX::XMMatrixOrthographicOffCenterRH(-radius, radius, -radius, radius, 2.0f * radius, -radius);
				DirectX::XMMATRIX light_matrix = light_view_matrix * ortho;
				DirectX::XMStoreFloat4x4A(&render_data.Lights[light_index].LightMatrices[i], light_matrix);
				AT::FrameGraphRenderPassIO csmIO = {};
				csmIO.SetDepthStencilBuffer(graph_builder.CreateDSV(csm_shadow_map, { .FirstSliceIndex = i, .ArraySize = 1 }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
				graph_builder.AddRenderPass(name + std::to_string(i), csmIO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {

					command_list->SetPipelineState(pso);
					command_list->SetGraphicsRootSignature(shader.GetRootSignature());

					RHI::Viewport viewport = {};
					viewport.TopLeftX = 0.0f;
					viewport.TopLeftY = 0.0f;
					viewport.Width = 4096;
					viewport.Height = 4096;
					viewport.MinDepth = 0.0f;
					viewport.MaxDepth = 1.0f;
					command_list->SetViewport(viewport);

					RHI::Rect scissor_rect = {};
					scissor_rect.Top = 0;
					scissor_rect.Left = 0;
					scissor_rect.Bottom = 4096;
					scissor_rect.Right = 4096;
					command_list->SetScissorRect(scissor_rect);
					
					AT::CascadedShadowMappingShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::CascadedShadowMappingShader::Parameters>();
					std::vector<AT::CascadedShadowMappingShader::ObjectGroup*> object_groups = graph_builder.AllocateShaderParameterGroup<AT::CascadedShadowMappingShader::ObjectGroup>(shader.GetRootSignature(), 0, render_data.RenderObjects.size());
					for (uint32_t j = 0; j < render_data.RenderObjects.size(); ++j) {
						const AT::RenderObject& render_object = render_data.RenderObjects[j];
						const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];
						DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);

						parameters.object = object_groups[j];
						parameters.object->constant_buffer = graph_builder.AllocateConstantBuffer<AT::CascadedShadowMappingShader::ObjectGroup::Constants>();
						DirectX::XMStoreFloat4x4A(&parameters.object->constants.MVP_Matrix, DirectX::XMMatrixTranspose(model_matrix) * light_matrix);

						shader.SetParameters(command_list, &parameters);
						
						command_list->SetVertexBuffer(0, 1, &mesh->VBView);
						command_list->SetIndexBuffer(mesh->IBView);
						command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
						command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
					}
				});
			}
			return csm_shadow_map;
		}

		inline FrameGraphTextureRef OSM(const std::string& name,
			AT::FrameGraphBuilder& graph_builder,
			AT::GPUShaderManager& shader_manager,
			AT::GPUPipelineStateManager& pipeline_state_manager,
			AT::GPURootSignatureManager& root_signature_manager,
			AT::RenderData& render_data,
			uint32_t light_index) {

			RHI::TextureClearValue clear_value = { .DepthAndStencil = {.Depth = 1.0f, .Stencil = 0 } };
			AT::FrameGraphTextureRef osm_shadow_map = graph_builder.CreateTexture({
				.Format = RHI::Format::D16_UNORM,
				.Width = 1024,
				.Height = 1024,
				.ArraySize = 6,
			});
			osm_shadow_map->Description.Clear_Value = std::optional<RHI::TextureClearValue>(clear_value);

			const DirectX::XMVECTOR UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR DOWN = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR LEFT = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
			const DirectX::XMVECTOR BACKWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			std::vector<DirectX::XMMATRIX> matrices = std::vector<DirectX::XMMATRIX>();
			float Max_Distance = render_data.Lights[light_index].Radius;
			//DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 1024.0f / 1024.0f, 1.0f, Max_Distance);
			DirectX::XMVECTOR light_position = DirectX::XMLoadFloat4A(&render_data.Lights[light_index].PositionOrDirection);
			DirectX::XMMATRIX projection_matrix =  DirectX::XMMatrixPerspectiveFovLH(0.5f * 3.14159f, 1024.0f / 1024.0f, 1.0f, Max_Distance);
			matrices.push_back(DirectX::XMMatrixLookToLH(light_position, RIGHT, UP) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToLH(light_position, LEFT, UP) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToLH(light_position, UP, FORWARD) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToLH(light_position, DOWN, BACKWARD) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToLH(light_position, BACKWARD, UP) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToLH(light_position, FORWARD, UP) * projection_matrix);
			
		/*	matrices.push_back(DirectX::XMMatrixLookToRH(light_position, RIGHT, DOWN) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(light_position, LEFT, DOWN) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(light_position, UP, BACKWARD) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(light_position, DOWN, FORWARD) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(light_position, BACKWARD, DOWN) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(light_position, FORWARD, DOWN) * projection_matrix);*/

	
		/*	const DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 1024.0f / 1024.0f, 1.0f, Max_Distance);
			std::vector<DirectX::XMMATRIX> view_matrices;
			DirectX::XMVECTOR position = DirectX::XMLoadFloat4A(&render_data.Lights[light_index].PositionOrDirection);
			matrices.push_back(DirectX::XMMatrixLookToRH(position, RIGHT, UP) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(position, LEFT, UP) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(position, UP, BACKWARD) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(position, DOWN, FORWARD) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(position, BACKWARD, UP) * projection_matrix);
			matrices.push_back(DirectX::XMMatrixLookToRH(position, FORWARD, UP) * projection_matrix);*/

			AT::OmnidirectionalShadowMappingShader shader = AT::OmnidirectionalShadowMappingShader(shader_manager.LoadRHIShader("OmnidirectionalShadowMap", RHI::ShaderType::VERTEX),
																								shader_manager.LoadRHIShader("OmnidirectionalShadowMap", RHI::ShaderType::PIXEL),
																								root_signature_manager);

			GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetInputLayout(VertexFormat::input_layout)
				.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
				.SetDepthFormat(RHI::Format::D16_UNORM)
				.SetDepthStencilComparisonFunction(RHI::ComparisonFunction::LESS_EQUAL)
				.SetDepthEnabled(true)
				.SetDepthWriteMask(RHI::DepthWriteMask::ALL)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);
			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			for (uint32_t i = 0u; i < 6u; ++i) {
				AT::FrameGraphRenderPassIO osmIO = {};
				osmIO.SetDepthStencilBuffer(graph_builder.CreateDSV(osm_shadow_map, { .FirstSliceIndex = i, .ArraySize = 1 }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
				graph_builder.AddRenderPass(name + std::to_string(i), osmIO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {

					command_list->SetPipelineState(pso);
					command_list->SetGraphicsRootSignature(shader.GetRootSignature());

					RHI::Viewport viewport = {};
					viewport.TopLeftX = 0.0f;
					viewport.TopLeftY = 0.0f;
					viewport.Width = 1024;
					viewport.Height = 1024;
					viewport.MinDepth = 0.0f;
					viewport.MaxDepth = 1.0f;
					command_list->SetViewport(viewport);

					RHI::Rect scissor_rect = {};
					scissor_rect.Top = 0;
					scissor_rect.Left = 0;
					scissor_rect.Bottom = 1024;
					scissor_rect.Right = 1024;
					command_list->SetScissorRect(scissor_rect);

					AT::OmnidirectionalShadowMappingShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::OmnidirectionalShadowMappingShader::Parameters>();
					std::vector<AT::OmnidirectionalShadowMappingShader::PassGroup*> pass_group = graph_builder.AllocateShaderParameterGroup<AT::OmnidirectionalShadowMappingShader::PassGroup>(shader.GetRootSignature(), 0, render_data.RenderObjects.size());
					for (uint32_t j = 0; j < render_data.RenderObjects.size(); ++j) {
						const AT::RenderObject& render_object = render_data.RenderObjects[j];
						const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];

						DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);						
						parameters.pass = pass_group[j];
						parameters.pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::OmnidirectionalShadowMappingShader::PassGroup::Constants>();

						DirectX::XMStoreFloat4x4A(&parameters.pass->constants.MVP_Matrix, DirectX::XMMatrixTranspose(model_matrix) * matrices[i]);
						DirectX::XMStoreFloat4x4A(&parameters.pass->constants.Model_Matrix, model_matrix);
						DirectX::XMStoreFloat4A(&parameters.pass->constants.Light_Position, light_position);
						parameters.pass->constants.Max_Distance = Max_Distance;

						shader.SetParameters(command_list, &parameters);

						command_list->SetVertexBuffer(0, 1, &mesh->VBView);
						command_list->SetIndexBuffer(mesh->IBView);
						command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
						command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
					}

				});
			}
			return osm_shadow_map;
		}
		//struct SSAO_Init_Output {
		//	GPUTexturePtr noise_texture;
		//	std::vector<DirectX::XMFLOAT4A> kernel_data;
		//};
		//inline SSAO_Init_Output  SSAO_Init(float noise_size, RHI::Device device, AT::GPUResourceManager& resource_manager, AT::GPUResourceUploadBatch& upload_batch) {
		//	std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
		//	std::default_random_engine random_engine;
		//	SSAO_Init_Output output;
		//	//Initialize Noise.
		//	RHI::TextureDescription test_noise_texture_description = {
		//		.TextureType = RHI::TextureType::TEXTURE_2D,
		//		.Format = RHI::Format::R32G32B32A32_FLOAT,
		//		.Width = (uint32_t)noise_size,
		//		.Height = (uint32_t)noise_size,
		//		.DepthOrArray = 1,
		//		.MipLevels = 1,
		//		.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE,
		//	};
		//	uint32_t pitch = ceil(noise_size * 4.0f * sizeof(float) / device->GetTexturePitchAlignment()) * device->GetTexturePitchAlignment();
		//	std::vector<float> noise_data = std::vector<float>((pitch / sizeof(float)) * noise_size);
		//	for (uint32_t i = 0; i < noise_size; ++i) {
		//		for (uint32_t j = 0; j < noise_size; ++j) {
		//			unsigned int index = i * (pitch / sizeof(float)) + j * 4;
		//			DirectX::XMVECTOR xm_noise_vector = DirectX::XMVectorSet(
		//				random_floats(random_engine) * 2.0f - 1.0f,
		//				random_floats(random_engine) * 2.0f - 1.0f,
		//				0.0f,
		//				0.0f
		//			);
		//			xm_noise_vector = DirectX::XMVector3Normalize(xm_noise_vector);
		//			DirectX::XMFLOAT4 noise_vector;
		//			DirectX::XMStoreFloat4(&noise_vector, xm_noise_vector);
		//			noise_data[index] = noise_vector.x;
		//			noise_data[index + 1] = noise_vector.y;
		//			noise_data[index + 2] = 0.0f;
		//			noise_data[index + 3] = 0.0f;
		//		}
		//	}

		//	RHI::TextureDescription noise_texture_description = {
		//		.TextureType = RHI::TextureType::TEXTURE_2D,
		//		.Format = RHI::Format::R32G32B32A32_FLOAT,
		//		.Width = (uint32_t)noise_size,
		//		.Height = (uint32_t)noise_size,
		//		.DepthOrArray = 1,
		//		.MipLevels = 1,
		//		.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE,
		//	};
		//	GPUTexturePtr noise_texture = resource_manager.CreateTexture(noise_texture_description);
		//	output.noise_texture = noise_texture;

		//	RHI::BufferDescription staging_buffer_description;
		//	staging_buffer_description.Size = noise_data.size() * sizeof(float);
		//	staging_buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
		//	GPUBufferPtr staging_buffer = resource_manager.CreateUploadBuffer(staging_buffer_description);
		//	staging_buffer->GetRHIHandle()->Map();
		//	staging_buffer->GetRHIHandle()->CopyData(0, noise_data.data(), noise_data.size() * sizeof(float));
		//	upload_batch.AddTextureUpload(noise_texture, staging_buffer);
		//	// noise_data.data(), noise_data.size() * sizeof(float)
		//	
		//	for (uint32_t i = 0; i < 64; ++i) {

		//		DirectX::XMFLOAT3 sample = DirectX::XMFLOAT3();
		//		DirectX::XMVECTOR sample_vector = DirectX::XMVectorSet(
		//			random_floats(random_engine) * 2.0f - 1.0f,
		//			random_floats(random_engine) * 2.0f - 1.0f,
		//			random_floats(random_engine),
		//			0.0f
		//		);

		//		sample_vector = DirectX::XMVector3Normalize(sample_vector);
		//		sample_vector = DirectX::XMVectorScale(sample_vector, random_floats(random_engine));
		//		float scale = (float)i / 64.0f;
		//		scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
		//		sample_vector = DirectX::XMVectorScale(sample_vector, scale);
		//		DirectX::XMStoreFloat3(&sample, sample_vector);

		//		output.kernel_data.emplace_back(DirectX::XMFLOAT4A(sample.x, sample.y, sample.z, 0.0f));
		//	}
		//	return output;
		//}

		inline void SSAO(const std::string& name,
			AT::FrameGraphBuilder& graph_builder,
			AT::GPUShaderManager& shader_manager,
			AT::GPUPipelineStateManager& pipeline_state_manager,
			AT::GPURootSignatureManager& root_signature_manager,
			uint32_t width,
			uint32_t height,
			AT::RenderData& render_data,
			FrameGraphRTVRef output_texture,
			FrameGraphSRVRef depth,
			FrameGraphSRVRef normal,
			uint32_t noise_size,
			RHI::Texture noise_texture, 
			const std::vector<DirectX::XMFLOAT4A>& kernel_data) {

			SSAOShader shader = SSAOShader(shader_manager.LoadRHIShader("SSAO", RHI::ShaderType::VERTEX),
				shader_manager.LoadRHIShader("SSAO", RHI::ShaderType::PIXEL),
				root_signature_manager);

			GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
				.SetRenderTargetFormats({ RHI::Format::R8_UNORM })
				.SetDepthEnabled(false)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);

			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			FrameGraphTextureRef SSAO_Output = graph_builder.CreateTexture({
				.Format = RHI::Format::R8_UNORM,
				.Width = width,
				.Height = height,
				.ArraySize = 1,
				.Clear_Value = {},
			});

			AT::FrameGraphRenderPassIO IO = {};
			IO.AddRenderTarget(graph_builder.CreateRTV(SSAO_Output, {}), RHI::RenderPassAttachment::LoadOperation::CLEAR);
			IO.AddShaderResource(depth);
			IO.AddShaderResource(normal);
			IO.AddShaderResource(graph_builder.CreateSRV(graph_builder.RegisterReadTexture(noise_texture, RHI::TextureState::PIXEL_SHADER_RESOURCE), {}));

			graph_builder.AddRenderPass(name, IO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso);
				command_list->SetGraphicsRootSignature(shader.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = width;
				viewport.Height = height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = height;
				scissor_rect.Right = width;
				command_list->SetScissorRect(scissor_rect);

				SSAOShader::Parameters& parameters = graph_builder.AllocateShaderParameters<SSAOShader::Parameters>();
				parameters.pass = graph_builder.AllocateShaderParameterGroup<AT::SSAOShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
				parameters.pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::SSAOShader::PassGroup::Constants>();

				DirectX::XMStoreFloat4x4(&parameters.pass->constants.Projection_Matrix, render_data.ProjectionMatrix);
				DirectX::XMStoreFloat4x4(&parameters.pass->constants.Inverse_Projection_Matrix, render_data.InverseProjectionMatrix);
				for (uint32_t i = 0u; i < 64u; ++i) {
					parameters.pass->constants.Kernel[i] = kernel_data[i];
				}

				parameters.pass->constants.Size = { (float)width, (float)height };
				parameters.pass->g_Depth.srv = IO.m_shader_resources[0]->RHIHandle;
				parameters.pass->g_Normal.srv = IO.m_shader_resources[1]->RHIHandle;
				parameters.pass->g_Noise.srv = IO.m_shader_resources[2]->RHIHandle;

				shader.SetParameters(command_list, &parameters);

				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
				command_list->Draw(4, 1, 0, 0);
			});

			AT::FrameGraphRenderPassIO IO2 = {};
			IO2.AddShaderResource(graph_builder.CreateSRV(SSAO_Output, {  .FirstSliceIndex = 0, .ArraySize = 1, . MipLevels = 1 }));
			IO2.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);
			SSAOBlurShader shader2 = SSAOBlurShader(shader_manager.LoadRHIShader("ssao_blur", RHI::ShaderType::VERTEX),
				shader_manager.LoadRHIShader("ssao_blur", RHI::ShaderType::PIXEL),
				root_signature_manager);

			GPUGraphicsPipelineStateBuilder pso2_builder;
			pso2_builder.SetRootSignature(shader2.GetRootSignature())
				.SetVertexShader(shader2.GetVertexShader())
				.SetPixelShader(shader2.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
				.SetRenderTargetFormats({ RHI::Format::R8_UNORM })
				.SetDepthEnabled(false)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);
			RHI::PipelineState pso2 = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso2_builder.ToRHIDescription());
			graph_builder.AddRenderPass(name + "_blur", IO2, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso2);
				command_list->SetGraphicsRootSignature(shader2.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = width;
				viewport.Height = height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = height;
				scissor_rect.Right = width;
				command_list->SetScissorRect(scissor_rect);

				SSAOBlurShader::Parameters parameters = graph_builder.AllocateShaderParameters<SSAOBlurShader::Parameters>();
				parameters.pass = graph_builder.AllocateShaderParameterGroup<AT::SSAOBlurShader::PassGroup>(shader2.GetRootSignature(), 0, 1)[0];
				parameters.pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::SSAOShader::PassGroup::Constants>();
				parameters.pass->frame.srv = IO2.m_shader_resources[0]->RHIHandle;
				parameters.pass->constants.Size = { (float)width, (float)height };

				shader2.SetParameters(command_list, &parameters);

				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
				command_list->Draw(4, 1, 0, 0);
			});
		}


		inline FrameGraphTextureRef Deferred(const std::string& name,
			AT::FrameGraphBuilder& graph_builder,
			AT::GPUShaderManager& shader_manager,
			AT::GPUPipelineStateManager& pipeline_state_manager,
			AT::GPURootSignatureManager& root_signature_manager,
			uint32_t width,
			uint32_t height,
			AT::RenderData& render_data,
			FrameGraphRTVRef output_texture, 
			FrameGraphSRVRef base_color, 
			FrameGraphSRVRef normal,
			FrameGraphSRVRef surface,
			FrameGraphSRVRef depth,
			FrameGraphSRVRef ssao,
			std::array <FrameGraphSRVRef,5>& csm,
			std::array<FrameGraphSRVRef, 5>& osm,
			FrameGraphSRVRef enviroment_map,
			FrameGraphSRVRef prefiltered_map,
			FrameGraphSRVRef env_brdf) {

			FrameGraphRenderPassIO deferredIO = {};
			deferredIO.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::LOAD);

			deferredIO.AddShaderResource(base_color);
			deferredIO.AddShaderResource(normal);
			deferredIO.AddShaderResource(surface);
			deferredIO.AddShaderResource(depth);
			deferredIO.AddShaderResource(ssao);
			for (uint32_t i = 0u; i < 5u; ++i) {
				deferredIO.AddShaderResource(csm[i]);
			}
			for (uint32_t i = 0u; i < 5u; ++i) {
				deferredIO.AddShaderResource(osm[i]);
			}
			deferredIO.AddShaderResource(enviroment_map);
			deferredIO.AddShaderResource(prefiltered_map);
			deferredIO.AddShaderResource(env_brdf);
			
			AT::DeferredShader shader = AT::DeferredShader(shader_manager.LoadRHIShader("Deferred", RHI::ShaderType::VERTEX),
				shader_manager.LoadRHIShader("Deferred", RHI::ShaderType::PIXEL),
				root_signature_manager);

			GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
				.SetRenderTargetFormats({ RHI::Format::R16G16B16A16_FLOAT })
				.SetDepthEnabled(false)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);
			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());
			graph_builder.AddRenderPass(name, deferredIO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso);
				command_list->SetGraphicsRootSignature(shader.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = width;
				viewport.Height = height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = height;
				scissor_rect.Right = width;
				command_list->SetScissorRect(scissor_rect);

				AT::DeferredShader::Parameters& deferred_parameters = graph_builder.AllocateShaderParameters<AT::DeferredShader::Parameters>();
				deferred_parameters.pass = graph_builder.AllocateShaderParameterGroup<AT::DeferredShader::DEFERREDINPUT>(shader.GetRootSignature(), 0, 1)[0];
				deferred_parameters.pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::DeferredShader::DEFERREDINPUT::Constants>();

				deferred_parameters.pass->constants.Use_Enviroment_Map = (enviroment_map == nullptr) ? false : true;
				
				for (uint32_t i = 0u; i < 5u; ++i) {
					
					DirectX::XMVECTOR light_direction_or_position = DirectX::XMLoadFloat4A(&render_data.Lights[i].PositionOrDirection);
					deferred_parameters.pass->constants.lights.lights[i].Type = render_data.Lights[i].Type;
					deferred_parameters.pass->constants.lights.lights[i].Intensity = render_data.Lights[i].Intensity;
					switch (render_data.Lights[i].Type) {
					case LIGHT_TYPE_DIRECTIONAL:
					{
						DirectX::XMStoreFloat4A(&(deferred_parameters.pass->constants.lights.lights[i].PositionOrDirection), DirectX::XMVector4Transform(DirectX::XMVector4Normalize(light_direction_or_position), render_data.ViewMatrix));
						for (uint32_t j = 0u; j < 4; ++j) {
							deferred_parameters.pass->constants.lights.lights[i].LightMatrices[j] = render_data.Lights[i].LightMatrices[j];
						}
					}
						break;
					case LIGHT_TYPE_POINT:
					{
						DirectX::XMStoreFloat4A(&(deferred_parameters.pass->constants.lights.lights[i].PositionOrDirection), DirectX::XMVector4Transform(light_direction_or_position, render_data.ViewMatrix));
						deferred_parameters.pass->constants.lights.lights[i].Radius = render_data.Lights[i].Radius;
					}
						break;
					}
				}
				
				DirectX::XMStoreFloat4x4A(&deferred_parameters.pass->constants.Inverse_Projection_Matrix, render_data.InverseProjectionMatrix);
				DirectX::XMStoreFloat4x4A(&deferred_parameters.pass->constants.Inverse_View_Matrix, render_data.InverseViewMatrix);


				deferred_parameters.pass->Base_Color.srv = deferredIO.m_shader_resources[0]->RHIHandle;
				deferred_parameters.pass->Normals.srv = deferredIO.m_shader_resources[1]->RHIHandle;
				deferred_parameters.pass->Surface.srv = deferredIO.m_shader_resources[2]->RHIHandle;
				deferred_parameters.pass->Depth.srv = deferredIO.m_shader_resources[3]->RHIHandle;
				deferred_parameters.pass->SSAO.srv = deferredIO.m_shader_resources[4]->RHIHandle;
				
				deferred_parameters.pass->CSM[0].srv = deferredIO.m_shader_resources[5]->RHIHandle;
				deferred_parameters.pass->CSM[1].srv = deferredIO.m_shader_resources[6]->RHIHandle;
				deferred_parameters.pass->CSM[2].srv = deferredIO.m_shader_resources[7]->RHIHandle;
				deferred_parameters.pass->CSM[3].srv = deferredIO.m_shader_resources[8]->RHIHandle;
				deferred_parameters.pass->CSM[4].srv = deferredIO.m_shader_resources[9]->RHIHandle;
				
				deferred_parameters.pass->OSM[0].srv = deferredIO.m_shader_resources[10]->RHIHandle;
				deferred_parameters.pass->OSM[1].srv = deferredIO.m_shader_resources[11]->RHIHandle;
				deferred_parameters.pass->OSM[2].srv = deferredIO.m_shader_resources[12]->RHIHandle;
				deferred_parameters.pass->OSM[3].srv = deferredIO.m_shader_resources[13]->RHIHandle;
				deferred_parameters.pass->OSM[4].srv = deferredIO.m_shader_resources[14]->RHIHandle;
				
				deferred_parameters.pass->Irradiance.srv = deferredIO.m_shader_resources[15]->RHIHandle;
				deferred_parameters.pass->prefiltered_map.srv = deferredIO.m_shader_resources[16]->RHIHandle;
				deferred_parameters.pass->env_brdf.srv = deferredIO.m_shader_resources[17]->RHIHandle;

				shader.SetParameters(command_list, &deferred_parameters);

				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
				command_list->Draw(4, 1, 0, 0);
			});

			return output_texture->TextureRef;
		}

		inline void ToneMapping(AT::FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, AT::GPUPipelineStateManager& pipeline_state_manager, AT::GPURootSignatureManager& root_signature_manager, FrameGraphTextureRef input_texture, FrameGraphTextureRef output_texture/*, RHI_VertexBufferView& quad_vertices*/) {
			FrameGraphRenderPassIO tone_mapping_io = {};
			tone_mapping_io.AddRenderTarget(graph_builder.CreateRTV(output_texture, {}), RHI::RenderPassAttachment::LoadOperation::CLEAR);
			tone_mapping_io.AddShaderResource(graph_builder.CreateSRV(input_texture, {}));

			ToneMappingShader shader = ToneMappingShader(shader_manager.LoadRHIShader("ToneMapping", RHI::ShaderType::VERTEX),
				shader_manager.LoadRHIShader("ToneMapping", RHI::ShaderType::PIXEL),
				root_signature_manager);

			GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
				.SetRenderTargetFormats({ output_texture->Description.Format })
				.SetDepthEnabled(false)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);
			
			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			graph_builder.AddRenderPass("ToneMapping", tone_mapping_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso);
				command_list->SetGraphicsRootSignature(shader.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = output_texture->Description.Width;
				viewport.Height = output_texture->Description.Height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = output_texture->Description.Height;
				scissor_rect.Right = output_texture->Description.Width;
				command_list->SetScissorRect(scissor_rect);

				ToneMappingShader::Parameters& parameters = graph_builder.AllocateShaderParameters<ToneMappingShader::Parameters>();
				parameters.pass = graph_builder.AllocateShaderParameterGroup<AT::ToneMappingShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
				parameters.pass->Frame.srv = tone_mapping_io.m_shader_resources[0]->RHIHandle;

				shader.SetParameters(command_list, &parameters);

				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
				command_list->Draw(4, 1, 0, 0);
			});
		}

		inline void Skybox(
			const std::string& name,
			AT::FrameGraphBuilder& graph_builder, 
			AT::GPUShaderManager& shader_manager,
			AT::GPUPipelineStateManager& pipeline_state_manager,
			AT::GPURootSignatureManager& root_signature_manager,
			RenderData& render_data,
			uint32_t width,
			uint32_t height,
			FrameGraphSRVRef skybox, 
			FrameGraphRTVRef output_texture
			) {
			FrameGraphRenderPassIO pass_io = {};
			pass_io.AddShaderResource(skybox);
			///FrameGraphTextureRef output_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = 1280, .Height = 720, .Array_Size = 1 });
			pass_io.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);
			AT::SkyboxShader shader = AT::SkyboxShader(shader_manager.LoadRHIShader("Skybox", RHI::ShaderType::VERTEX),
				shader_manager.LoadRHIShader("Skybox", RHI::ShaderType::PIXEL),
				root_signature_manager);

			AT::GPUGraphicsPipelineStateBuilder pso_builder;
			pso_builder.SetRootSignature(shader.GetRootSignature())
				.SetVertexShader(shader.GetVertexShader())
				.SetPixelShader(shader.GetPixelShader())
				.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
				.SetRenderTargetFormats({ output_texture->TextureRef->Description.Format })
				.SetDepthEnabled(false)
				.SetCullMode(RHI::CullMode::BACK)
				.SetFrontCounterClockwise(true)
				.SetFillMode(RHI::FillMode::SOLID);

			/*RHI_GraphicsPipelineStateDescription pso_desc = pso_builder.Compile();
			RHI_PipelineState pso = AT::PipelineStateCache::CreateOrGetPipelineState(graph_builder.GetDevice(), pso_desc);*/
			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

			//view_matrix * projection_matrix

		/*	DirectX::XMFLOAT4X4 view3x3;
			DirectX::XMStoreFloat4x4(&view3x3, view_matrix);
			view3x3.m[3][0] = 0;
			view3x3.m[3][1] = 0;
			view3x3.m[3][2] = 0;
			view3x3.m[3][3] = 1;
			view3x3.m[0][3] = 0;
			view3x3.m[1][3] = 0;
			view3x3.m[2][3] = 0;*/
			DirectX::XMFLOAT4 cam_pos;
			DirectX::XMStoreFloat4(&cam_pos, render_data.CameraPosition);
			DirectX::XMMATRIX model_matrix = DirectX::XMMatrixTranslation(cam_pos.x, cam_pos.y, cam_pos.z);
			DirectX::XMMATRIX new_view_matrix = model_matrix * render_data.ViewMatrix;

			graph_builder.AddRenderPass(name, pass_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {
				command_list->SetPipelineState(pso);
				command_list->SetGraphicsRootSignature(shader.GetRootSignature());

				RHI::Viewport viewport = {};
				viewport.TopLeftX = 0.0f;
				viewport.TopLeftY = 0.0f;
				viewport.Width = width;
				viewport.Height = height;
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				command_list->SetViewport(viewport);

				RHI::Rect scissor_rect = {};
				scissor_rect.Top = 0;
				scissor_rect.Left = 0;
				scissor_rect.Bottom = height;
				scissor_rect.Right = width;
				command_list->SetScissorRect(scissor_rect);

				SkyboxShader::Parameters& parameters = graph_builder.AllocateShaderParameters<SkyboxShader::Parameters>();
				parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::SkyboxShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
				parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::SkyboxShader::PassGroup::Constants>();

				DirectX::XMStoreFloat4x4(&parameters.Pass->constants.MV_Matrix, new_view_matrix * render_data.ProjectionMatrix);

				parameters.Pass->SkyBox.srv = pass_io.m_shader_resources[0]->RHIHandle;
				
				shader.SetParameters(command_list, &parameters);

				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
				command_list->Draw(36, 1, 0, 0);
			});
		}

		/*struct GenerateEnviromentMapOutput {
			FrameGraphTextureRef irradiance;
			FrameGraphTextureRef prefiltered;
			FrameGraphTextureRef env_brdf;
		};*/

		//inline GenerateEnviromentMapOutput GenerateEnviromentMap(FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, DirectX::XMVECTOR camera_position, std::vector<RenderObject>& render_objects, std::vector<Mesh*>& meshes, std::vector<Material*>& materials, SSAO_Init_Output& ssao_data, DirectX::XMVECTOR light_direction, std::array<DirectX::XMMATRIX, 4>& light_matrices, FrameGraphSRVRef csm_texture, FrameGraphSRVRef skybox, RHI_Texture noise_texture, const std::vector<DirectX::XMFLOAT4A>& kernel_data, FrameGraphSRVRef null_texture_srv, FrameGraphSRVRef null_cube_texture_srv) {
		//	uint32_t size = 128;

		//	DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 128.0f / 128.0f, 1000.0f, 1.0f);
		//	const DirectX::XMVECTOR UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		//	const DirectX::XMVECTOR DOWN = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		//	const DirectX::XMVECTOR LEFT = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		//	const DirectX::XMVECTOR RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		//	const DirectX::XMVECTOR FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		//	const DirectX::XMVECTOR BACKWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		//	std::vector<DirectX::XMMATRIX> matrices;
		///*	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, RIGHT, UP) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, LEFT, UP) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, UP, FORWARD) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, DOWN, BACKWARD) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, FORWARD, UP) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, BACKWARD, UP) * projection_matrix);*/

		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, RIGHT, UP) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, LEFT, UP) * projection_matrix);
		//	

		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, UP, BACKWARD) * projection_matrix); //normal is forward
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, DOWN, FORWARD) * projection_matrix); //normal is backward

		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, BACKWARD, UP) * projection_matrix);
		//	matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, FORWARD, UP) * projection_matrix);

		//	std::vector<DirectX::XMMATRIX> view_matrices;
		//	view_matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, RIGHT, UP));
		//	view_matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, LEFT, UP));


		//	view_matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, UP, BACKWARD));
		//	view_matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, DOWN, FORWARD));

		//	view_matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, BACKWARD, UP));
		//	view_matrices.push_back(DirectX::XMMatrixLookToRH(camera_position, FORWARD, UP));
		//	

		//	RHI_FORMAT gbuffer_formats[5] = {
		//		RHI::Format::R8G8B8A8_UNORM_SRGB, //Base Color.
		//		RHI::Format::R16G16B16A16_UNORM, //Normals.
		//		RHI::Format::R8G8B8A8_UNORM, //Roughness Metalness.
		//		RHI::Format::D32_FLOAT_S8X24_UINT, //Depth.
		//		RHI::Format::R8_UNORM, //SSAO
		//	};

		//	std::vector<FrameGraphTextureRef> cube_maps;
		//	for (uint32_t i = 0; i < 4; ++i) {
		//		AT::FrameGraphTextureDescription description;
		//		description.Width = size;
		//		description.Height = size;
		//		description.Array_Size = 6;
		//		description.Format = gbuffer_formats[i];
		//		if (i == 3) {
		//			RHI_TextureClearValue clear_value;
		//			clear_value.Format = gbuffer_formats[i];
		//			clear_value.Depth_AND_Stencil.Depth = 0.0f;
		//			description.Clear_Value = clear_value;
		//		}
		//		cube_maps.push_back(graph_builder.CreateTexture(description));
		//	}

		//	AT::FrameGraphTextureDescription description;
		//	description.Width = size;
		//	description.Height = size;
		//	description.Array_Size = 6;
		//	description.Format = gbuffer_formats[4];
		//	cube_maps.push_back(graph_builder.CreateTexture(description));

		//	{
		//		AT::FrameGraphTextureDescription description;
		//		description.Width = size;
		//		description.Height = size;
		//		description.Array_Size = 6;
		//		description.Mip_Levels = 7;
		//		description.Format = RHI::Format::R16G16B16A16_FLOAT;
		//		cube_maps.push_back(graph_builder.CreateTexture(description));
		//	}

		//	AT::FrameGraphTextureRef irradiance_map;
		//	
		//	{
		//		AT::FrameGraphTextureDescription description;
		//		description.Width = size;
		//		description.Height = size;
		//		description.Array_Size = 6;
		//		description.Format = RHI::Format::R16G16B16A16_FLOAT;
		//		irradiance_map = graph_builder.CreateTexture(description);
		//	}

		//	FrameGraphTextureRef prefiltered_map = graph_builder.CreateTexture({
		//				.Format = RHI::Format::R16G16B16A16_FLOAT,
		//				.Width = size,
		//				.Height = size,
		//				.Array_Size = 6,
		//				.Mip_Levels = 7
		//		});

		//		for (uint32_t i = 0; i < 6; ++i) {
		//			//GBuffer
		//			DepthPass(graph_builder, shader_manager, "DepthPass[" + std::to_string(i) + "]", graph_builder.CreateDSV(cube_maps[3], { .First_Slice_Index = i, .Array_Size = 1 }), render_objects, meshes, materials, projection_matrix, view_matrices[i], size, size);

		//			GBuffer(graph_builder, shader_manager, "EnviromentMap[" + std::to_string(i) + "]",
		//				render_objects,
		//				meshes, materials,
		//				projection_matrix,
		//				view_matrices[i],
		//				graph_builder.CreateRTV(cube_maps[0], { .First_Slice_Index = i, .Array_Size = 1 }),
		//				graph_builder.CreateRTV(cube_maps[1], { .First_Slice_Index = i, .Array_Size = 1 }),
		//				graph_builder.CreateRTV(cube_maps[2], { .First_Slice_Index = i, .Array_Size = 1 }),
		//				graph_builder.CreateDSV(cube_maps[3], { .First_Slice_Index = i, .Array_Size = 1 }),
		//				size,
		//				size,
		//				RHI::ComparisonFunction::EQUAL,
		//				RHI::DepthWriteMask::ZERO
		//			);

		//			//SSAO
		//			{
		//				SSAO(4, 
		//					"EnviromentMap[" + std::to_string(i) + "]",
		//					graph_builder,
		//					shader_manager,
		//					DirectX::XMMatrixInverse(nullptr, projection_matrix),
		//					projection_matrix,
		//					graph_builder.CreateRTV(cube_maps[4], { .First_Slice_Index = i, .Array_Size = 1, .Mip_Slice = 0 }),
		//					graph_builder.CreateSRV(cube_maps[3], { .First_Slice_Index = i, .Array_Size = 1, .Mip_Levels = 1 }),
		//					graph_builder.CreateSRV(cube_maps[1], { .First_Slice_Index = i, .Array_Size = 1, .Mip_Levels = 1 }),
		//					noise_texture,
		//					kernel_data,
		//					size,
		//					size
		//				);
		//			}

		//			//Skybox {
		//			{
		//				Skybox(graph_builder, shader_manager, "EnviromentMap[" + std::to_string(i) + "]", skybox, graph_builder.CreateRTV(cube_maps.back(), { .First_Slice_Index = i, .Array_Size = 1, .Mip_Slice = 0 }), projection_matrix, view_matrices[i], camera_position, size, size);
		//			}

		//			//Deferred
		//			{
		//				for (uint32_t j = 0; j < cube_maps.back()->Description.Mip_Levels; ++j) {
		//					Deferred(graph_builder, shader_manager, "Enviroment_Map[" + std::to_string(i) + "] Mip[" + std::to_string(j) + "]",
		//						graph_builder.CreateRTV(cube_maps.back(), { .First_Slice_Index = i, .Array_Size = 1, .Mip_Slice = j}),
		//						graph_builder.CreateSRV(cube_maps[0], { .First_Slice_Index = i, .Array_Size = 1 }),
		//						graph_builder.CreateSRV(cube_maps[1], { .First_Slice_Index = i, .Array_Size = 1 }),
		//						graph_builder.CreateSRV(cube_maps[2], { .First_Slice_Index = i, .Array_Size = 1 }),
		//						graph_builder.CreateSRV(cube_maps[3], { .First_Slice_Index = i, .Array_Size = 1 }),
		//						graph_builder.CreateSRV(cube_maps[4], { .First_Slice_Index = i, .Array_Size = 1 }),
		//						null_cube_texture_srv,
		//						csm_texture,
		//						null_cube_texture_srv,
		//						null_cube_texture_srv,
		//						null_texture_srv,
		//						DirectX::XMMatrixInverse(nullptr, projection_matrix),
		//						DirectX::XMMatrixInverse(nullptr, view_matrices[i]),
		//						view_matrices[i],
		//						light_matrices,
		//						light_direction,
		//						size >> j,
		//						size >> j
		//					);
		//				}
		//			}

		//			//Generate Mips.
		//			{
		//				//AT::GenerateMipsShader shader = AT::GenerateMipsShader(device);
		//			}

		//			//Irradiance Map.
		//			{
		//				//RenderPassIO pass_io = {};
		//				//pass_io.AddShaderResource(graph_builder.CreateSRV(cube_maps.back(), { .First_Slice_Index = 0, .Array_Size = 6 }));
		//				//pass_io.AddRenderTarget(graph_builder.CreateRTV(irradiance_map, { .First_Slice_Index = i, .Array_Size = 1 }), RRHI::RenderPassAttachment::LoadOperation::CLEAR);
		//				//AT::IrradianceMapShader shader = AT::IrradianceMapShader(graph_builder.GetDevice());
		//				//AT::PipelineStateBuilder pso_builder = {};
		//				//pso_builder.SetRootSignature(shader.GetRootSignature());
		//				//RHI_InputLayout input_layout = {};
		//				//input_layout.Input_Elements = {};
		//				//input_layout.Input_Stride = 0;
		//				//pso_builder.SetInputLayout(input_layout);
		//				//pso_builder.SetVertexShader(shader.GetVertexShader());
		//				//pso_builder.SetPixelShader(shader.GetPixelShader());
		//				//pso_builder.SetPrimiteTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
		//				//pso_builder.SetRenderTargets({ RHI::Format::R16G16B16A16_FLOAT });
		//				//pso_builder.SetFrontCounterClockWise(false);

		//				//RHI_GraphicsPipelineStateDescription pso_desc = pso_builder.Compile();
		//				//RHI_PipelineState pso = AT::PipelineStateCache::CreateOrGetPipelineState(graph_builder.GetDevice(), pso_desc);


		//				//std::vector<DirectX::XMMATRIX> view_matrices;
		//				//view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), LEFT, UP));
		//				//view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), RIGHT, UP));
		//				//view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DOWN, FORWARD));
		//				//view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), UP, BACKWARD));
		//				//view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), FORWARD, UP));
		//				//view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), BACKWARD, UP));

		//				//graph_builder.AddRenderPass("IrradianceMap" + std::to_string(i), size, size, pass_io, [=, &graph_builder](RHI_CommandList command_list) {
		//				//	command_list->SetPipelineState(pso);
		//				//	command_list->SetGraphicsRootSignature(shader.GetRootSignature());
		//				//	IrradianceMapShader::Parameters& parameters = graph_builder.AllocateShaderParameters<IrradianceMapShader::Parameters>();
		//				//	parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::IrradianceMapShader::PassGroup>(shader.GetRootSignature(), 0);
		//				//	parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::IrradianceMapShader::PassGroup::Constants>();

		//				//	//DirectX::XMStoreFloat4x4(&parameters.Pass->constants.View_Matrix, view_matrices[i]);
		//				//	DirectX::XMStoreFloat4x4(&parameters.Pass->constants.MV_Matrix, view_matrices[i] * DirectX::XMMatrixPerspectiveFovLH(0.5f * 3.14159f, 128.0f / 128.0f, 1.0f, 10.0f));

		//				//	parameters.Pass->Enviroment_Map.srv = pass_io.m_shader_resources[0]->native;

		//				//	shader.SetParameters(command_list, &parameters);

		//				//	command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
		//				//	command_list->Draw(36, 1, 0, 0);
		//				//});
		//			}

		//			//PreFiltered Map
		//			{
		//				for (uint32_t j = 0; j  < prefiltered_map->Description.Mip_Levels; ++j) {
		//					RenderPassIO pass_io = {};
		//					pass_io.AddShaderResource(graph_builder.CreateSRV(cube_maps.back(), { .First_Slice_Index = 0, .Array_Size = 6 }));
		//					pass_io.AddRenderTarget(graph_builder.CreateRTV(prefiltered_map, { .First_Slice_Index = i, .Array_Size = 1, .Mip_Slice = j }), RRHI::RenderPassAttachment::LoadOperation::CLEAR);
		//					AT::PreFilteredEnviromentMapShader shader = AT::PreFilteredEnviromentMapShader(shader_manager.LoadRHIShader("PreFilterMap", RHI::ShaderType::VERTEX),
		//																									shader_manager.LoadRHIShader("PreFilterMap", RHI::ShaderType::PIXEL),
		//																									graph_builder.GetDevice());
		//					AT::PipelineStateBuilder pso_builder = {};
		//					pso_builder.SetRootSignature(shader.GetRootSignature());
		//					RHI_InputLayout input_layout = {};
		//					input_layout.Input_Elements = {};
		//					input_layout.Input_Stride = 0;
		//					pso_builder.SetInputLayout(input_layout);
		//					pso_builder.SetVertexShader(shader.GetVertexShader());
		//					pso_builder.SetPixelShader(shader.GetPixelShader());
		//					pso_builder.SetPrimiteTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
		//					pso_builder.SetRenderTargets({ RHI::Format::R16G16B16A16_FLOAT });
		//					pso_builder.SetFrontCounterClockWise(true);

		//					RHI_GraphicsPipelineStateDescription pso_desc = pso_builder.Compile();
		//					RHI_PipelineState pso = AT::PipelineStateCache::CreateOrGetPipelineState(graph_builder.GetDevice(), pso_desc);


		//					std::vector<DirectX::XMMATRIX> view_matrices;


		//					view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), RIGHT, UP));
		//					view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), LEFT, UP));


		//					view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), UP, BACKWARD));
		//					view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DOWN, FORWARD));

		//					view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), BACKWARD, UP));
		//					view_matrices.push_back(DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), FORWARD, UP));

		//					graph_builder.AddRenderPass("PrefilteredMap" + std::to_string(i) + "Mip" + std::to_string(j), size >> j, size >> j, pass_io, [=, &graph_builder](RHI_CommandList command_list) {
		//						command_list->SetPipelineState(pso);
		//						command_list->SetGraphicsRootSignature(shader.GetRootSignature());
		//						PreFilteredEnviromentMapShader::Parameters& parameters = graph_builder.AllocateShaderParameters<PreFilteredEnviromentMapShader::Parameters>();
		//						parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::PreFilteredEnviromentMapShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
		//						parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::PreFilteredEnviromentMapShader::PassGroup::Constants>();

		//						DirectX::XMStoreFloat4x4(&parameters.Pass->constants.MV_Matrix, view_matrices[i] * DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 128.0f / 128.0f, 1.0f, 10.0f));
		//						parameters.Pass->constants.Roughness = (float)j / (float)(prefiltered_map->Description.Mip_Levels - 1);
		//						parameters.Pass->Enviroment_Map.srv = pass_io.m_shader_resources[0]->native;

		//						shader.SetParameters(command_list, &parameters);

		//						command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
		//						command_list->Draw(36, 1, 0, 0);
		//					});
		//				}
		//			}
		//			//end
		//		}

		//		FrameGraphTextureRef brdf_lut = graph_builder.CreateTexture({
		//			.Format = RHI::Format::R16G16_FLOAT,
		//			.Width = 512,
		//			.Height = 512,
		//			.Array_Size = 1,
		//			.Mip_Levels = 1
		//		});

		//		//Integrate BRDF 
		//		{
		//			EnvBRDF shader = EnvBRDF(shader_manager.LoadRHIShader("EnvBRDF", RHI::ShaderType::VERTEX),
		//				shader_manager.LoadRHIShader("EnvBRDF", RHI::ShaderType::PIXEL),
		//				graph_builder.GetDevice());

		//			AT::PipelineStateBuilder pso_builder;
		//			RHI_InputLayout input_layout = {};
		//			input_layout.Input_Elements = {};
		//			input_layout.Input_Stride = 0;
		//			pso_builder.SetRootSignature(shader.GetRootSignature());
		//			pso_builder.SetVertexShader(shader.GetVertexShader());
		//			pso_builder.SetPixelShader(shader.GetPixelShader());
		//			pso_builder.SetPrimiteTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
		//			pso_builder.SetRenderTargets({ RHI::Format::R16G16_FLOAT });
		//			pso_builder.SetDepthEnabled(false);
		//			pso_builder.SetInputLayout(input_layout);
		//			RHI_PipelineState pso = AT::PipelineStateCache::CreateOrGetPipelineState(graph_builder.GetDevice(), pso_builder.Compile());

		//			AT::RenderPassIO pass_io = {};
		//			pass_io.AddRenderTarget(graph_builder.CreateRTV(brdf_lut, {}), RRHI::RenderPassAttachment::LoadOperation::CLEAR);

		//			// pso, shader, kernel_data, srv, SSAO_IO
		//			graph_builder.AddRenderPass("BRDF_LUT", 512, 512, pass_io, [=, &graph_builder](RHI_CommandList command_list) {
		//				command_list->SetPipelineState(pso);
		//				command_list->SetGraphicsRootSignature(shader.GetRootSignature());


		//				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
		//				command_list->Draw(4, 1, 0, 0);
		//				});
		//		}
		//		return GenerateEnviromentMapOutput{
		//			.irradiance = irradiance_map,
		//			.prefiltered = prefiltered_map,
		//			.env_brdf = brdf_lut
		//		};
		//}
		
		struct ReflectionProbeOut {
			FrameGraphTextureRef prefiltered_map;
			FrameGraphTextureRef brdf_lut;
		};

		inline FrameGraphTextureRef GenerateMipForTexture(const std::string& name,
			AT::FrameGraphBuilder& graph_builder,
			AT::GPUShaderManager& shader_manager,
			AT::GPUPipelineStateManager& pipeline_state_manager,
			AT::GPURootSignatureManager& root_signature_manager,
			FrameGraphTextureRef texture, FrameGraphTextureRef output_texture, uint32_t array_slice, uint16_t mip_levels) {
			
			AT::GenerateMipChain2DShader shader = AT::GenerateMipChain2DShader(shader_manager.LoadRHIShader("GenerateMipChain", RHI::ShaderType::COMPUTE), root_signature_manager);

			GPUComputePipelineStateBuilder pso_builder;
			pso_builder.SetRootSignature(shader.GetRootSignature())
				.SetComputeShader(shader.GetComputeShader());

			RHI::PipelineState pso = pipeline_state_manager.CreateOrGetComputePipelineState(pso_builder.ToRHIDescription());

			FrameGraphRenderPassIO io = {};
			FrameGraphSRVRef srv = graph_builder.CreateSRV(texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MipLevels = 1 });
			io.AddShaderResource(srv);
			io.AddUnorderedAccessWrite(graph_builder.CreateUAV(output_texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MipSlice = 0 }));
			std::vector<FrameGraphUAVRef> uavs;
			for (uint32_t i = 0u; i < mip_levels; ++i) {
				uavs.push_back(graph_builder.CreateUAV(output_texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MipSlice = i }));
			}
			graph_builder.AddRenderPass(name, io, FrameGraphRenderPass::PIPELINE_TYPE::COMPUTE, [=, &graph_builder](RHI::CommandList command_list) {

				std::vector<AT::GenerateMipChain2DShader::ComputeGroup*> compute_groups = graph_builder.AllocateShaderParameterGroup<AT::GenerateMipChain2DShader::ComputeGroup>(shader.GetRootSignature(), 0, mip_levels);
				for (uint32_t i = 0u; i < mip_levels; ++i) {
					AT::GenerateMipChain2DShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::GenerateMipChain2DShader::Parameters>();
					uint32_t width = texture->Description.Width >> (i);
					uint32_t height = texture->Description.Height >> (i);
					//set compute groups values.
					compute_groups[i]->constant_buffer = graph_builder.AllocateConstantBuffer<AT::GenerateMipChain2DShader::ComputeGroup::Constants>();
					compute_groups[i]->constants.Output_Resolution = { (float)width, (float)height };

					compute_groups[i]->Source_Texture.srv = srv->RHIHandle;
					compute_groups[i]->Output_Texture.uav = uavs[i]->RHIHandle;

					parameters.compute = compute_groups[i];
					command_list->SetPipelineState(pso);
					command_list->SetComputeRootSignature(shader.GetRootSignature());
					shader.SetParameters(command_list, &parameters);
					command_list->Dispatch(std::ceil(width / 8.0f), std::ceil(height / 8.0f), 1);
				}
			});
			return output_texture;
		}

		inline ReflectionProbeOut ReflectionProbe(const std::string& name,
			AT::FrameGraphBuilder& graph_builder,
			AT::GPUShaderManager& shader_manager,
			AT::GPUPipelineStateManager& pipeline_state_manager,
			AT::GPURootSignatureManager& root_signature_manager,
			uint32_t width,
			uint32_t height,
			AT::RenderData& render_data,
			std::array <FrameGraphSRVRef, 5>& csm,
			std::array<FrameGraphSRVRef, 5>& osm,
			FrameGraphSRVRef null_texture_srv,
			FrameGraphSRVRef null_cube_texture_srv,
			RHI::Texture noise_texture,
			const std::vector<DirectX::XMFLOAT4A>& kernel_data,
			std::array<RenderData, 6>& render_infos,
			FrameGraphSRVRef skybox_srv) {

			ReflectionProbeOut output;

			const DirectX::XMVECTOR UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR DOWN = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR LEFT = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			const DirectX::XMVECTOR FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
			const DirectX::XMVECTOR BACKWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			const DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 128.0f / 128.0f, 1000.0f, 1.0f);
			std::vector<DirectX::XMMATRIX> view_matrices;
			DirectX::XMVECTOR position = DirectX::XMLoadFloat4A(&render_data.ReflectionProbe->Position);
			view_matrices.push_back(DirectX::XMMatrixLookToRH(position, RIGHT, UP));
			view_matrices.push_back(DirectX::XMMatrixLookToRH(position, LEFT, UP));
			view_matrices.push_back(DirectX::XMMatrixLookToRH(position, UP, BACKWARD));
			view_matrices.push_back(DirectX::XMMatrixLookToRH(position, DOWN, FORWARD));
			view_matrices.push_back(DirectX::XMMatrixLookToRH(position, BACKWARD, UP));
			view_matrices.push_back(DirectX::XMMatrixLookToRH(position, FORWARD, UP));

			FrameGraphTextureRef base_color = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM_SRGB, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
			FrameGraphTextureRef normal = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_UNORM, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
			FrameGraphTextureRef surface = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
			RHI::TextureClearValue clear_value;
			clear_value.DepthAndStencil.Depth = 0.0f;
			clear_value.DepthAndStencil.Stencil = 0u;
			FrameGraphTextureRef depth = graph_builder.CreateTexture({ .Format = RHI::Format::D32_FLOAT_S8X24_UINT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1, .Clear_Value = clear_value });
			FrameGraphTextureRef ssao = graph_builder.CreateTexture({ .Format = RHI::Format::R8_UNORM, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
			FrameGraphTextureRef composite = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
			FrameGraphTextureRef composite_mipped = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 8 });
			FrameGraphTextureRef Prefiltered_Map = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 8 });
			output.prefiltered_map = Prefiltered_Map;
			for (uint32_t i = 0u; i < 6u; ++i) {
				render_infos[i] = render_data;
				render_infos[i].CameraPosition = render_data.CameraPosition;
				render_infos[i].ProjectionMatrix = projection_matrix;
				render_infos[i].ViewMatrix = view_matrices[i];
				render_infos[i].ViewProjectionMatrix = view_matrices[i] * projection_matrix;
				render_infos[i].InverseProjectionMatrix = DirectX::XMMatrixInverse(nullptr, projection_matrix);
				render_infos[i].InverseViewMatrix = DirectX::XMMatrixInverse(nullptr, view_matrices[i]);
				//AT::RenderData& render_data2 = render_data[i];
				/*render_info2.CameraPosition = render_data.CameraPosition;
				render_info2.ProjectionMatrix = projection_matrix;
				render_info2.ViewMatrix = view_matrices[i];
				render_info2.ViewProjectionMatrix = view_matrices[i] * projection_matrix;
				render_info2.InverseProjectionMatrix = DirectX::XMMatrixInverse(nullptr, projection_matrix);
				render_info2.InverseViewMatrix = DirectX::XMMatrixInverse(nullptr, view_matrices[i]);*/
				render_data.ReflectionProbe;
				FrameGraphDSVRef depth_buffer_dsv = graph_builder.CreateDSV(depth, { .FirstSliceIndex = i, .ArraySize = 1 });
				DepthPass("Reflection_Probe_Depth_Pass_" + std::to_string(i), graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_infos[i], depth_buffer_dsv);
				FrameGraphRTVRef base_color_rtv = graph_builder.CreateRTV(base_color, { .FirstSliceIndex = i,. ArraySize = 1, .MipSlice = 0 });
				FrameGraphRTVRef normal_rtv = graph_builder.CreateRTV(normal, { .FirstSliceIndex = i,. ArraySize = 1, .MipSlice = 0 });
				FrameGraphRTVRef surface_rtv = graph_builder.CreateRTV(surface, { .FirstSliceIndex = i,. ArraySize = 1, .MipSlice = 0 });
				GBuffer("Reflection_Probe_GBuffer_Pass_" + std::to_string(i), graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_infos[i], base_color_rtv, normal_rtv, surface_rtv, depth_buffer_dsv);
				FrameGraphRTVRef ssao_rtv = graph_builder.CreateRTV(ssao, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
				FrameGraphSRVRef depth_srv = graph_builder.CreateSRV(depth, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
				FrameGraphSRVRef normal_srv = graph_builder.CreateSRV(normal, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
				SSAO("Reflection_Probe_SSAO_Pass_" + std::to_string(i), graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_infos[i], ssao_rtv, depth_srv, normal_srv, 4, noise_texture, kernel_data);
				FrameGraphSRVRef base_color_srv = graph_builder.CreateSRV(base_color, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
				FrameGraphSRVRef surface_srv = graph_builder.CreateSRV(surface, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
				FrameGraphSRVRef ssao_srv = graph_builder.CreateSRV(ssao, { .FirstSliceIndex = i, . ArraySize = 1, .MipLevels = 1 });


				FrameGraphRTVRef composite_rtv = graph_builder.CreateRTV(composite, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
				Skybox("Reflection_Probe_Skybox_Pass_" + std::to_string(i) + "_", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, render_infos[i], width, height, skybox_srv, composite_rtv);
				Deferred("Reflection_Probe_Deferred_Pass_" + std::to_string(i) + "_", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_infos[i], composite_rtv, base_color_srv, normal_srv, surface_srv, depth_srv, ssao_srv, csm, osm, null_cube_texture_srv, null_cube_texture_srv, null_texture_srv);
				GenerateMipForTexture("Reflection_Probe_Generate_Mips_Pass_" + std::to_string(i), graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, composite, composite_mipped, i, 8);
				FrameGraphSRVRef composite_srv = graph_builder.CreateSRV(composite_mipped, {.FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 8});
				//Prefilter
				{
					DirectX::XMVECTOR position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
					std::vector<DirectX::XMMATRIX> view_matrices;
					view_matrices.push_back(DirectX::XMMatrixLookToRH(position, RIGHT, UP));
					view_matrices.push_back(DirectX::XMMatrixLookToRH(position, LEFT, UP));
					view_matrices.push_back(DirectX::XMMatrixLookToRH(position, UP, BACKWARD));
					view_matrices.push_back(DirectX::XMMatrixLookToRH(position, DOWN, FORWARD));
					view_matrices.push_back(DirectX::XMMatrixLookToRH(position, BACKWARD, UP));
					view_matrices.push_back(DirectX::XMMatrixLookToRH(position, FORWARD, UP));
					for (uint32_t j = 0; j < 8u; ++j) {
						FrameGraphRenderPassIO pass_io = {};
						pass_io.AddShaderResource(composite_srv);
						pass_io.AddRenderTarget(graph_builder.CreateRTV(Prefiltered_Map, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = j }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
						AT::PreFilteredEnviromentMapShader shader = AT::PreFilteredEnviromentMapShader(shader_manager.LoadRHIShader("PreFilterMap", RHI::ShaderType::VERTEX),
							shader_manager.LoadRHIShader("PreFilterMap", RHI::ShaderType::PIXEL),
							root_signature_manager);
						GPUGraphicsPipelineStateBuilder pso_builder;
						pso_builder.SetRootSignature(shader.GetRootSignature())
							.SetVertexShader(shader.GetVertexShader())
							.SetPixelShader(shader.GetPixelShader())
							.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
							.SetRenderTargetFormats({ RHI::Format::R16G16B16A16_FLOAT })
							.SetDepthEnabled(false)
							.SetCullMode(RHI::CullMode::BACK)
							.SetFrontCounterClockwise(true)
							.SetFillMode(RHI::FillMode::SOLID);
						RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());
						graph_builder.AddRenderPass("PrefilteredMap" + std::to_string(i) + "Mip" + std::to_string(j), pass_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
							command_list->SetPipelineState(pso);
							command_list->SetGraphicsRootSignature(shader.GetRootSignature());

							RHI::Viewport viewport = {};
							viewport.TopLeftX = 0.0f;
							viewport.TopLeftY = 0.0f;
							viewport.Width = width >> j;
							viewport.Height = height >> j;
							viewport.MinDepth = 0.0f;
							viewport.MaxDepth = 1.0f;
							command_list->SetViewport(viewport);

							RHI::Rect scissor_rect = {};
							scissor_rect.Top = 0;
							scissor_rect.Left = 0;
							scissor_rect.Bottom = height >> j;
							scissor_rect.Right = width >> j;
							command_list->SetScissorRect(scissor_rect);

							PreFilteredEnviromentMapShader::Parameters& parameters = graph_builder.AllocateShaderParameters<PreFilteredEnviromentMapShader::Parameters>();
							parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::PreFilteredEnviromentMapShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
							parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::PreFilteredEnviromentMapShader::PassGroup::Constants>();

							DirectX::XMStoreFloat4x4(&parameters.Pass->constants.MV_Matrix, view_matrices[i] * DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 128.0f / 128.0f, 1.0f, 10.0f));
							parameters.Pass->constants.Roughness = (float)j / (float)(Prefiltered_Map->Description.MipLevels - 1);
							parameters.Pass->constants.Mip_Slice = j;
							parameters.Pass->Enviroment_Map.srv = pass_io.m_shader_resources[0]->RHIHandle;

							shader.SetParameters(command_list, &parameters);

							command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
							command_list->Draw(36, 1, 0, 0);
						});
					}
				}
			}

			FrameGraphTextureRef BRDF_Lut = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16_UNORM, .Width = 512, .Height = 512, .ArraySize = 1, .MipLevels = 1 });
			FrameGraphRTVRef BRDF_Lut_rtv = graph_builder.CreateRTV(BRDF_Lut, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
			output.brdf_lut = BRDF_Lut;
			//Integrate BRDF 
			{
				EnvBRDF shader = EnvBRDF(shader_manager.LoadRHIShader("EnvBRDF", RHI::ShaderType::VERTEX),
					shader_manager.LoadRHIShader("EnvBRDF", RHI::ShaderType::PIXEL),
					root_signature_manager);

				GPUGraphicsPipelineStateBuilder pso_builder;
				pso_builder.SetRootSignature(shader.GetRootSignature())
					.SetVertexShader(shader.GetVertexShader())
					.SetPixelShader(shader.GetPixelShader())
					.SetRenderTargetFormats({ RHI::Format::R16G16_UNORM })
					.SetDepthEnabled(false)
					.SetCullMode(RHI::CullMode::BACK)
					.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
					.SetFrontCounterClockwise(true)
					.SetFillMode(RHI::FillMode::SOLID);

				RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

				FrameGraphRenderPassIO pass_io = {};
				pass_io.AddRenderTarget(BRDF_Lut_rtv, RHI::RenderPassAttachment::LoadOperation::CLEAR);

				// pso, shader, kernel_data, srv, SSAO_IO
				graph_builder.AddRenderPass("BRDF_LUT", pass_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
					command_list->SetPipelineState(pso);
					command_list->SetGraphicsRootSignature(shader.GetRootSignature());

					RHI::Viewport viewport = {};
					viewport.TopLeftX = 0.0f;
					viewport.TopLeftY = 0.0f;
					viewport.Width = 512;
					viewport.Height = 512;
					viewport.MinDepth = 0.0f;
					viewport.MaxDepth = 1.0f;
					command_list->SetViewport(viewport);

					RHI::Rect scissor_rect = {};
					scissor_rect.Top = 0;
					scissor_rect.Left = 0;
					scissor_rect.Bottom = 512;
					scissor_rect.Right = 512;
					command_list->SetScissorRect(scissor_rect);

					command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
					command_list->Draw(4, 1, 0, 0);
				});
			}
			return output;
		}
	}
}
#endif