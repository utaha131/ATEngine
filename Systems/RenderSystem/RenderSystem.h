#ifndef _AT_RENDER_SYSTEM_H_
#define _AT_RENDER_SYSTEM_H_
#ifdef _DEBUG
#define DEBUG true
#else
#define DEBUG false
#endif
#define FRAME_COUNT 2
#include "../RHI/RHI.h"
#include "../Platform/IWindow.h"
#include "GPUResourceManager.h"
#include "GPUShaderManager.h"
#include "GPURootSignatureManager.h"
#include "GPUPipelineStateManager.h"
#include "../Scene/Scene.h"
#include "Renderers/Renderer.h"
#include <memory>

namespace AT {
	class RenderSystem {
	public:
		RenderSystem(IWindow* window, RHI::RenderAPI api) {
			RHI::CreateRenderBackend(api, DEBUG, m_RenderBackend);
			std::vector<RHI::Adapter> v_adapters = std::vector<RHI::Adapter>();
			RHI::Adapter selected_adapter = RHI_NULL_HANDLE;
			m_RenderBackend->GetAdapters(v_adapters);
			for (uint32_t i = 0; i < v_adapters.size(); ++i) {
				if (v_adapters[i]->GetVendor() == RHI::Vendor::NVIDIA) {
					selected_adapter = v_adapters[i];
				}
			}
			m_RenderBackend->CreateDevice(selected_adapter, m_Device);

			m_ShaderManager = new GPUShaderManager(m_Device, api);
			m_PipelineStateManager = new GPUPipelineStateManager(m_Device);
			m_RootSignatureManager = new GPURootSignatureManager(m_Device);
			m_ResourceManager = new GPUResourceManager(m_Device, *m_ShaderManager, *m_RootSignatureManager);

			RHI::SwapChainDescription swap_chain_description = {};
			swap_chain_description.BufferCount = FRAME_COUNT;
			swap_chain_description.BufferFormat = RHI::Format::B8G8R8A8_UNORM;// RHI_FORMAT_R8G8B8A8_UNORM;
			swap_chain_description.Width = window->GetWidth();
			swap_chain_description.Height = window->GetHeight();
			swap_chain_description.Window = window->GetNative();
			m_RenderBackend->CreateSwapChain(m_Device, swap_chain_description, m_SwapChain);


			m_Device->CreateFence(0, m_Fence);

			for (uint32_t i = 0; i < FRAME_COUNT; ++i) {
				m_Device->CreateCommandAllocator(RHI::CommandType::DIRECT, m_FrameResources[i].command_allocator);
				m_Device->CreateCommandList(RHI::CommandType::DIRECT, m_FrameResources[i].command_allocator, m_FrameResources[i].command_list);
				m_FrameResources[i].graph_builder = new AT::FrameGraphBuilder(m_Device, m_FrameResources[i].command_list);
			}
		}

		void UploadScene(Scene& scene) {

		}

		GPUResourceManager& GetResourceManager() {
			return *m_ResourceManager;
		}

		GPUShaderManager& GetShaderManager() {
			return *m_ShaderManager;
		}
		
		GPUPipelineStateManager& GetPipelineStateManager() {
			return *m_PipelineStateManager;
		}

		GPURootSignatureManager& GetRootSignatureManager() {
			return *m_RootSignatureManager;
		}

		RHI::Device GetDevice() const {
			return m_Device;
		}

		~RenderSystem() {
			for (uint32_t i = 0u; i < FRAME_COUNT; ++i) {
				delete m_FrameResources[i].command_list;
				delete m_FrameResources[i].command_allocator;
				delete m_FrameResources[i].graph_builder;
			}

			delete m_PipelineStateManager;
			delete m_RootSignatureManager;
			delete m_ShaderManager;
			delete m_ResourceManager;

			delete m_Fence;
			delete m_SwapChain;
			delete m_Device;
			delete m_RenderBackend;
		}

		Renderer* RegisterRenderer(const std::string& name, Renderer* renderer) {
			if (m_RendererMap.find(name) == m_RendererMap.end()) {
				m_RendererMap[name] = renderer;
			}
			return m_RendererMap[name];
		}

		void SetRenderer(const std::string& name) {
			m_CurrentRenderer = m_RendererMap[name];
		}

		Renderer* GetRenderer(const std::string& name) {
			return m_RendererMap[name];
		}

		void Wait() {
			m_Device->HostWait(m_Fence, m_CurrentFenceValue);
		}

		void RunRenderLogic(FrameParameters& frame_parameters, Scene* scene) {
			m_CurrentRenderer->RenderLogic(frame_parameters, scene);
		}

		void RunGPUExecution(FrameParameters& frame_parameters) {
			uint64_t fence_count = 0;
			if (frame_parameters.FrameNumber > FRAME_COUNT - 1) {
				fence_count = frame_parameters.FrameNumber - FRAME_COUNT + 1;
			}
			m_Device->HostWait(m_Fence, fence_count);
			uint64_t index = frame_parameters.FrameNumber % FRAME_COUNT;
			uint32_t swap_chain_index = m_SwapChain->GetCurrentBackBufferIndex();
			RHI::Texture swap_chain_image = m_SwapChain->GetBackBuffer(swap_chain_index);
			m_FrameResources[index].command_allocator->Reset();
			m_FrameResources[index].command_list->Reset(RHI_NULL_HANDLE);
			m_FrameResources[index].graph_builder->Reset();
			m_CurrentRenderer->GPUExecution(frame_parameters, *m_FrameResources[index].graph_builder, *m_ShaderManager, *m_PipelineStateManager, *m_RootSignatureManager, swap_chain_image);
			{
				RHI::ResourceBarrier barrier = {};
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = m_SwapChain->GetBackBuffer(index);
				barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
				barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::RENDER_TARGET;
				barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::PRESENT;
				m_FrameResources[index].command_list->ResourceBarrier(1, &barrier);
			}
			m_FrameResources[index].command_list->Close();
			m_Device->ExecuteCommandList(RHI::CommandType::DIRECT, 1, &m_FrameResources[index].command_list);
			m_SwapChain->Present();
			m_Device->SignalQueue(RHI::CommandType::DIRECT, m_Fence, frame_parameters.FrameNumber + 1);
			m_CurrentFenceValue = frame_parameters.FrameNumber + 1;
		}
	private:
		RHI::RenderBackend m_RenderBackend;
		RHI::Device m_Device;
		RHI::SwapChain m_SwapChain;

		struct FrameResources {
			RHI::CommandAllocator command_allocator;
			RHI::CommandList command_list;
			FrameGraphBuilder* graph_builder;
		} m_FrameResources[FRAME_COUNT];

		GPUResourceManager* m_ResourceManager;
		GPUShaderManager* m_ShaderManager;
		GPURootSignatureManager* m_RootSignatureManager;
		GPUPipelineStateManager* m_PipelineStateManager;

		RHI::Fence m_Fence;
		uint64_t m_CurrentFenceValue;

		std::unordered_map<std::string, Renderer*> m_RendererMap;
		Renderer* m_CurrentRenderer;
	};
}
#endif