#include "Platform/Platform.h"
#include "Systems/JobSystem/JobSystem.h"
#include "Systems/RenderSystem/RenderSystem.h"
#include "ResourceManagers/SceneManager.h"
#include "Systems/RenderSystem/GPUResourceManager.h"
#include "Systems/RenderSystem/Renderers/DeferredRenderer.h"
#include "Systems/RenderSystem/Renderers/PathTracer.h"

#include <cmath>
#include <chrono>

static float g_time = 0.0f;

void Simulation(AT::FrameParameters& frame_parameters, AT::SceneRef scene) {
	const float speed = 1.0f;
	float move_unit = speed * frame_parameters.DeltaTime;
	DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionRotationRollPitchYawFromVector(scene->Camera.Rotation));
	if (AT::Input::GetKeyDown('W')) {
		scene->Camera.Position = DirectX::XMVectorAdd(scene->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, -move_unit, 0.0f), rotation_matrix));
	}
	if (AT::Input::GetKeyDown('A')) {
		scene->Camera.Position = DirectX::XMVectorAdd(scene->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(-move_unit, 0.0f, 0.0, 0.0f), rotation_matrix));
	}
	if (AT::Input::GetKeyDown('S')) {
		scene->Camera.Position = DirectX::XMVectorAdd(scene->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, move_unit, 0.0f), rotation_matrix));
	}
	if (AT::Input::GetKeyDown('D')) {
		scene->Camera.Position = DirectX::XMVectorAdd(scene->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(move_unit, 0.0f, 0.0, 0.0f), rotation_matrix));
	}
	if (AT::Input::GetKeyDown(' ')) {
		scene->Camera.Position = DirectX::XMVectorAdd(scene->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0, move_unit, 0.0, 0.0f), rotation_matrix));
	}
	if (AT::Input::GetKeyDown('B')) {
		scene->Camera.Position = DirectX::XMVectorAdd(scene->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0, -move_unit, 0.0, 0.0f), rotation_matrix));
	}

	AT::Input::MouseState state = AT::Input::GetMouseState();
	DirectX::XMVECTOR rotation_delta = DirectX::XMVectorScale(DirectX::XMVector2Normalize(DirectX::XMVectorSet(state.delta_y, state.delta_x, 0.0f, 0.0f)), -7.5f / 2.0f / 3.1416f * frame_parameters.DeltaTime);
	//scene->Camera.Rotation = DirectX::XMVectorAdd(scene->Camera.Rotation, rotation_delta);
	AT::Input::Reset();
	scene->m_Lights[1].PositionOrDirection = { -9.0f * cos(g_time), 1.0f, 0.0f, 1.0f};
	g_time += frame_parameters.DeltaTime  / 2.0f;
	if (g_time >= 2.0f * 3.14156f) {
		g_time = 0.0f;
	}
}

