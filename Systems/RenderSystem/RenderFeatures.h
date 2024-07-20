#ifndef _AT_RENDER_SYSTEM_RENDER_FEATURES_H_
#define _AT_RENDER_SYSTEM_RENDER_FEATURES_H_
#include <string>
#include "FrameGraphBuilder.h"
#include "GPUShaderManager.h"
#include "GPUPipelineStateManager.h"
#include "GPURootSignatureManager.h"
#include "../RenderData.h"
#include "./VertexFormats.h"
#include "./MeshManager.h"
#include "./MaterialManager.h"
//Shader Includes
#include "ShaderPrograms/GenerateMipChain2DShader.h"
#include "ShaderPrograms/SkyboxShader.h"
#include "ShaderPrograms/DepthPrepassShader.h"
#include "ShaderPrograms/GBufferShader.h"
#include "ShaderPrograms/CascadedShadowMappingShader.h"
#include "ShaderPrograms/OmnidirectionalShadowMappingShader.h"
#include "ShaderPrograms/DeferredShader.h"
#include "ShaderPrograms/ScreenSpaceAmbientOcclusionShader.h"
#include "ShaderPrograms/ScreenSpaceAmbientOcclusionBlurShader.h"
#include "ShaderPrograms/DiffuseIndirectShader.h"
#include "ShaderPrograms/ReflectionShader.h"
#include "ShaderPrograms/IrradianceMap.h"
#include "ShaderPrograms/PreFilterEnviromentMapShader.h"
#include "ShaderPrograms/IntegrateBRDFShader.h"
#include "ShaderPrograms/ScreenSpaceReflection.h"
#include "ShaderPrograms/ToneMappingShader.h"
#include "ShaderPrograms/ReSTIRCompositeShader.h"
static RHI::IRayTracingTopLevelAccelerationStructure* scene_srv;
namespace AT::RenderFeatures {
	//Generate Mips Pass
	inline void GenerateMipMaps(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t array_slice,
		uint16_t mip_levels,
		FrameGraphTextureRef input_texture, 
		FrameGraphTextureRef output_texture
	) {
		AT::GenerateMipChain2DShader shader = AT::GenerateMipChain2DShader(shader_manager.LoadRHIShader("GenerateMipChain", RHI::ShaderType::COMPUTE), root_signature_manager);

		GPUComputePipelineStateBuilder pso_builder;
		pso_builder.SetRootSignature(shader.GetRootSignature())
			.SetComputeShader(shader.GetComputeShader());

		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetComputePipelineState(pso_builder.ToRHIDescription());

		FrameGraphRenderPassIO io = {};
		FrameGraphSRVRef srv = graph_builder.CreateSRV(input_texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MipLevels = 1 });
		io.AddShaderResource(srv);
		io.AddUnorderedAccessWrite(graph_builder.CreateUAV(output_texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MipSlice = 0 }));
		std::vector<FrameGraphSRVRef> srvs;
		srvs.push_back(srv);
		std::vector<FrameGraphUAVRef> uavs;
		for (uint32_t i = 0u; i < mip_levels; ++i) {
			if (i != 0u) {
				srvs.push_back(graph_builder.CreateSRV(output_texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MostDetailedMip = i - 1, .MipLevels = 1 }));
			}
			uavs.push_back(graph_builder.CreateUAV(output_texture, { .FirstSliceIndex = array_slice, .ArraySize = 1, .MipSlice = i }));
		}
		graph_builder.AddRenderPass(name, io, FrameGraphRenderPass::PIPELINE_TYPE::COMPUTE, [=, &graph_builder](RHI::CommandList command_list) {
			std::vector<AT::GenerateMipChain2DShader::ComputeGroup*> compute_groups = graph_builder.AllocateShaderParameterGroup<AT::GenerateMipChain2DShader::ComputeGroup>(shader.GetRootSignature(), 0, mip_levels);
			
			std::vector<RHI::ResourceBarrier> final_barriers;
			
			for (uint32_t i = 0u; i < mip_levels; ++i) {
				AT::GenerateMipChain2DShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::GenerateMipChain2DShader::Parameters>();
				uint32_t width = input_texture->Description.Width >> (i);
				uint32_t height = input_texture->Description.Height >> (i);
				compute_groups[i]->constant_buffer = graph_builder.AllocateConstantBuffer<AT::GenerateMipChain2DShader::ComputeGroup::Constants>();
				compute_groups[i]->constants.OutputResolution = { (float)width, (float)height };
				compute_groups[i]->SourceTexture.srv = srvs[i]->RHIHandle;
				compute_groups[i]->OutputTexture.uav = uavs[i]->RHIHandle;

				parameters.Compute = compute_groups[i];
				command_list->SetPipelineState(pso);
				command_list->SetComputeRootSignature(shader.GetRootSignature());
				shader.SetParameters(command_list, &parameters);
				command_list->Dispatch(std::ceil(width / 8.0f), std::ceil(height / 8.0f), 1);

				{
					RHI::ResourceBarrier resource_barrier;
					resource_barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
					resource_barrier.TransitionBarrierTexture.Texture = output_texture->RHIHandle;
					resource_barrier.TransitionBarrierTexture.Subresource.ArraySlice = array_slice;
					resource_barrier.TransitionBarrierTexture.Subresource.MipSlice = i;
					resource_barrier.TransitionBarrierTexture.Subresource.MipLevels = 8;
					resource_barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::UNORDERED_ACCESS;
					resource_barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::NON_PIXEL_SHADER_RESOURCE;
					command_list->ResourceBarrier(1, &resource_barrier);

				}
				{
					RHI::ResourceBarrier resource_barrier;
					resource_barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
					resource_barrier.TransitionBarrierTexture.Texture = output_texture->RHIHandle;
					resource_barrier.TransitionBarrierTexture.Subresource.ArraySlice = array_slice;
					resource_barrier.TransitionBarrierTexture.Subresource.MipSlice = i;
					resource_barrier.TransitionBarrierTexture.Subresource.MipLevels = 8;
					resource_barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::NON_PIXEL_SHADER_RESOURCE;
					resource_barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::UNORDERED_ACCESS;
					final_barriers.push_back(resource_barrier);
				}
			}
			command_list->ResourceBarrier(final_barriers.size(), final_barriers.data());
		});
	}
	//Skybox Pass.
	inline void Skybox(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphSRVRef input_skybox,
		FrameGraphRTVRef output_texture
	) {
		FrameGraphRenderPassIO pass_io = {};
		pass_io.AddShaderResource(input_skybox);
		pass_io.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);
		AT::SkyboxShader shader = AT::SkyboxShader(
			shader_manager.LoadRHIShader("Skybox", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("Skybox", RHI::ShaderType::PIXEL),
			root_signature_manager
		);

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
		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

		//Translate View Matrix
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
			DirectX::XMStoreFloat4x4(&parameters.Pass->constants.ModelViewMatrix, new_view_matrix * render_data.ProjectionMatrix);
			parameters.Pass->SkyboxTexture.srv = pass_io.m_ShaderResources[0]->RHIHandle;
			shader.SetParameters(command_list, &parameters);
			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
			command_list->Draw(36, 1, 0, 0);
		});
	}
	//Depth PrePass.
	inline void DepthPrePass(const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		const AT::RenderData& render_data,
		FrameGraphDSVRef output) {
		AT::DepthPrepassShader shader = AT::DepthPrepassShader(
			shader_manager.LoadRHIShader("Depth", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("Depth", RHI::ShaderType::PIXEL),
			root_signature_manager
		);
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
		IO.SetDepthStencilBuffer(output, RHI::RenderPassAttachment::LoadOperation::CLEAR);

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
				parameters.Object = object_groups[i];
				parameters.Object->constant_buffer = graph_builder.AllocateConstantBuffer<AT::DepthPrepassShader::ObjectGroup::Constants>();
				const AT::RenderObject& render_object = render_data.RenderObjects[i];
				const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];
				DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);
				DirectX::XMStoreFloat4x4A(&parameters.Object->constants.ModelViewProjectionMatrix, DirectX::XMMatrixTranspose(model_matrix) * render_data.ViewProjectionMatrix);
				DirectX::XMStoreFloat4x4A(&parameters.Object->constants.NormalMatrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, model_matrix * render_data.ViewMatrix)));
				parameters.Object->BaseColorMap.srv = render_data.Materials[mesh->MaterialID]->BaseColorSRV;
				shader.SetParameters(command_list, &parameters);
				command_list->SetVertexBuffer(0, 1, &mesh->VBView);
				command_list->SetIndexBuffer(mesh->IBView);
				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
				command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
			}
		});
	}

	//GBuffer Pass.
	inline void GBuffer(const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		AT::RenderData& render_data,
		AT::FrameGraphDSVRef input_depth_buffer,
		AT::FrameGraphRTVRef output_base_color,
		AT::FrameGraphRTVRef output_normal,
		AT::FrameGraphRTVRef output_surface
	) {

		AT::GBufferShader shader = AT::GBufferShader(
			shader_manager.LoadRHIShader("GBuffer", RHI::ShaderType::VERTEX),
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
		IO.AddRenderTarget(output_base_color, RHI::RenderPassAttachment::LoadOperation::CLEAR);
		IO.AddRenderTarget(output_normal, RHI::RenderPassAttachment::LoadOperation::CLEAR);
		IO.AddRenderTarget(output_surface, RHI::RenderPassAttachment::LoadOperation::CLEAR);
		IO.SetDepthStencilBufferAsDependency(input_depth_buffer, RHI::RenderPassAttachment::LoadOperation::LOAD);
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
				parameters.Object = object_groups[i];
				parameters.Object->constant_buffer = graph_builder.AllocateConstantBuffer<AT::GBufferShader::ObjectGroup::Constants>();
				parameters.Material = material_groups[i];

				const AT::RenderObject& render_object = render_data.RenderObjects[i];
				const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];
				DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);

				DirectX::XMStoreFloat4x4A(&parameters.Object->constants.ModelViewProjectionMatrix, DirectX::XMMatrixTranspose(model_matrix) * render_data.ViewProjectionMatrix);
				DirectX::XMStoreFloat4x4A(&parameters.Object->constants.NormalMatrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, model_matrix /* render_data.ViewMatrix*/)));

				parameters.Material->BaseColor.srv = render_data.Materials[mesh->MaterialID]->BaseColorSRV;
				parameters.Material->Normals.srv = render_data.Materials[mesh->MaterialID]->NormalSRV;
				parameters.Material->Surface.srv = render_data.Materials[mesh->MaterialID]->RoughnessMetalnessSRV;
				parameters.Material->constant_buffer = &render_data.Materials[mesh->MaterialID]->FactorConstantBuffer;

				shader.SetParameters(command_list, &parameters);

				command_list->SetVertexBuffer(0, 1, &mesh->VBView);
				command_list->SetIndexBuffer(mesh->IBView);
				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
				command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
			}
		});
	}

	//Cascaded Shadow Map
	inline void CascadedShadowMapping(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		AT::RenderData& render_data,
		uint32_t light_index,
		AT::FrameGraphTextureRef output_shadow_map) {

		const DirectX::XMFLOAT2A partitions[4] = {
			{ 1.0f, 20.0f },
			{ 20.0f, 100.0f},
			{ 100.0f, 300.0f },
			{ 300.0f , 600.0f }
		};
		
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

		AT::CascadedShadowMappingShader shader = AT::CascadedShadowMappingShader(
			shader_manager.LoadRHIShader("CascadedShadowMap", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("CascadedShadowMap", RHI::ShaderType::PIXEL),
			root_signature_manager
		);
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
		for (uint32_t i = 0u; i < 4; ++i) {
			float near_distance = partitions[i].x / 1.0f;
			float far_distance = partitions[i].y / 1.0f;
			DirectX::XMMATRIX camera_projection = DirectX::XMMatrixPerspectiveFovRH(render_data.FOV, 1280.0f / 720.0f, far_distance, near_distance);
			DirectX::XMMATRIX inverse_view_projection = DirectX::XMMatrixInverse(nullptr, render_data.ViewMatrix * camera_projection);
			DirectX::XMVECTOR center = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			DirectX::XMFLOAT4A frustrum_corners[8];
			for (uint32_t j = 0u; j < 8u; ++j) {
				DirectX::XMVECTOR world_space = DirectX::XMVector4Transform(DirectX::XMLoadFloat4A(&frustrum_points[j]), inverse_view_projection);
				DirectX::XMFLOAT4A world_space_float4;
				DirectX::XMStoreFloat4A(&world_space_float4, world_space);
				world_space = DirectX::XMVectorScale(world_space, 1.0f / world_space_float4.w);
				DirectX::XMStoreFloat4A(&frustrum_corners[j], world_space);
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

			float units = width / radius / 2.0f;
			DirectX::XMMATRIX scalar_matrix = DirectX::XMMatrixScaling(units, units, units);
			DirectX::XMVECTOR light_direction = DirectX::XMLoadFloat4A(&render_data.Lights[light_index].PositionOrDirection);
			DirectX::XMMATRIX look_at = DirectX::XMMatrixLookAtRH(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVector3Normalize(light_direction), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			look_at = scalar_matrix * look_at;

			//Texel Snapping.
			DirectX::XMFLOAT4 new_center4;
			DirectX::XMStoreFloat4(&new_center4, center);
			new_center4.w = 1.0f;
			DirectX::XMVECTOR snap_center = DirectX::XMVector4Transform(DirectX::XMLoadFloat4(&new_center4), look_at);
			DirectX::XMFLOAT4 snap_center4;
			DirectX::XMStoreFloat4(&snap_center4, snap_center);
			snap_center4.x = std::ceil(snap_center4.x / snap_center4.w);
			snap_center4.y = std::ceil(snap_center4.y / snap_center4.w);
			snap_center4.z = std::ceil(snap_center4.z / snap_center4.w);
			snap_center4.w = 1.0f;
			snap_center = DirectX::XMLoadFloat4(&snap_center4);

			DirectX::XMVECTOR new_center = DirectX::XMVector4Transform(snap_center, DirectX::XMMatrixInverse(nullptr, look_at));
			DirectX::XMMATRIX light_view_matrix = DirectX::XMMatrixLookAtRH(DirectX::XMVectorAdd(new_center, DirectX::XMVectorScale(DirectX::XMVector3Normalize(light_direction), radius * 1.0f)), new_center, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

			float scale = 5.0f;
			DirectX::XMMATRIX ortho = DirectX::XMMatrixOrthographicOffCenterRH(-radius, radius, -radius, radius, 2.0f * radius, -radius);
			DirectX::XMMATRIX light_matrix = light_view_matrix * ortho;
			DirectX::XMStoreFloat4x4A(&render_data.Lights[light_index].LightMatrices[i], light_matrix);
			AT::FrameGraphRenderPassIO csmIO = {};
			csmIO.SetDepthStencilBuffer(graph_builder.CreateDSV(output_shadow_map, { .FirstSliceIndex = i, .ArraySize = 1 }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
			graph_builder.AddRenderPass(name + std::to_string(i), csmIO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {

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

				AT::CascadedShadowMappingShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::CascadedShadowMappingShader::Parameters>();
				std::vector<AT::CascadedShadowMappingShader::ObjectGroup*> object_groups = graph_builder.AllocateShaderParameterGroup<AT::CascadedShadowMappingShader::ObjectGroup>(shader.GetRootSignature(), 0, render_data.RenderObjects.size());
				for (uint32_t j = 0; j < render_data.RenderObjects.size(); ++j) {
					const AT::RenderObject& render_object = render_data.RenderObjects[j];
					const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];
					DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);

					parameters.Object = object_groups[j];
					parameters.Object->constant_buffer = graph_builder.AllocateConstantBuffer<AT::CascadedShadowMappingShader::ObjectGroup::Constants>();
					DirectX::XMStoreFloat4x4A(&parameters.Object->constants.ModelViewProjectionMatrix, DirectX::XMMatrixTranspose(model_matrix) * light_matrix);

					shader.SetParameters(command_list, &parameters);

					command_list->SetVertexBuffer(0, 1, &mesh->VBView);
					command_list->SetIndexBuffer(mesh->IBView);
					command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
					command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
				}
				});
		}
	}

	//Omnidirectional Shadow Mapping
	inline void OmnidirectionalShadowMapping(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		AT::RenderData& render_data,
		uint32_t light_index,
		AT::FrameGraphTextureRef output_shadow_map
	) {
		const DirectX::XMVECTOR UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR DOWN = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR LEFT = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		const DirectX::XMVECTOR BACKWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		std::vector<DirectX::XMMATRIX> matrices = std::vector<DirectX::XMMATRIX>();
		float Max_Distance = render_data.Lights[light_index].Radius;
		DirectX::XMVECTOR light_position = DirectX::XMLoadFloat4A(&render_data.Lights[light_index].PositionOrDirection);
		DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovLH(0.5f * 3.14159f, (float)width / (float)height, 1.0f, Max_Distance);
		matrices.push_back(DirectX::XMMatrixLookToLH(light_position, RIGHT, UP) * projection_matrix);
		matrices.push_back(DirectX::XMMatrixLookToLH(light_position, LEFT, UP) * projection_matrix);
		matrices.push_back(DirectX::XMMatrixLookToLH(light_position, UP, FORWARD) * projection_matrix);
		matrices.push_back(DirectX::XMMatrixLookToLH(light_position, DOWN, BACKWARD) * projection_matrix);
		matrices.push_back(DirectX::XMMatrixLookToLH(light_position, BACKWARD, UP) * projection_matrix);
		matrices.push_back(DirectX::XMMatrixLookToLH(light_position, FORWARD, UP) * projection_matrix);

		AT::OmnidirectionalShadowMappingShader shader = AT::OmnidirectionalShadowMappingShader(
			shader_manager.LoadRHIShader("OmnidirectionalShadowMap", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("OmnidirectionalShadowMap", RHI::ShaderType::PIXEL),
			root_signature_manager
		);

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
			osmIO.SetDepthStencilBuffer(graph_builder.CreateDSV(output_shadow_map, { .FirstSliceIndex = i, .ArraySize = 1 }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
			graph_builder.AddRenderPass(name + std::to_string(i), osmIO, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder, &render_data](RHI::CommandList command_list) {

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

				AT::OmnidirectionalShadowMappingShader::Parameters& parameters = graph_builder.AllocateShaderParameters<AT::OmnidirectionalShadowMappingShader::Parameters>();
				std::vector<AT::OmnidirectionalShadowMappingShader::PassGroup*> pass_group = graph_builder.AllocateShaderParameterGroup<AT::OmnidirectionalShadowMappingShader::PassGroup>(shader.GetRootSignature(), 0, render_data.RenderObjects.size());
				for (uint32_t j = 0; j < render_data.RenderObjects.size(); ++j) {
					const AT::RenderObject& render_object = render_data.RenderObjects[j];
					const AT::Mesh* mesh = render_data.Meshes[render_object.MeshID];

					DirectX::XMMATRIX model_matrix = DirectX::XMLoadFloat4x4(&render_object.Transform);
					parameters.Pass = pass_group[j];
					parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::OmnidirectionalShadowMappingShader::PassGroup::Constants>();

					DirectX::XMStoreFloat4x4A(&parameters.Pass->constants.ModelViewProjectionMatrix, DirectX::XMMatrixTranspose(model_matrix) * matrices[i]);
					DirectX::XMStoreFloat4x4A(&parameters.Pass->constants.ModelMatrix, model_matrix);
					DirectX::XMStoreFloat4A(&parameters.Pass->constants.LightPosition, light_position);
					parameters.Pass->constants.MaxDistance = Max_Distance;

					shader.SetParameters(command_list, &parameters);

					command_list->SetVertexBuffer(0, 1, &mesh->VBView);
					command_list->SetIndexBuffer(mesh->IBView);
					command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
					command_list->DrawIndexed(mesh->IndicesCount, 1, 0, 0, 0);
				}
			});
		}
	}

	//Deferred Lighting Pass
	inline void Deferred(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		AT::RenderData& render_data,
		FrameGraphSRVRef input_base_color,
		FrameGraphSRVRef input_normal,
		FrameGraphSRVRef input_surface,
		FrameGraphSRVRef input_depth,
		std::array <FrameGraphSRVRef, 5>& input_csms,
		std::array<FrameGraphSRVRef, 5>& input_osms,
		FrameGraphRTVRef output_texture
	) {

		FrameGraphRenderPassIO deferredIO = {};
		deferredIO.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::LOAD);
		deferredIO.AddShaderResource(input_base_color);
		deferredIO.AddShaderResource(input_normal);
		deferredIO.AddShaderResource(input_surface);
		deferredIO.AddShaderResource(input_depth);
		for (uint32_t i = 0u; i < 5u; ++i) {
			deferredIO.AddShaderResource(input_csms[i]);
		}
		for (uint32_t i = 0u; i < 5u; ++i) {
			deferredIO.AddShaderResource(input_osms[i]);
		}

		AT::DeferredShader shader = AT::DeferredShader(
			shader_manager.LoadRHIShader("Deferred", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("Deferred", RHI::ShaderType::PIXEL),
			root_signature_manager
		);

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
			deferred_parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::DeferredShader::DEFERREDINPUT>(shader.GetRootSignature(), 0, 1)[0];
			deferred_parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::DeferredShader::DEFERREDINPUT::Constants>();

			for (uint32_t i = 0u; i < 5u; ++i) {
				DirectX::XMVECTOR light_direction_or_position = DirectX::XMLoadFloat4A(&render_data.Lights[i].PositionOrDirection);
				deferred_parameters.Pass->constants.Lights.lights[i].Type = render_data.Lights[i].Type;
				deferred_parameters.Pass->constants.Lights.lights[i].Intensity = render_data.Lights[i].Intensity;
				deferred_parameters.Pass->constants.Lights.lights[i].Color = render_data.Lights[i].Color;
				switch (render_data.Lights[i].Type) {
				case LIGHT_TYPE_DIRECTIONAL:
				{
					DirectX::XMStoreFloat4A(&(deferred_parameters.Pass->constants.Lights.lights[i].PositionOrDirection), DirectX::XMVector4Transform(DirectX::XMVector4Normalize(light_direction_or_position), render_data.ViewMatrix));
					for (uint32_t j = 0u; j < 4; ++j) {
						deferred_parameters.Pass->constants.Lights.lights[i].LightMatrices[j] = render_data.Lights[i].LightMatrices[j];
					}
				}
				break;
				case LIGHT_TYPE_POINT:
				{
					DirectX::XMStoreFloat4A(&(deferred_parameters.Pass->constants.Lights.lights[i].PositionOrDirection), DirectX::XMVector4Transform(light_direction_or_position, render_data.ViewMatrix));
					deferred_parameters.Pass->constants.Lights.lights[i].Radius = render_data.Lights[i].Radius;
				}
				break;
				}
			}

			DirectX::XMStoreFloat4x4A(&deferred_parameters.Pass->constants.InverseProjectionMatrix, render_data.InverseProjectionMatrix);
			DirectX::XMStoreFloat4x4A(&deferred_parameters.Pass->constants.InverseViewMatrix, render_data.InverseViewMatrix);


			deferred_parameters.Pass->BaseColorTexture.srv = deferredIO.m_ShaderResources[0]->RHIHandle;
			deferred_parameters.Pass->NormalTexture.srv = deferredIO.m_ShaderResources[1]->RHIHandle;
			deferred_parameters.Pass->SurfaceTexture.srv = deferredIO.m_ShaderResources[2]->RHIHandle;
			deferred_parameters.Pass->DepthTexture.srv = deferredIO.m_ShaderResources[3]->RHIHandle;
			deferred_parameters.Pass->CSM[0].srv = deferredIO.m_ShaderResources[4]->RHIHandle;
			deferred_parameters.Pass->CSM[1].srv = deferredIO.m_ShaderResources[5]->RHIHandle;
			deferred_parameters.Pass->CSM[2].srv = deferredIO.m_ShaderResources[6]->RHIHandle;
			deferred_parameters.Pass->CSM[3].srv = deferredIO.m_ShaderResources[7]->RHIHandle;
			deferred_parameters.Pass->CSM[4].srv = deferredIO.m_ShaderResources[8]->RHIHandle;
			deferred_parameters.Pass->OSM[0].srv = deferredIO.m_ShaderResources[9]->RHIHandle;
			deferred_parameters.Pass->OSM[1].srv = deferredIO.m_ShaderResources[10]->RHIHandle;
			deferred_parameters.Pass->OSM[2].srv = deferredIO.m_ShaderResources[11]->RHIHandle;
			deferred_parameters.Pass->OSM[3].srv = deferredIO.m_ShaderResources[12]->RHIHandle;
			deferred_parameters.Pass->OSM[4].srv = deferredIO.m_ShaderResources[13]->RHIHandle;

			shader.SetParameters(command_list, &deferred_parameters);

			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
			command_list->Draw(4, 1, 0, 0);
		});
	}

	//Screen-Spacee Ambient Occlusion
	inline void SSAO(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		AT::RenderData& render_data,
		uint32_t noise_size,
		RHI::Texture noise_texture,
		const std::vector<DirectX::XMFLOAT4A>& kernel_data,
		FrameGraphSRVRef input_depth,
		FrameGraphSRVRef input_normal,
		FrameGraphRTVRef output_texture
	) {

		SSAOShader shader = SSAOShader(
			shader_manager.LoadRHIShader("SSAO", RHI::ShaderType::VERTEX),
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
		IO.AddShaderResource(input_depth);
		IO.AddShaderResource(input_normal);
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
			parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::SSAOShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
			parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::SSAOShader::PassGroup::Constants>();

			DirectX::XMStoreFloat4x4(&parameters.Pass->constants.ProjectionMatrix, render_data.ProjectionMatrix);
			DirectX::XMStoreFloat4x4(&parameters.Pass->constants.InverseProjectionMatrix, render_data.InverseProjectionMatrix);
			for (uint32_t i = 0u; i < 64u; ++i) {
				parameters.Pass->constants.Kernel[i] = kernel_data[i];
			}

			parameters.Pass->constants.Size = { (float)width, (float)height };
			parameters.Pass->g_DepthTexture.srv = IO.m_ShaderResources[0]->RHIHandle;
			parameters.Pass->g_NormalTexture.srv = IO.m_ShaderResources[1]->RHIHandle;
			parameters.Pass->g_NoiseTexture.srv = IO.m_ShaderResources[2]->RHIHandle;

			shader.SetParameters(command_list, &parameters);

			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
			command_list->Draw(4, 1, 0, 0);
		});

		AT::FrameGraphRenderPassIO IO2 = {};
		IO2.AddShaderResource(graph_builder.CreateSRV(SSAO_Output, { .FirstSliceIndex = 0, .ArraySize = 1, . MipLevels = 1 }));
		IO2.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);
		SSAOBlurShader shader2 = SSAOBlurShader(
			shader_manager.LoadRHIShader("ssao_blur", RHI::ShaderType::VERTEX),
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
			parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::SSAOBlurShader::PassGroup>(shader2.GetRootSignature(), 0, 1)[0];
			parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::SSAOShader::PassGroup::Constants>();
			parameters.Pass->SSAOTexture.srv = IO2.m_ShaderResources[0]->RHIHandle;
			parameters.Pass->constants.Size = { (float)width, (float)height };

			shader2.SetParameters(command_list, &parameters);

			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
			command_list->Draw(4, 1, 0, 0);
		});
	}

	//Diffuse Indirect Pass
	inline void DiffuseIndirectPass(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		RenderData& render_data,
		uint32_t width,
		uint32_t height,
		FrameGraphSRVRef input_scene_color_texture,
		FrameGraphSRVRef input_base_color_texture,
		FrameGraphSRVRef input_normal_texture,
		FrameGraphSRVRef input_roughness_metalness_texture,
		FrameGraphSRVRef input_ssao_texture,
		FrameGraphSRVRef input_diffuse_irradiance_map,
		FrameGraphUAVRef output_texture
	) {
		DiffuseIndirectShader shader = DiffuseIndirectShader(
			shader_manager.LoadRHIShader("DiffuseIndirect", RHI::ShaderType::COMPUTE), 
			root_signature_manager
		);
		GPUComputePipelineStateBuilder pso_builder;
		pso_builder.SetRootSignature(shader.GetRootSignature());
		pso_builder.SetComputeShader(shader.GetComputeShader());
		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetComputePipelineState(pso_builder.ToRHIDescription());
		FrameGraphRenderPassIO io;
		io.AddShaderResource(input_scene_color_texture);
		io.AddShaderResource(input_base_color_texture);
		io.AddShaderResource(input_roughness_metalness_texture);
		io.AddShaderResource(input_ssao_texture);
		io.AddShaderResource(input_diffuse_irradiance_map);
		io.AddUnorderedAccessWrite(output_texture);
		DiffuseIndirectShader::Parameters* parameters = &graph_builder.AllocateShaderParameters<DiffuseIndirectShader::Parameters>();
		DiffuseIndirectShader::ComputeGroup* compute_group = graph_builder.AllocateShaderParameterGroup<DiffuseIndirectShader::ComputeGroup>(shader.GetRootSignature(), 0, 1)[0];
		compute_group->constant_buffer = graph_builder.AllocateConstantBuffer<DiffuseIndirectShader::ComputeGroup::Constants>();
		graph_builder.AddRenderPass(name, io, FrameGraphRenderPass::PIPELINE_TYPE::COMPUTE, [=, &graph_builder](RHI::CommandList command_list) {
			command_list->SetPipelineState(pso);
			command_list->SetComputeRootSignature(shader.GetRootSignature());
			parameters->Compute = compute_group;
			parameters->Compute->SceneColorTexture.srv = input_scene_color_texture->RHIHandle;
			parameters->Compute->BaseColorTexture.srv = input_base_color_texture->RHIHandle;
			parameters->Compute->NormalTexture.srv = input_normal_texture->RHIHandle;
			parameters->Compute->RoughnessMetalnessTexture.srv = input_roughness_metalness_texture->RHIHandle;
			parameters->Compute->SSAOTexture.srv = input_ssao_texture->RHIHandle;
			parameters->Compute->DiffuseIrradianceTexture.srv = input_diffuse_irradiance_map->RHIHandle;
			parameters->Compute->OutputTexture.uav = output_texture->RHIHandle;
			DirectX::XMStoreFloat4x4(&parameters->Compute->constants.InverseViewMatrix, render_data.InverseViewMatrix);
			parameters->Compute->constants.OutputResolution = { (float)width, (float)height };
			shader.SetParameters(command_list, parameters);
			command_list->Dispatch(std::ceil(width / 8.0f), std::ceil(height / 8.0f), 1);
		});
	}

	//Specular Indirect Pass (Reflection Pass)
	inline void ReflectionPass(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphSRVRef input_scene_color_texture,
		FrameGraphSRVRef input_base_color_texture,
		FrameGraphSRVRef input_normal_texture,
		FrameGraphSRVRef input_roughness_metalness_texture,
		FrameGraphSRVRef input_depth_texture,
		FrameGraphSRVRef input_ssao_texture,
		FrameGraphSRVRef input_ssr_texture,
		FrameGraphSRVRef input_brdf_lut_texture,
		FrameGraphSRVRef input_prefiltered_enviroment_map_texture,
		FrameGraphUAVRef output_texture
	) {
		ReflectionShader shader = ReflectionShader(
			shader_manager.LoadRHIShader("Reflection", RHI::ShaderType::COMPUTE), 
			root_signature_manager
		);
		GPUComputePipelineStateBuilder pso_builder;
		pso_builder.SetRootSignature(shader.GetRootSignature());
		pso_builder.SetComputeShader(shader.GetComputeShader());
		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetComputePipelineState(pso_builder.ToRHIDescription());
		FrameGraphRenderPassIO io;
		io.AddShaderResource(input_scene_color_texture);
		io.AddShaderResource(input_base_color_texture);
		io.AddShaderResource(input_roughness_metalness_texture);
		io.AddShaderResource(input_ssao_texture);
		io.AddShaderResource(input_ssr_texture);
		io.AddShaderResource(input_brdf_lut_texture);
		io.AddShaderResource(input_prefiltered_enviroment_map_texture);
		io.AddUnorderedAccessWrite(output_texture);
		ReflectionShader::Parameters* parameters = &graph_builder.AllocateShaderParameters<ReflectionShader::Parameters>();
		ReflectionShader::ComputeGroup* compute_group = graph_builder.AllocateShaderParameterGroup<ReflectionShader::ComputeGroup>(shader.GetRootSignature(), 0, 1)[0];
		compute_group->constant_buffer = graph_builder.AllocateConstantBuffer<ReflectionShader::ComputeGroup::Constants>();
		graph_builder.AddRenderPass("Reflection", io, FrameGraphRenderPass::PIPELINE_TYPE::COMPUTE, [=, &graph_builder](RHI::CommandList command_list) {
			command_list->SetPipelineState(pso);
			command_list->SetComputeRootSignature(shader.GetRootSignature());
			parameters->Compute = compute_group;
			parameters->Compute->SceneColorTexture.srv = input_scene_color_texture->RHIHandle;
			parameters->Compute->BaseColorTexture.srv = input_base_color_texture->RHIHandle;
			parameters->Compute->NormalTexture.srv = input_normal_texture->RHIHandle;
			parameters->Compute->RoughnessMetalnessTexture.srv = input_roughness_metalness_texture->RHIHandle;
			parameters->Compute->DepthTexture.srv = input_depth_texture->RHIHandle;
			parameters->Compute->SSAOTexture.srv = input_ssao_texture->RHIHandle;
			parameters->Compute->SSRTexture.srv = input_ssr_texture->RHIHandle;
			parameters->Compute->BRDFLutTexture.srv = input_brdf_lut_texture->RHIHandle;
			parameters->Compute->PrefilteredEnviromentMapTexture.srv = input_prefiltered_enviroment_map_texture->RHIHandle;
			parameters->Compute->OutputTexture.uav = output_texture->RHIHandle;
			DirectX::XMStoreFloat4x4(&parameters->Compute->constants.InverseProjectionMatrix, render_data.InverseProjectionMatrix);
			DirectX::XMStoreFloat4x4(&parameters->Compute->constants.InverseViewMatrix, render_data.InverseViewMatrix);
			parameters->Compute->constants.OutputResolution = { (float)width, (float)height };
			shader.SetParameters(command_list, parameters);
			command_list->Dispatch(std::ceil(width / 8.0f), std::ceil(height / 8.0f), 1);
		});
	}

	//Diffuse IBL
	inline void IrradianceMap(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data, 
		FrameGraphSRVRef input_enviroment_map,
		FrameGraphTextureRef output_texture
	) {
		AT::IrradianceMapShader shader = AT::IrradianceMapShader(
			shader_manager.LoadRHIShader("IrradianceMap", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("IrradianceMap", RHI::ShaderType::PIXEL),
			root_signature_manager
		);
		AT::GPUGraphicsPipelineStateBuilder pso_builder = {};
		pso_builder.SetRootSignature(shader.GetRootSignature());
		RHI::InputLayout input_layout = {};
		input_layout.InputElements = {};
		input_layout.InputStride = 0;
		pso_builder.SetVertexShader(shader.GetVertexShader())
			.SetPixelShader(shader.GetPixelShader())
			.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST)
			.SetRenderTargetFormats({ RHI::Format::R16G16B16A16_FLOAT })
			.SetDepthEnabled(false)
			.SetCullMode(RHI::CullMode::BACK)
			.SetFrontCounterClockwise(true)
			.SetFillMode(RHI::FillMode::SOLID);

		RHI::GraphicsPipelineStateDescription pso_desc = pso_builder.ToRHIDescription();
		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_desc);

		const DirectX::XMVECTOR UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR DOWN = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR LEFT = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		const DirectX::XMVECTOR BACKWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		const DirectX::XMMATRIX projection_matrix = DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 128.0f / 128.0f, 1.0f, 10.0f);
		std::vector<DirectX::XMMATRIX> view_matrices;
		DirectX::XMVECTOR position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, RIGHT, UP));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, LEFT, UP));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, UP, BACKWARD));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, DOWN, FORWARD));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, BACKWARD, UP));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, FORWARD, UP));

		for (uint32_t i = 0; i < 6; ++i) {
			AT::FrameGraphRenderPassIO pass_io = {};
			pass_io.AddShaderResource(input_enviroment_map);
			pass_io.AddRenderTarget(graph_builder.CreateRTV(output_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
			graph_builder.AddRenderPass(name + "[" + std::to_string(i) + "]", pass_io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
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
				
				IrradianceMapShader::Parameters& parameters = graph_builder.AllocateShaderParameters<IrradianceMapShader::Parameters>();
				parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::IrradianceMapShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
				parameters.Pass->constant_buffer = graph_builder.AllocateConstantBuffer<AT::IrradianceMapShader::PassGroup::Constants>();
				DirectX::XMStoreFloat4x4(&parameters.Pass->constants.ModelViewProjectionMatrix, view_matrices[i] * projection_matrix);
				parameters.Pass->EnviromentMapTexture.srv = pass_io.m_ShaderResources[0]->RHIHandle;
				shader.SetParameters(command_list, &parameters);
				command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
				command_list->Draw(36, 1, 0, 0);
			});
		}
	}

	//Specular IBL
	inline void PreFilterMap(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphSRVRef input_enviroment_map,
		FrameGraphTextureRef output_texture
	) {
		DirectX::XMVECTOR position = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		const DirectX::XMVECTOR UP = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR DOWN = DirectX::XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR LEFT = DirectX::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR RIGHT = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		const DirectX::XMVECTOR FORWARD = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
		const DirectX::XMVECTOR BACKWARD = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		std::vector<DirectX::XMMATRIX> view_matrices;
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, RIGHT, UP));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, LEFT, UP));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, UP, BACKWARD));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, DOWN, FORWARD));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, BACKWARD, UP));
		view_matrices.push_back(DirectX::XMMatrixLookToRH(position, FORWARD, UP));
		for (uint32_t i = 0u; i < 6u; ++i) {
			for (uint32_t j = 0; j < 8u; ++j) {
				FrameGraphRenderPassIO pass_io = {};
				pass_io.AddShaderResource(input_enviroment_map);
				pass_io.AddRenderTarget(graph_builder.CreateRTV(output_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = j }), RHI::RenderPassAttachment::LoadOperation::CLEAR);
				AT::PreFilteredEnviromentMapShader shader = AT::PreFilteredEnviromentMapShader(
					shader_manager.LoadRHIShader("PreFilterMap", RHI::ShaderType::VERTEX),
					shader_manager.LoadRHIShader("PreFilterMap", RHI::ShaderType::PIXEL),
					root_signature_manager
				);
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
				graph_builder.AddRenderPass(name + std::to_string(i) + "Mip" + std::to_string(j), pass_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
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
					DirectX::XMStoreFloat4x4(&parameters.Pass->constants.ModelViewMatrix, view_matrices[i] * DirectX::XMMatrixPerspectiveFovRH(0.5f * 3.14159f, 128.0f / 128.0f, 1.0f, 10.0f));
					parameters.Pass->constants.Roughness = (float)j / (float)(output_texture->Description.MipLevels - 1);
					parameters.Pass->constants.MipSlice = j;
					parameters.Pass->EnviromentMapTexture.srv = pass_io.m_ShaderResources[0]->RHIHandle;
					shader.SetParameters(command_list, &parameters);
					command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_LIST);
					command_list->Draw(36, 1, 0, 0);
				});
			}
		}
	}

	//Integrate BRDF
	inline void IntegrateBRDF(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphRTVRef output_texture
	) {
		IntegrateBRDFShader shader = IntegrateBRDFShader(
			shader_manager.LoadRHIShader("IntegrateBRDF", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("IntegrateBRDF", RHI::ShaderType::PIXEL),
			root_signature_manager
		);

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
		pass_io.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);

		graph_builder.AddRenderPass(name, pass_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
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
			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
			command_list->Draw(4, 1, 0, 0);
		});
	}

	//Screen-Space Reflections
	inline void SSR(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphSRVRef input_scene_color_texture,
		FrameGraphSRVRef input_depth_texture,
		FrameGraphSRVRef input_normal_texture,
		FrameGraphSRVRef input_surface_texture,
		FrameGraphTextureRef output_texture
	) {
		ScreenSpaceReflectionShader shader = ScreenSpaceReflectionShader(shader_manager.LoadRHIShader("SSR", RHI::ShaderType::COMPUTE), root_signature_manager);
		GPUComputePipelineStateBuilder pso_builder;
		pso_builder.SetRootSignature(shader.GetRootSignature());
		pso_builder.SetComputeShader(shader.GetComputeShader());
		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetComputePipelineState(pso_builder.ToRHIDescription());
		FrameGraphRenderPassIO io;
		io.AddShaderResource(input_depth_texture);
		io.AddShaderResource(input_normal_texture);
		io.AddShaderResource(input_surface_texture);
		io.AddShaderResource(input_scene_color_texture);
		FrameGraphUAVRef output_texture_uav = graph_builder.CreateUAV(output_texture, { .FirstSliceIndex = 0, .ArraySize = 1, .MipSlice = 0 });
		io.AddUnorderedAccessWrite(output_texture_uav);
		ScreenSpaceReflectionShader::Parameters* parameters = &graph_builder.AllocateShaderParameters<ScreenSpaceReflectionShader::Parameters>();
		ScreenSpaceReflectionShader::ComputeGroup* compute_group = graph_builder.AllocateShaderParameterGroup<ScreenSpaceReflectionShader::ComputeGroup>(shader.GetRootSignature(), 0, 1)[0];
		compute_group->constant_buffer = graph_builder.AllocateConstantBuffer<ScreenSpaceReflectionShader::ComputeGroup::Constants>();
		graph_builder.AddRenderPass(name, io, FrameGraphRenderPass::PIPELINE_TYPE::COMPUTE, [=, &graph_builder](RHI::CommandList command_list) {
			command_list->SetPipelineState(pso);
			command_list->SetComputeRootSignature(shader.GetRootSignature());
			parameters->Compute = compute_group;
			DirectX::XMStoreFloat4x4(&parameters->Compute->constants.InverseProjectionMatrix, render_data.InverseProjectionMatrix);
			DirectX::XMStoreFloat4x4(&parameters->Compute->constants.ProjectionMatrix, render_data.ProjectionMatrix);
			parameters->Compute->DepthTexture.srv = input_depth_texture->RHIHandle;
			parameters->Compute->NormalTexture.srv = input_normal_texture->RHIHandle;
			parameters->Compute->SceneColorTexture.srv = input_scene_color_texture->RHIHandle;
			parameters->Compute->SurfaceTexture.srv = input_surface_texture->RHIHandle;
			parameters->Compute->OutputTexture.uav = output_texture_uav->RHIHandle;
			parameters->Compute->constants.OutputResolution = { (float)width, (float)height };
			shader.SetParameters(command_list, parameters);
			command_list->Dispatch(std::ceil(width / 8.0f), std::ceil(height / 8.0f), 1);
		});
	}

	//Capute Scene as a CubeMap at Point.
	inline void CaptureScene(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		std::array<RenderData, 6>& render_datas,
		std::array <FrameGraphSRVRef, 5>& input_csms,
		std::array<FrameGraphSRVRef, 5>& input_osms,
		RHI::Texture noise_texture,
		const std::vector<DirectX::XMFLOAT4A>& kernel_data,
		FrameGraphSRVRef input_skybox,
		FrameGraphSRVRef input_sky_irradiance_map,
		FrameGraphTextureRef output_texture
	) {
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

		FrameGraphTextureRef base_color_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM_SRGB, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
		FrameGraphTextureRef normal_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_UNORM, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
		FrameGraphTextureRef surface_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8G8B8A8_UNORM, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
		RHI::TextureClearValue clear_value;
		clear_value.DepthAndStencil.Depth = 0.0f;
		clear_value.DepthAndStencil.Stencil = 0u;
		FrameGraphTextureRef depth_texture = graph_builder.CreateTexture({ .Format = RHI::Format::D32_FLOAT_S8X24_UINT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1, .Clear_Value = clear_value });
		FrameGraphTextureRef ssao_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R8_UNORM, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
		FrameGraphTextureRef scene_color_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
		FrameGraphTextureRef composite_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width = width, .Height = height, .ArraySize = 6, .MipLevels = 1 });
		for (uint32_t i = 0; i < 6u; ++i) {
			const std::string prefix = "Scene_Capture_Face[" + std::to_string(i) + "]_";
			render_datas[i] = render_data;
			render_datas[i].CameraPosition = DirectX::XMLoadFloat4A(&render_data.ReflectionProbe->Position);
			render_datas[i].ProjectionMatrix = projection_matrix;
			render_datas[i].ViewMatrix = view_matrices[i];
			render_datas[i].ViewProjectionMatrix = view_matrices[i] * projection_matrix;
			render_datas[i].InverseProjectionMatrix = DirectX::XMMatrixInverse(nullptr, projection_matrix);
			render_datas[i].InverseViewMatrix = DirectX::XMMatrixInverse(nullptr, view_matrices[i]);

			FrameGraphRTVRef scene_color_texture_rtv = graph_builder.CreateRTV(scene_color_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
			Skybox(prefix + "Skybox", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_datas[i], input_skybox, scene_color_texture_rtv);
			FrameGraphDSVRef depth_texture_dsv = graph_builder.CreateDSV(depth_texture, { .FirstSliceIndex = i, .ArraySize = 1 });
			DepthPrePass(prefix + "DepthPrepass", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_datas[i], depth_texture_dsv);
			FrameGraphRTVRef base_color_texture_rtv = graph_builder.CreateRTV(base_color_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
			FrameGraphRTVRef normal_texture_rtv = graph_builder.CreateRTV(normal_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
			FrameGraphRTVRef surface_texture_rtv = graph_builder.CreateRTV(surface_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
			GBuffer(prefix + "GBuffer", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_datas[i], depth_texture_dsv, base_color_texture_rtv, normal_texture_rtv, surface_texture_rtv);
			FrameGraphSRVRef base_color_texture_srv = graph_builder.CreateSRV(base_color_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
			FrameGraphSRVRef normal_texture_srv = graph_builder.CreateSRV(normal_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
			FrameGraphSRVRef surface_texture_srv = graph_builder.CreateSRV(surface_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
			FrameGraphSRVRef depth_texture_srv = graph_builder.CreateSRV(depth_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
			Deferred(prefix + "Deferred_Lighting", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_datas[i], base_color_texture_srv, normal_texture_srv, surface_texture_srv, depth_texture_srv, input_csms, input_osms, scene_color_texture_rtv);
			FrameGraphRTVRef ssao_texture_rtv = graph_builder.CreateRTV(ssao_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
			SSAO(prefix + "SSAO", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_datas[i], 4, noise_texture, kernel_data, depth_texture_srv, normal_texture_srv, ssao_texture_rtv);
			FrameGraphSRVRef scene_color_texture_srv = graph_builder.CreateSRV(scene_color_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1});
			FrameGraphSRVRef ssao_texture_srv = graph_builder.CreateSRV(ssao_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipLevels = 1 });
			FrameGraphUAVRef composite_texture_uav = graph_builder.CreateUAV(composite_texture, { .FirstSliceIndex = i, .ArraySize = 1, .MipSlice = 0 });
			DiffuseIndirectPass(prefix + "DiffuseIndirect", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, render_datas[i], width, height, scene_color_texture_srv, base_color_texture_srv, normal_texture_srv, surface_texture_srv, ssao_texture_srv, input_sky_irradiance_map, composite_texture_uav);
			GenerateMipMaps(prefix + "GenerateMipMaps", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, i, 8, composite_texture, output_texture);
		}
	}

	//Tone Mapping
	inline void ToneMapping(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphSRVRef input_texture,
		FrameGraphRTVRef output_texture
	) {
		FrameGraphRenderPassIO tone_mapping_io = {};
		tone_mapping_io.AddShaderResource(input_texture);
		tone_mapping_io.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);

		ToneMappingShader shader = ToneMappingShader(
			shader_manager.LoadRHIShader("ToneMapping", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("ToneMapping", RHI::ShaderType::PIXEL),
			root_signature_manager
		);

		GPUGraphicsPipelineStateBuilder pso_builder;
		pso_builder.SetRootSignature(shader.GetRootSignature())
			.SetVertexShader(shader.GetVertexShader())
			.SetPixelShader(shader.GetPixelShader())
			.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
			.SetRenderTargetFormats({ output_texture->TextureRef->Description.Format })
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

			ToneMappingShader::Parameters& parameters = graph_builder.AllocateShaderParameters<ToneMappingShader::Parameters>();
			parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::ToneMappingShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
			parameters.Pass->InputTexture.srv = tone_mapping_io.m_ShaderResources[0]->RHIHandle;

			shader.SetParameters(command_list, &parameters);

			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
			command_list->Draw(4, 1, 0, 0);
		});
	}

	inline void ReflectionProbe(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		std::array<RenderData, 6>& render_datas,
		std::array <FrameGraphSRVRef, 5>& input_csms,
		std::array<FrameGraphSRVRef, 5>& input_osms,
		RHI::Texture noise_texture,
		const std::vector<DirectX::XMFLOAT4A>& kernel_data,
		FrameGraphSRVRef input_skybox,
		FrameGraphSRVRef input_sky_irradiance_map,
		FrameGraphTextureRef output_texture
	) {
		FrameGraphTextureRef scene_capture_texture = graph_builder.CreateTexture({ .Format = RHI::Format::R16G16B16A16_FLOAT, .Width  = width, .Height = height, .ArraySize = 6, .MipLevels = 8 });
		CaptureScene(name + "_Capture_Scene", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_data, render_datas, input_csms, input_osms, noise_texture, kernel_data, input_skybox, input_sky_irradiance_map, scene_capture_texture);
		FrameGraphSRVRef scene_capture_texture_mipped_srv = graph_builder.CreateSRV(scene_capture_texture, { .FirstSliceIndex = 0, .ArraySize = 6, .MipLevels = 8 });
		PreFilterMap(name + "_PreFilterMap", graph_builder, shader_manager, pipeline_state_manager, root_signature_manager, width, height, render_data, scene_capture_texture_mipped_srv, output_texture);
	}

	inline void RayTraceTest(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef output_texture
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("PathTracer", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;
	
		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
			.RootDescriptorTable = RHI::RootDescriptorTable{
				.Ranges = {
					RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
					RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
					RHI::RootDescriptorTableRange{ .RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 1 } 
				}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 2;
		
		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);
		
		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessWrite(output_texture);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass("RTX", io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			command_list->DispatchRays(dispatch_ray_description);
		});
	}

	inline void ReSTIRGenerateSamples(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef output_texture
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_DI_GenerateInitialSamples", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 1 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessWrite(output_texture);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			command_list->DispatchRays(dispatch_ray_description);
		});
	}

	inline void ReSTIRTemporalSamples(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef previous_reservoir_buffer,
		AT::FrameGraphUAVRef reservoir_buffer,
		AT::FrameGraphUAVRef output_texture
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_DI_TemporalSampling", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 3 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessRead(reservoir_buffer);
		//io.AddUnorderedAccessRead(previous_resivoir_buffer);
		io.AddUnorderedAccessWrite(output_texture);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			device->WriteDescriptorTable(table, 9, 1, &previous_reservoir_buffer->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &reservoir_buffer->RHIHandle);
			command_list->DispatchRays(dispatch_ray_description);
			});
	}

	inline void ReSTIRSpatialSamples(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef reservoir_buffer,
		AT::FrameGraphUAVRef output_texture
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_DI_SpatialSamplingBetaTest", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 2 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessRead(reservoir_buffer);
		io.AddUnorderedAccessWrite(output_texture);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &reservoir_buffer->RHIHandle);
			command_list->DispatchRays(dispatch_ray_description);
		});
	}



	inline void ReSTIRFinalShade(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef save_resivoir_buffer,
		AT::FrameGraphUAVRef reservoir_buffer,
		AT::FrameGraphUAVRef output_texture
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_DI_FinalShading", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 3 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessRead(reservoir_buffer);
		io.AddUnorderedAccessWrite(output_texture);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &reservoir_buffer->RHIHandle);
			device->WriteDescriptorTable(table, 9, 1, &save_resivoir_buffer->RHIHandle);
			command_list->DispatchRays(dispatch_ray_description);
		});
	}



	//ReSTI GI
	inline void ReSTIR_GI_GenerateSamples(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef reservoir_output,
		AT::FrameGraphUAVRef reservoir_output1,
		AT::FrameGraphUAVRef reservoir_output2,
		AT::FrameGraphUAVRef reservoir_output3
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_GI_InitialSampling", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 4 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessWrite(reservoir_output);
		io.AddUnorderedAccessWrite(reservoir_output1);
		io.AddUnorderedAccessWrite(reservoir_output2);
		io.AddUnorderedAccessWrite(reservoir_output3);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &io.m_UnorderedAccessViewWrite[1]->RHIHandle);
			device->WriteDescriptorTable(table, 9, 1, &io.m_UnorderedAccessViewWrite[2]->RHIHandle);
			device->WriteDescriptorTable(table, 10, 1, &io.m_UnorderedAccessViewWrite[3]->RHIHandle);
			command_list->DispatchRays(dispatch_ray_description);
		});
	}


	inline void ReSTIR_GI_TemporalSampling(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef reservoir_output,
		AT::FrameGraphUAVRef reservoir_output1,
		AT::FrameGraphUAVRef reservoir_output2,
		AT::FrameGraphUAVRef reservoir_output3,

		AT::FrameGraphUAVRef current_reservoir_output,
		AT::FrameGraphUAVRef current_reservoir_output1,
		AT::FrameGraphUAVRef current_reservoir_output2,
		AT::FrameGraphUAVRef current_reservoir_output3,

		AT::FrameGraphUAVRef previous_reservoir_output,
		AT::FrameGraphUAVRef previous_reservoir_output1,
		AT::FrameGraphUAVRef previous_reservoir_output2,
		AT::FrameGraphUAVRef previous_reservoir_output3
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_GI_TemporalSampling", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 12 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);

		io.AddUnorderedAccessWrite(reservoir_output);
		io.AddUnorderedAccessWrite(reservoir_output1);
		io.AddUnorderedAccessWrite(reservoir_output2);
		io.AddUnorderedAccessWrite(reservoir_output3);

		io.AddUnorderedAccessRead(current_reservoir_output);
		io.AddUnorderedAccessRead(current_reservoir_output1);
		io.AddUnorderedAccessRead(current_reservoir_output2);
		io.AddUnorderedAccessRead(current_reservoir_output3);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &io.m_UnorderedAccessViewWrite[1]->RHIHandle);
			device->WriteDescriptorTable(table, 9, 1, &io.m_UnorderedAccessViewWrite[2]->RHIHandle);
			device->WriteDescriptorTable(table, 10, 1, &io.m_UnorderedAccessViewWrite[3]->RHIHandle);
			device->WriteDescriptorTable(table, 11, 1, &current_reservoir_output->RHIHandle);
			device->WriteDescriptorTable(table, 12, 1, &current_reservoir_output1->RHIHandle);
			device->WriteDescriptorTable(table, 13, 1, &current_reservoir_output2->RHIHandle);
			device->WriteDescriptorTable(table, 14, 1, &current_reservoir_output3->RHIHandle);
			
			device->WriteDescriptorTable(table, 15, 1, &previous_reservoir_output->RHIHandle);
			device->WriteDescriptorTable(table, 16, 1, &previous_reservoir_output1->RHIHandle);
			device->WriteDescriptorTable(table, 17, 1, &previous_reservoir_output2->RHIHandle);
			device->WriteDescriptorTable(table, 18, 1, &previous_reservoir_output3->RHIHandle);
			
			command_list->DispatchRays(dispatch_ray_description);
		});
	}

	inline void ReSTIR_GI_SpatialSampling(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,

		AT::FrameGraphUAVRef reservoir_output,
		AT::FrameGraphUAVRef reservoir_output1,
		AT::FrameGraphUAVRef reservoir_output2,
		AT::FrameGraphUAVRef reservoir_output3,

		AT::FrameGraphUAVRef save_reservoir_output,
		AT::FrameGraphUAVRef save_reservoir_output1,
		AT::FrameGraphUAVRef save_reservoir_output2,
		AT::FrameGraphUAVRef save_reservoir_output3
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_GI_SpatialSampling", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 8 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessWrite(reservoir_output);
		io.AddUnorderedAccessWrite(reservoir_output1);
		io.AddUnorderedAccessWrite(reservoir_output2);
		io.AddUnorderedAccessWrite(reservoir_output3);
		io.AddUnorderedAccessRead(save_reservoir_output);
		io.AddUnorderedAccessRead(save_reservoir_output1);
		io.AddUnorderedAccessRead(save_reservoir_output2);
		io.AddUnorderedAccessRead(save_reservoir_output3);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &reservoir_output->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &reservoir_output1->RHIHandle);
			device->WriteDescriptorTable(table, 9, 1, &reservoir_output2->RHIHandle);
			device->WriteDescriptorTable(table, 10, 1, &reservoir_output3->RHIHandle);

			device->WriteDescriptorTable(table, 11, 1, &save_reservoir_output->RHIHandle);
			device->WriteDescriptorTable(table, 12, 1, &save_reservoir_output1->RHIHandle);
			device->WriteDescriptorTable(table, 13, 1, &save_reservoir_output2->RHIHandle);
			device->WriteDescriptorTable(table, 14, 1, &save_reservoir_output3->RHIHandle);

			command_list->DispatchRays(dispatch_ray_description);
			});
	}


	inline void ReSTIR_GI_FinalShading(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		RHI::Buffer sbt_buffer,
		RHI::ShaderResourceView instance_info_buffer,
		AT::FrameGraphSRVRef base_color_texture,
		AT::FrameGraphSRVRef normal_texture,
		AT::FrameGraphSRVRef surface_texture,
		AT::FrameGraphSRVRef depth_texture,
		AT::FrameGraphUAVRef output,
			
		AT::FrameGraphUAVRef reservoir_output,
		AT::FrameGraphUAVRef reservoir_output1,
		AT::FrameGraphUAVRef reservoir_output2,
		AT::FrameGraphUAVRef reservoir_output3,

		AT::FrameGraphUAVRef save_reservoir_output,
		AT::FrameGraphUAVRef save_reservoir_output1,
		AT::FrameGraphUAVRef save_reservoir_output2,
		AT::FrameGraphUAVRef save_reservoir_output3
	) {
		RHI::Shader shader = shader_manager.LoadRHIShader("ReSTIR_GI_FinalShading", RHI::ShaderType::LIBRARY);
		RHI::RayTracingShaderLibrary shader_library;
		shader_library.SetLibrary(shader);
		shader_library.DefineExport("RayGen");
		shader_library.DefineExport("ClosestHit");
		shader_library.DefineExport("Miss");

		RHI::RayTracingHitGroup hit_group;
		hit_group.Name = "HitGroup";
		hit_group.Type = RHI::RayTracingHitGroupType::TRIANGLES;
		hit_group.AnyHitShaderImport;
		hit_group.ClosestHitShaderImport = "ClosestHit";
		hit_group.IntersectionShaderImport;

		RHI::RayTracingExportAssociation export_association1;
		export_association1.Type = RHI::RayTracingExportAssociationType::SHADER_PAYLOAD;
		export_association1.ExportNames.push_back("HitGroup");
		export_association1.AssociationObjectIndex = 0;

		RHI::RootSignatureDescription root_signature_description;
		root_signature_description.Parameters.push_back(RHI::RootParameter{
			.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE,
				.RootDescriptorTable = RHI::RootDescriptorTable{
					.Ranges = {
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW, .DescriptorCount = 1 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE, .DescriptorCount = 6 },
						RHI::RootDescriptorTableRange{.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE, .DescriptorCount = 9 }
					}
			}
		});
		root_signature_description.StaticSamplers.push_back({});

		RHI::RootSignature global_root_signature = root_signature_manager.CreateOrGetRootSignature(root_signature_description);



		RHI::RayTracingPipelineStateDescription pipeline_description;
		pipeline_description.ShaderLibraries.emplace_back(shader_library);
		pipeline_description.HitGroups.emplace_back(hit_group);
		pipeline_description.GlobalRootSignature = global_root_signature;
		pipeline_description.LocalRootSignatures;
		pipeline_description.ExportAssociations.emplace_back(export_association1);
		pipeline_description.ShaderConfiguration.MaxAttributeSizeInBytes = 2 * sizeof(float);
		pipeline_description.ShaderConfiguration.MaxPayloadSizeInBytes = 27 * sizeof(float);
		pipeline_description.MaxTraceRecursionDepth = 3;

		RHI::IRayTracingPipeline* pso = pipeline_state_manager.CreateOrGetRayTracingPipelineState(pipeline_description);


		sbt_buffer->Map();
		void* RayGen = pso->GetShaderIdentifier("RayGen");
		sbt_buffer->CopyData(0, RayGen, 32);

		void* Miss = pso->GetShaderIdentifier("Miss");
		sbt_buffer->CopyData(64, Miss, 32);

		void* HitGroup = pso->GetShaderIdentifier("HitGroup");
		sbt_buffer->CopyData(128, HitGroup, 32);

		AT::FrameGraphRenderPassIO io;
		io.AddShaderResource(base_color_texture);
		io.AddShaderResource(normal_texture);
		io.AddShaderResource(surface_texture);
		io.AddShaderResource(depth_texture);
		io.AddUnorderedAccessWrite(output);
		io.AddUnorderedAccessRead(reservoir_output);
		io.AddUnorderedAccessRead(reservoir_output1);
		io.AddUnorderedAccessRead(reservoir_output2);
		io.AddUnorderedAccessRead(reservoir_output3);

		RHI::DescriptorTable table;
		RHI::Device device = graph_builder.GetDevice();
		graph_builder.AllocateDescriptorTable(global_root_signature, 0, table);

		struct GlobalConstants {
			DirectX::XMFLOAT4X4 InverseViewProjectionMatrix;
			DirectX::XMFLOAT4X4 PreviousViewProjectionMatrix;
			DirectX::XMFLOAT4 CameraPosition;
			uint32_t FrameNumber;
		} constants;

		DirectX::XMStoreFloat4x4(&constants.InverseViewProjectionMatrix, render_data.InverseViewProjectionMatrix);
		DirectX::XMStoreFloat4x4(&constants.PreviousViewProjectionMatrix, render_data.PreviousViewProjectionMatrix);
		DirectX::XMStoreFloat4(&constants.CameraPosition, render_data.CameraPosition);
		constants.FrameNumber = render_data.FrameNumber;
		GPUConstantBuffer* cbuffer = graph_builder.AllocateConstantBuffer<GlobalConstants>();
		cbuffer->WriteData(constants);

		//Upload Materials.

		graph_builder.AddRenderPass(name, io, AT::FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=](RHI::CommandList command_list) {
			command_list->SetComputeRootSignature(global_root_signature);
			command_list->SetRayTracingPipeline(pso);
			command_list->SetComputeRootDescriptorTable(0, table);
			RHI::RayTracingDispatchRaysDescription dispatch_ray_description;
			dispatch_ray_description.Width = 1280;
			dispatch_ray_description.Height = 720;
			dispatch_ray_description.Depth = 1;
			dispatch_ray_description.RayGenerationShaderRecord = { .Buffer = sbt_buffer, .Offset = 0, .Size = 32 };
			dispatch_ray_description.MissShaderTable = { .Buffer = sbt_buffer, .Offset = 64, .Size = 32, .Stride = 32 };
			dispatch_ray_description.HitGroupTable = { .Buffer = sbt_buffer, .Offset = 128, .Size = 32, .Stride = 32 };
			device->WriteDescriptorTable(table, 0, 1, &cbuffer->GetNative());
			device->WriteDescriptorTable(table, 1, 1, &scene_srv);
			device->WriteDescriptorTable(table, 2, 1, &base_color_texture->RHIHandle);
			device->WriteDescriptorTable(table, 3, 1, &normal_texture->RHIHandle);
			device->WriteDescriptorTable(table, 4, 1, &surface_texture->RHIHandle);
			device->WriteDescriptorTable(table, 5, 1, &depth_texture->RHIHandle);
			device->WriteDescriptorTable(table, 6, 1, &instance_info_buffer);
			device->WriteDescriptorTable(table, 7, 1, &io.m_UnorderedAccessViewWrite[0]->RHIHandle);
			device->WriteDescriptorTable(table, 8, 1, &reservoir_output->RHIHandle);
			device->WriteDescriptorTable(table, 9, 1, &reservoir_output1->RHIHandle);
			device->WriteDescriptorTable(table, 10, 1, &reservoir_output2->RHIHandle);
			device->WriteDescriptorTable(table, 11, 1, &reservoir_output3->RHIHandle);

			device->WriteDescriptorTable(table, 12, 1, &save_reservoir_output->RHIHandle);
			device->WriteDescriptorTable(table, 13, 1, &save_reservoir_output1->RHIHandle);
			device->WriteDescriptorTable(table, 14, 1, &save_reservoir_output2->RHIHandle);
			device->WriteDescriptorTable(table, 15, 1, &save_reservoir_output3->RHIHandle);

			command_list->DispatchRays(dispatch_ray_description);
		});
	}


	//ReSTIR Composite
	inline void ReSTIRComposite(
		const std::string& name,
		AT::FrameGraphBuilder& graph_builder,
		AT::GPUShaderManager& shader_manager,
		AT::GPUPipelineStateManager& pipeline_state_manager,
		AT::GPURootSignatureManager& root_signature_manager,
		uint32_t width,
		uint32_t height,
		RenderData& render_data,
		FrameGraphSRVRef direct_illumination,
		FrameGraphSRVRef global_illumination,
		FrameGraphRTVRef output_texture
	) {
		FrameGraphRenderPassIO tone_mapping_io = {};
		tone_mapping_io.AddShaderResource(direct_illumination);
		tone_mapping_io.AddShaderResource(global_illumination);
		tone_mapping_io.AddRenderTarget(output_texture, RHI::RenderPassAttachment::LoadOperation::CLEAR);

		ReSTIRCompositeShader shader = ReSTIRCompositeShader(
			shader_manager.LoadRHIShader("ReSTIRComposite", RHI::ShaderType::VERTEX),
			shader_manager.LoadRHIShader("ReSTIRComposite", RHI::ShaderType::PIXEL),
			root_signature_manager
		);

		GPUGraphicsPipelineStateBuilder pso_builder;
		pso_builder.SetRootSignature(shader.GetRootSignature())
			.SetVertexShader(shader.GetVertexShader())
			.SetPixelShader(shader.GetPixelShader())
			.SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP)
			.SetRenderTargetFormats({ output_texture->TextureRef->Description.Format })
			.SetDepthEnabled(false)
			.SetCullMode(RHI::CullMode::BACK)
			.SetFrontCounterClockwise(true)
			.SetFillMode(RHI::FillMode::SOLID);

		RHI::PipelineState pso = pipeline_state_manager.CreateOrGetGraphicsPipelineState(pso_builder.ToRHIDescription());

		graph_builder.AddRenderPass("ReSTIRComposite", tone_mapping_io, FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS, [=, &graph_builder](RHI::CommandList command_list) {
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

			ReSTIRCompositeShader::Parameters& parameters = graph_builder.AllocateShaderParameters<ReSTIRCompositeShader::Parameters>();
			parameters.Pass = graph_builder.AllocateShaderParameterGroup<AT::ReSTIRCompositeShader::PassGroup>(shader.GetRootSignature(), 0, 1)[0];
			parameters.Pass->DirectIllumination.srv = tone_mapping_io.m_ShaderResources[0]->RHIHandle;
			parameters.Pass->GlobalIllumination.srv = tone_mapping_io.m_ShaderResources[1]->RHIHandle;

			shader.SetParameters(command_list, &parameters);

			command_list->SetPrimitiveTopology(RHI::PrimitiveTopology::TRIANGLE_STRIP);
			command_list->Draw(4, 1, 0, 0);
		});
	}
}
#endif