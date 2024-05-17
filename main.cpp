#include "Platform/Platform.h"
#include "Systems/JobSystem/JobSystem.h"
#include "Systems/RenderSystem/RenderSystem.h"
#include "ResourceManagers/SceneManager.h"
#include "Systems/RenderSystem/GPUResourceManager.h"
#include "Systems/RenderSystem/Renderers/DeferredRenderer.h"

#include <cmath>
#include <chrono>

void Simulation(AT::FrameParameters& frame_parameters, AT::SceneRef scene) {
	const float speed = 2.0f;
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
	scene->Camera.Rotation = DirectX::XMVectorAdd(scene->Camera.Rotation, rotation_delta);
	AT::Input::Reset();
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

	AT::RenderSystem render_system = AT::RenderSystem(window, RHI::RenderAPI::VULKAN);
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
	scene->Camera.Position = DirectX::XMVectorSet(-7.2f, 1.0f, 0.0f, 1.0f);
	scene->Camera.Rotation = DirectX::XMVectorSet(0.0f, -3.1415f / 2.0f, 0.0f, 0.0f);

	render_system.RegisterRenderer("Deferred", new AT::DeferredRenderer(resource_manager));
	render_system.SetRenderer("Deferred");
	while (!quit) {
		AT::WindowEvent ev;
		window->PollEvent(ev);
		if (AT::Input::GetEvent(AT::Input::INPUT_EVENT_QUIT)) {
			quit = true;
			AT::JobSystem::Stop();
			break;
		}
		//Single Threaded.
		AT::FrameParameters parameters;
		parameters.FrameNumber = frame_counter;
		parameters.DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - frame_end).count() / 1000.0f;
		Simulation(parameters, scene);
		frame_end = std::chrono::high_resolution_clock::now();
		render_system.RunRenderLogic(parameters, scene);
		render_system.RunGPUExecution(parameters);
		++frame_counter;
	}
	render_system.Wait();
	delete window;
	return 0;
}