HINSTANCE app_instance;int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, PSTR command_line, int show_command) {
	AT::JobSystem::Initialize();
	app_instance = instance;
	AT::WindowDescription window_description = {};
	window_description.Title = "ATEngine";
	window_description.Width = 1280;
	window_description.Height = 720;
	AT::IWindow* window;
	AT::Platform::CreatePlatformWindow(window_description, window);

	AT::RenderSystem render_system = AT::RenderSystem(window, RHI::RenderAPI::DIRECTX);
	AT::GPUResourceManager& resource_manager = render_system.GetResourceManager();
	AT::MeshManager mesh_manager = AT::MeshManager(resource_manager);
	AT::MaterialManager material_manager = AT::MaterialManager(render_system.GetDevice(), resource_manager);
	AT::SceneManager scene_manager = AT::SceneManager(mesh_manager, material_manager);
	AT::SceneRef scene = scene_manager.LoadScene("./Assets/Sponza/Sponza.gltf");
	mesh_manager.UploadMeshes();
	material_manager.UploadMaterial();

	bool quit = false;
	uint64_t frame_counter = 0;
	std::chrono::milliseconds time = {};
	
	std::chrono::steady_clock::time_point frame_end = {};
	scene->Camera.VerticalFOV = 0.3f * 3.14159f;
	scene->Camera.Position = DirectX::XMVectorSet(9.0f, 1.0f, 0.0f, 1.0f);
	scene->Camera.Rotation = DirectX::XMVectorSet(0.0f, 3.1415f / 2.0f, 0.0f, 0.0f);

	render_system.RegisterRenderer("Deferred", new AT::DeferredRenderer(resource_manager));
	/*render_system.RegisterRenderer("PathTracer", new AT::PathTracer(resource_manager));
	render_system.SetRenderer("PathTracer");*/

	RHI::CommandAllocator cmd_alloc;
	RHI::CommandList cmd_list;
	RHI::Fence fence;
	resource_manager.GetDevice()->CreateCommandAllocator(RHI::CommandType::COMPUTE, cmd_alloc);
	resource_manager.GetDevice()->CreateCommandList(RHI::CommandType::COMPUTE, cmd_alloc, cmd_list);
	resource_manager.GetDevice()->CreateFence(0, fence);
	cmd_alloc->Reset();
	cmd_list->Reset(RHI_NULL_HANDLE);

	AT::RenderData render_data;
	scene->GenerateRenderData(render_data);
	std::vector<RHI::IRayTracingBottomLevelAccelerationStructure*> blases;

	RHI::DescriptorTable scene_buffer_table;
	{
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_BUFFER;
		range.DescriptorCount = render_data.Meshes.size() * 2;
		range.IsArray = false;
		range.RegisterSpace = 0;
		range.ShaderRegister = 0;
		RHI::RootParameter root_parameter;
		root_parameter.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE;
		root_parameter.ShaderVisibility = RHI::ShaderVisibility::ALL;
		root_parameter.RootDescriptorTable.Ranges.push_back(range);
		RHI::RootSignatureDescription rs_description;
		rs_description.Parameters.emplace_back(root_parameter);
		RHI::RootSignature root_signature;
		render_system.GetDevice()->CreateRootSignature(rs_description, root_signature);
		render_system.GetDevice()->AllocateDescriptorTable(root_signature, 0, scene_buffer_table);
		delete root_signature;
	}
	for (uint32_t i = 0; i < render_data.Meshes.size(); ++i) {
		RHI::RayTracingGeometryDescription geometry;
		geometry.VertexBuffer.Buffer = render_data.Meshes[i]->VertexBuffer->GetRHIHandle();
		geometry.VertexBuffer.Format = RHI::Format::R32G32B32_FLOAT;
		geometry.VertexBuffer.Stride = sizeof(AT::VertexFormat::Vertex);
		geometry.VertexBuffer.VertexCount = render_data.Meshes[i]->Vertices.size();
		geometry.IndexBuffer.Buffer = render_data.Meshes[i]->IndexBuffer->GetRHIHandle();
		geometry.IndexBuffer.Format = RHI::Format::R32_UINT;
		geometry.IndexBuffer.IndexCount = render_data.Meshes[i]->IndicesCount;

		//Create BLAS
		RHI::RayTracingBottomLevelAccelerationStructureDescription blas_description;
		blas_description.Geometries.emplace_back(geometry);
		RHI::RayTracingAccelerationStructureMemoryInfo blas_memory_info;
		render_system.GetDevice()->GetRayTracingBottomLevelAccelerationStructureMemoryInfo(blas_description, blas_memory_info);
		RHI::Buffer scratch_buffer, destination_buffer;
		{
			RHI::BufferDescription buffer_description;
			buffer_description.Size = blas_memory_info.ScratchDataSize;
			buffer_description.UsageFlags = RHI::BufferUsageFlag::UNORDERED_ACCESS;
			scratch_buffer = resource_manager.CreateBuffer(buffer_description)->GetRHIHandle();
		}
		{
			RHI::BufferDescription buffer_description;
			buffer_description.Size = blas_memory_info.DestinationDataSize;
			buffer_description.UsageFlags = RHI::BufferUsageFlag::UNORDERED_ACCESS;
			destination_buffer = resource_manager.CreateBuffer(buffer_description, RHI::BufferState::RAYTRACING_ACCELERATION_STRUCTURE)->GetRHIHandle();
		}
		RHI::IRayTracingBottomLevelAccelerationStructure* scratch_blas, * blas;
		resource_manager.GetDevice()->CreateRayTracingBottomLevelAccelerationStructure(scratch_buffer, scratch_blas);
		resource_manager.GetDevice()->CreateRayTracingBottomLevelAccelerationStructure(destination_buffer, blas);
		RHI::BuildRayTracingBottomLevelAccelerationStructure blas_build_info;
		blas_build_info.description = &blas_description;
		blas_build_info.ScratchBottomLevelAccelerationStructure = scratch_blas;
		blas_build_info.DestinationBottomLevelAccelerationStructure = blas;
		blases.push_back(blas);
		cmd_list->BuildRaytracingBottomLevelAccelerationStructure(blas_build_info);
		
		//Upload Vertex and Index Buffer.
		uint32_t table_index = i * 2;
		{
			RHI::ShaderResourceViewDescription srv_description;
			srv_description.Format = RHI::Format::UNKNOWN;
			srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::BUFFER;
			srv_description.Buffer.FirstElement = 0;
			srv_description.Buffer.NumberOfElements = render_data.Meshes[i]->Vertices.size();
			srv_description.Buffer.ElementStride = sizeof(AT::VertexFormat::Vertex);

			RHI::ShaderResourceView srv;
			render_system.GetDevice()->CreateShaderResourceView(render_data.Meshes[i]->VertexBuffer->GetRHIHandle(), srv_description, srv);
			render_system.GetDevice()->WriteDescriptorTable(scene_buffer_table, table_index, 1, &srv);
		}
		{
			RHI::ShaderResourceViewDescription srv_description;
			srv_description.Format = RHI::Format::UNKNOWN;
			srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::BUFFER;
			srv_description.Buffer.FirstElement = 0;
			srv_description.Buffer.NumberOfElements = render_data.Meshes[i]->Indices.size();
			srv_description.Buffer.ElementStride = sizeof(AT::VertexFormat::Index);

			RHI::ShaderResourceView srv;
			render_system.GetDevice()->CreateShaderResourceView(render_data.Meshes[i]->IndexBuffer->GetRHIHandle(), srv_description, srv);
			render_system.GetDevice()->WriteDescriptorTable(scene_buffer_table, table_index + 1, 1, &srv);
		}
		

	}

	cmd_list->Close();
	resource_manager.GetDevice()->ExecuteCommandList(RHI::CommandType::COMPUTE, 1, &cmd_list);
	resource_manager.GetDevice()->SignalQueue(RHI::CommandType::COMPUTE, fence, 1);

	RHI::BufferDescription buffer_description;
	buffer_description.Size = render_data.RenderObjects.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
	buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
	AT::GPUBufferPtr instance_desc_buffer = resource_manager.CreateUploadBuffer(buffer_description);
	instance_desc_buffer->GetRHIHandle()->Map();
	RHI::IRayTracingInstanceBuffer* instance_buffer;
	resource_manager.GetDevice()->CreateRayTracingInstanceBuffer(instance_desc_buffer->GetRHIHandle(), 1, instance_buffer);

	struct InstanceInfo {
		uint32_t VertexBufferIndex;
		uint32_t IndexBufferIndex;
		uint32_t MaterialIndex;
	};
	RHI::BufferDescription b_description;
	b_description.Size = sizeof(InstanceInfo) * render_data.RenderObjects.size();
	b_description.UsageFlags = RHI::BufferUsageFlag::NONE;
	AT::GPUBufferPtr instance_info_buffer = resource_manager.CreateUploadBuffer(b_description);
	instance_info_buffer->GetRHIHandle()->Map();

	//RT Materials.
	RHI::DescriptorTable material_table;
	{
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE;
		range.DescriptorCount = material_manager.m_Materials.size() * 3;
		RHI::RootParameter root_parameter;
		root_parameter.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE;
		root_parameter.RootDescriptorTable.Ranges.emplace_back(range);
		root_parameter.ShaderVisibility = RHI::ShaderVisibility::ALL;
		RHI::RootSignatureDescription rs_description;
		rs_description.Parameters = { root_parameter };
		RHI::RootSignature rs;
		render_system.GetDevice()->CreateRootSignature(rs_description, rs);
		render_system.GetDevice()->AllocateDescriptorTable(rs, 0, material_table);
	}

	for (uint32_t i = 0u; i < material_manager.m_Materials.size(); ++i) {
		uint32_t index = i * 3;
		render_system.GetDevice()->WriteDescriptorTable(material_table, index, 1, &material_manager.m_Materials[i]->BaseColorSRV);
		render_system.GetDevice()->WriteDescriptorTable(material_table, index + 1, 1, &material_manager.m_Materials[i]->NormalSRV);
		render_system.GetDevice()->WriteDescriptorTable(material_table, index + 2, 1, &material_manager.m_Materials[i]->RoughnessMetalnessSRV);
	}

	uint32_t material_table_offset = static_cast<RHI::DX12::DX12DescriptorTable*>(material_table)->m_AllocationInfo.offset;

	for (uint32_t i = 0; i < render_data.RenderObjects.size(); ++i) {
		RHI::RayTracingInstance instance_desc;
		for (uint32_t j = 0; j < 3; ++j) {
			for (uint32_t k = 0; k < 4; ++k) {
				instance_desc.Transform[j][k] = render_data.RenderObjects[i].Transform.m[j][k];
			}
		}
		instance_desc.InstanceID = 0;
		instance_desc.InstanceContributionToHitGroupIndex = 0;
		instance_desc.InstanceMask = 0xFF;
		instance_desc.BottomLevelAccelerationStructure = blases[render_data.RenderObjects[i].MeshID];
		instance_buffer->WriteInstance(i, instance_desc);
		//Write Geometry Info.
		InstanceInfo instance_info;
		uint32_t buffer_index = static_cast<RHI::DX12::DX12DescriptorTable*>(scene_buffer_table)->m_AllocationInfo.offset + (render_data.RenderObjects[i].MeshID * 2);
		instance_info.VertexBufferIndex = buffer_index;
		instance_info.IndexBufferIndex = buffer_index + 1;
		instance_info.MaterialIndex = material_table_offset + (render_data.Meshes[render_data.RenderObjects[i].MeshID]->MaterialID * 3);
		instance_info_buffer->GetRHIHandle()->CopyData(i * sizeof(InstanceInfo), &instance_info, sizeof(InstanceInfo));
	}

	RHI::ShaderResourceView instance_info_srv;
	{
		RHI::ShaderResourceViewDescription srv_description;
		srv_description.Format = RHI::Format::UNKNOWN;
		srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::BUFFER;
		srv_description.Buffer.FirstElement = 0;
		srv_description.Buffer.NumberOfElements = render_data.RenderObjects.size();
		srv_description.Buffer.ElementStride = sizeof(InstanceInfo);
		render_system.GetDevice()->CreateShaderResourceView(instance_info_buffer->GetRHIHandle(), srv_description, instance_info_srv);
	}

	//Build TLAS.
	RHI::RayTracingTopLevelAccelerationStructureDescription tlas_description;
	tlas_description.InstancesBuffer = instance_buffer;
	tlas_description.InstanceCount = render_data.RenderObjects.size();
	RHI::RayTracingAccelerationStructureMemoryInfo tlas_memory_info;
	resource_manager.GetDevice()->GetRayTracingTopLevelAccelerationStructureMemoryInfo(tlas_description, tlas_memory_info);
	RHI::Buffer scratch_tlas_buffer, destination_tlas_buffer;
	{
		RHI::BufferDescription buffer_description;
		buffer_description.Size = tlas_memory_info.ScratchDataSize;
		buffer_description.UsageFlags = RHI::BufferUsageFlag::UNORDERED_ACCESS;
		scratch_tlas_buffer = resource_manager.CreateBuffer(buffer_description)->GetRHIHandle();
	}
	{
		RHI::BufferDescription buffer_description;
		buffer_description.Size = tlas_memory_info.DestinationDataSize;
		buffer_description.UsageFlags = RHI::BufferUsageFlag::UNORDERED_ACCESS;
		destination_tlas_buffer = resource_manager.CreateBuffer(buffer_description, RHI::BufferState::RAYTRACING_ACCELERATION_STRUCTURE)->GetRHIHandle();
	}
	RHI::IRayTracingTopLevelAccelerationStructure* scratch_tlas, * tlas;
	resource_manager.GetDevice()->CreateRayTracingTopLevelAccelerationStructure(scratch_tlas_buffer, scratch_tlas);
	resource_manager.GetDevice()->CreateRayTracingTopLevelAccelerationStructure(destination_tlas_buffer, tlas);
	RHI::BuildRayTracingTopLevelAccelerationStructure tlas_build_info;
	tlas_build_info.description = &tlas_description;
	tlas_build_info.ScratchTopLevelAccelerationStructure = scratch_tlas;
	tlas_build_info.DestinationTopLevelAccelerationStructure = tlas;

	resource_manager.GetDevice()->HostWait(fence, 1);
	cmd_alloc->Reset();
	cmd_list->Reset(RHI_NULL_HANDLE);
	cmd_list->BuildRaytracingTopLevelAccelerationStructure(tlas_build_info);
	cmd_list->Close();
	resource_manager.GetDevice()->ExecuteCommandList(RHI::CommandType::COMPUTE, 1, &cmd_list);
	resource_manager.GetDevice()->SignalQueue(RHI::CommandType::COMPUTE, fence, 2);
	resource_manager.GetDevice()->HostWait(fence, 2);

	scene_srv = tlas;

	render_system.RegisterRenderer("PathTracer", new AT::PathTracer(resource_manager, instance_info_srv));
	render_system.SetRenderer("PathTracer");
	//render_system.SetRenderer("Deferred");

	while (!quit) {
		AT::WindowEvent ev;
		window->PollEvent(ev);
		if (AT::Input::GetEvent(AT::Input::INPUT_EVENT_QUIT)) {
			quit = true;
			AT::JobSystem::Stop();
			break;
		}
		//Single Threaded Loop.
		AT::FrameParameters parameters;
		parameters.FrameNumber = frame_counter;
		parameters.RenderData.FrameNumber = frame_counter - (frame_counter % 2);

		parameters.RenderData.PreviousViewProjectionMatrix = scene->m_PreviousViewProjectionMatrix;
		render_data.PreviousViewProjectionMatrix = scene->m_PreviousViewProjectionMatrix;

		parameters.DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - frame_end).count() / 1000.0f;
		Simulation(parameters, scene);
		frame_end = std::chrono::high_resolution_clock::now();
		render_system.RunRenderLogic(parameters, scene);
		render_system.RunGPUExecution(parameters);
		++frame_counter;

		scene->m_PreviousViewProjectionMatrix = render_data.ViewProjectionMatrix;
		scene->m_PreviousViewProjectionMatrix = parameters.RenderData.ViewProjectionMatrix;
	}
	render_system.Wait();
	delete window;
	return 0;
}