#include "FrameGraphBuilder.h"


namespace AT {
	//Build Record Render Pass To Command List Helper Function.
	inline void BuildRenderPassCommandList(RHI::Device device, RHI::CommandList command_list, FrameGraphRenderPass* render_pass, RHI::RenderPass rhi_render_pass) {
		if (render_pass->GetPipelineType() == FrameGraphRenderPass::PIPELINE_TYPE::COMPUTE) {
			render_pass->Execute(command_list);
			return;
		}
		RHI::BeginRenderPassInfo begin_info;
		RHI::FrameBufferDescription fb_description;
		fb_description.Width = 0;
		fb_description.Height = 0;
		fb_description.RenderPass = rhi_render_pass;

		const FrameGraphRenderPassIO& io = render_pass->GetIO();
		for (uint32_t i = 0; i < io.m_render_targets.size(); ++i) {
			fb_description.RenderTargetViews.push_back(render_pass->GetIO().m_render_targets[i]->RHIHandle);
			if (io.m_render_targets[i]->TextureRef->Description.Clear_Value.has_value()) {
				begin_info.ClearValues.push_back(io.m_render_targets[i]->TextureRef->Description.Clear_Value.value());
			}
			else {
				RHI::TextureClearValue value;
				value.Color[0] = 0;
				value.Color[1] = 0;
				value.Color[2] = 0;
				value.Color[3] = 0;
				begin_info.ClearValues.emplace_back(value);
			}
			fb_description.Width = (std::max)(fb_description.Width, render_pass->GetIO().m_render_targets[i]->TextureRef->RHIHandle->GetDescription().Width >> render_pass->GetIO().m_render_targets[i]->Description.MipSlice);
			fb_description.Height = (std::max)(fb_description.Height, render_pass->GetIO().m_render_targets[i]->TextureRef->RHIHandle->GetDescription().Height >> render_pass->GetIO().m_render_targets[i]->Description.MipSlice);

		}
		if (io.m_depth_buffer.has_value()) {
			fb_description.DepthStencilView = io.m_depth_buffer.value()->RHIHandle;
			if (io.m_depth_buffer.value()->TextureRef->Description.Clear_Value.has_value()) {
				begin_info.DepthClearValue = io.m_depth_buffer.value()->TextureRef->Description.Clear_Value.value();
			}
			else {
				RHI::TextureClearValue value;
				value.DepthAndStencil.Depth = 1.0f;
				value.DepthAndStencil.Stencil = 0;
				begin_info.DepthClearValue.emplace(value);
			}
			fb_description.Width = (std::max)(fb_description.Width, io.m_depth_buffer.value()->TextureRef->RHIHandle->GetDescription().Width);
			fb_description.Height = (std::max)(fb_description.Height, io.m_depth_buffer.value()->TextureRef->RHIHandle->GetDescription().Height);
		}
		device->CreateFrameBuffer(fb_description, begin_info.FrameBuffer);

		begin_info.RenderPass = fb_description.RenderPass;

		command_list->BeginRenderPass(begin_info);
		render_pass->Execute(command_list);
		command_list->EndRenderPass();
		delete begin_info.FrameBuffer;
	}

	//Build Command List for Render Pass Job.
	void BuildCommandListAsyncJob(uint32_t thread_ID, uint32_t i, uint32_t j, RHI::Device device, FrameGraphRenderPass* render_pass, RHI::RenderPass& rhi_render_pass, AT::FrameGraphRenderThreadCommandObjects* command_objects, std::vector<RHI::ResourceBarrier>& start_barriers, std::vector<std::function<void(uint32_t)>>& jobs, std::vector<RHI::CommandList>& command_lists, uint32_t command_list_id) {
		RHI::CommandList command_list = nullptr;
		command_list = command_objects->GetCommandList(render_pass->GetPipelineType());
		//If job thread has no more command lists, execute on another job thread.
		if (command_list == RHI_NULL_HANDLE) {
			AT::JobSystem::JobCounter* counter = nullptr;
			JobSystem::Execute(jobs[j], &counter);
			JobSystem::WaitForCounter(counter, 0);
			delete counter;
			return;
		}
		command_lists[command_list_id] = command_list;
		command_list->Reset(RHI_NULL_HANDLE);
		if (j == 0 && render_pass->GetPipelineType() == FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS) {
			command_list->ResourceBarrier(start_barriers.size(), start_barriers.data());
		}
		BuildRenderPassCommandList(device, command_list, render_pass, rhi_render_pass);
		command_list->Close();
	}

	//Build Command List for Resource Transition Job.
	void BuildComputeResourceBarrierCommandList(AT::FrameGraphRenderThreadCommandObjects* command_objects, std::vector<RHI::ResourceBarrier>& start_barriers, std::vector<std::function<void(uint32_t)>>& jobs, std::vector<RHI::CommandList>& command_lists) {
		RHI::CommandList command_list = nullptr;
		command_list = command_objects->GetCommandList(FrameGraphRenderPass::GRAPHICS);
		//If job thread has no more command lists, execute on another job thread.
		if (command_list == RHI_NULL_HANDLE) {
			AT::JobSystem::JobCounter* counter = nullptr;
			JobSystem::Execute(jobs.back(), &counter);
			JobSystem::WaitForCounter(counter, 0);
			delete counter;
			return;
		}
		command_lists[0] = command_list;
		command_list->Reset(RHI_NULL_HANDLE);
		command_list->ResourceBarrier(start_barriers.size(), start_barriers.data());
		command_list->Close();
	}

	FrameGraphBuilder::FrameGraphBuilder(RHI::Device device, RHI::CommandList command_list) :
		m_Device(device),
		m_RHIResourceAllocator(FrameGraphRHIResourceAllocator(m_Device)),
		m_CommandList(command_list),
		m_CurrentFenceValue(0ull)
	{
		m_256_pool = new FrameGraphCBufferPool<256, 4500>(m_RHIResourceAllocator, m_Device);
		m_2048_pool = new FrameGraphCBufferPool<2048, 500>(m_RHIResourceAllocator, m_Device);
		for (uint32_t i = 0u; i < JobSystem::Threads(); ++i) {
			FrameGraphRenderThreadCommandObjects* command_objects = new FrameGraphRenderThreadCommandObjects{};
			m_Device->CreateCommandAllocator(RHI::CommandType::DIRECT, command_objects->GraphicsCommandAllocator);
			m_Device->CreateCommandAllocator(RHI::CommandType::COMPUTE, command_objects->ComputeCommandAllocator);
			for (uint32_t j = 0u; j < COMMAND_LISTS_PER_POOL; ++j) {
				m_Device->CreateCommandList(RHI::CommandType::DIRECT, command_objects->GraphicsCommandAllocator, command_objects->GraphicsCommandListPool[j]);
				m_Device->CreateCommandList(RHI::CommandType::COMPUTE, command_objects->ComputeCommandAllocator, command_objects->ComputeCommandListPool[j]);
			}
			m_RenderThreadCommandObjects.emplace_back(command_objects);
		}
		m_Device->CreateFence(0ull, m_Fence);
	}

	void FrameGraphBuilder::CreateResources() {
		for (uint32_t i = 0; i < m_Textures.size(); ++i) {
			FrameGraphTextureRef cached_texture_ref = nullptr;
			for (auto cached_texture : m_texture_cache) {
				if (m_Textures[i]->Description == cached_texture->Description) {
					*m_Textures[i] = *cached_texture;
					cached_texture_ref = cached_texture;
					break;
				}
			}
			if (cached_texture_ref == nullptr) {
				m_Textures[i]->RHIHandle = m_RHIResourceAllocator.CreateTexture(m_Textures[i]->Description, m_Textures[i]->UsageFlags, m_Textures[i]->Description.Clear_Value);
			}
			else {
				delete cached_texture_ref;
				m_texture_cache.remove(cached_texture_ref);
			}
		}

		for (uint32_t i = 0; i < m_RTVs.size(); ++i) {
			RHI::RenderTargetViewDescription rtv_description = FrameGraphConvertRTVDescription(m_RTVs[i]->Description, m_RTVs[i]->TextureRef->Description.Format);
			m_Device->CreateRenderTargetView(m_RTVs[i]->TextureRef->RHIHandle, rtv_description, m_RTVs[i]->RHIHandle);
		}

		for (uint32_t i = 0; i < m_SRVs.size(); ++i) {
			RHI::ShaderResourceViewDescription srv_description = FrameGraphConvertSRVDescription(m_SRVs[i]->Description, m_SRVs[i]->TextureRef->Description.Format);
			m_Device->CreateShaderResourceView(m_SRVs[i]->TextureRef->RHIHandle, srv_description, m_SRVs[i]->RHIHandle);
		}

		for (uint32_t i = 0; i < m_DSVs.size(); ++i) {
			RHI::DepthStencilViewDescription dsv_description = FrameGraphConvertDSVDescription(m_DSVs[i]->Description, m_DSVs[i]->TextureRef->Description.Format);
			m_Device->CreateDepthStencilView(m_DSVs[i]->TextureRef->RHIHandle, dsv_description, m_DSVs[i]->RHIHandle);
		}

		for (uint32_t i = 0u; i < m_UAVs.size(); ++i) {
			RHI::UnorderedAccessViewDescription uav_description = FrameGraphConvertUAVDescription(m_UAVs[i]->Description, m_UAVs[i]->TextureRef->Description.Format);
			m_Device->CreateUnorderedAccessView(m_UAVs[i]->TextureRef->RHIHandle, uav_description, m_UAVs[i]->RHIHandle);
		}

		m_RHIRenderPasses = std::vector<RHI::RenderPass>(m_RenderPasses.size());
		for (uint32_t i = 0u; i < m_RenderPasses.size(); ++i) {
			// Create RHI RenderPass.
			RHI::RenderPassDescription render_pass_description;
			const FrameGraphRenderPassIO io = m_RenderPasses[i]->GetIO();
			for (uint32_t j = 0; j < io.m_render_targets.size(); ++j) {
				RHI::RenderPassAttachment attachment;
				attachment.Format = io.m_render_targets[j]->TextureRef->Description.Format;
				attachment.InitialState = RHI::TextureState::RENDER_TARGET;
				attachment.FinalState = RHI::TextureState::RENDER_TARGET;
				attachment.LoadOp = m_RenderPasses[i]->GetIO().GetLoadOp(io.m_render_targets[j]);
				attachment.StoreOp = RHI::RenderPassAttachment::StoreOperation::STORE;
				attachment.StencilLoadOp = RHI::RenderPassAttachment::LoadOperation::DONT_CARE;
				attachment.StencilStoreOp = RHI::RenderPassAttachment::StoreOperation::DONT_CARE;
				render_pass_description.Attachments.emplace_back(attachment);
			}

			if (io.m_depth_buffer.has_value()) {
				RHI::RenderPassAttachment attachment;
				attachment.Format = m_RenderPasses[i]->GetIO().m_depth_buffer.value()->TextureRef->Description.Format;
				attachment.InitialState = RHI::TextureState::DEPTH_WRITE;
				attachment.FinalState = RHI::TextureState::DEPTH_WRITE;
				attachment.LoadOp = m_RenderPasses[i]->GetIO().GetLoadOp(io.m_depth_buffer.value());
				attachment.StoreOp = RHI::RenderPassAttachment::StoreOperation::STORE;
				attachment.StencilLoadOp = RHI::RenderPassAttachment::LoadOperation::DONT_CARE;
				attachment.StencilStoreOp = RHI::RenderPassAttachment::StoreOperation::DONT_CARE;
				render_pass_description.DepthAttachment.emplace(attachment);
			}
			m_Device->CreateRenderPass(render_pass_description, m_RHIRenderPasses[i]);
		}
	}

	void FrameGraphBuilder::Compile() {
		//Constructing Adjacency List.
		std::vector<std::unordered_set<uint32_t>> adjacency_list = std::vector<std::unordered_set<uint32_t>>(m_RenderPasses.size());
		std::vector<bool> visited = std::vector<bool>(m_RenderPasses.size(), false);
		std::vector<bool> on_stack = std::vector<bool>(m_RenderPasses.size(), false);

		for (uint32_t i = 0; i < m_RenderPasses.size(); ++i) {
			const FrameGraphRenderPassIO& io = m_RenderPasses[i]->GetIO();
			for (uint32_t j = 0; j < m_RenderPasses.size(); ++j) {
				if (i == j) {
					continue;
				}
				const FrameGraphRenderPassIO& io2 = m_RenderPasses[j]->GetIO();
				for (auto read_texture : io2.m_read_textures) {
					if (io.m_write_textures.contains(read_texture)) {
						adjacency_list[i].insert(j);
					}
				}
				if (io.m_depth_buffer.has_value()) {
					if (io2.m_depth_buffer.has_value() && io2.m_depth_buffer_is_dependency && io2.m_depth_buffer.value() == io.m_depth_buffer.value()) {
						adjacency_list[i].insert(j);
					}
				}
			}
		}

		// Run Topological Sort.
		TopologicalSort(adjacency_list, visited, on_stack, m_sorted_order);

		// Create All Resources.
		CreateResources();

		// Calculate Dependency Level For Each Render Pass.
		m_RenderPassIDToDependencyLevels = std::vector<uint32_t>(m_RenderPasses.size(), 0u);
		for (uint32_t i = 0; i < m_sorted_order.size(); ++i) {
			// Calculate Dependency Level.
			for (auto j : adjacency_list[m_sorted_order[i]]) {
				if (m_RenderPassIDToDependencyLevels[j] < m_RenderPassIDToDependencyLevels[m_sorted_order[i]] + 1) {
					m_RenderPassIDToDependencyLevels[j] = m_RenderPassIDToDependencyLevels[m_sorted_order[i]] + 1;
				}
			}
		}

		//Fill Execution Dependency Level Structure.
		uint32_t max_level = (*std::max_element(m_RenderPassIDToDependencyLevels.begin(), m_RenderPassIDToDependencyLevels.end())) + 1;
		m_DependencyLevels = std::vector<DependencyLevel>(max_level);
		for (uint32_t i = 0; i < m_RenderPassIDToDependencyLevels.size(); ++i) {
			m_DependencyLevels[m_RenderPassIDToDependencyLevels[i]].RenderPassIDs.push_back(i);
			if (m_RenderPasses[i]->GetPipelineType() == FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS) {
				++m_DependencyLevels[m_RenderPassIDToDependencyLevels[i]].GraphicsPassCount;

			}
			else {
				++m_DependencyLevels[m_RenderPassIDToDependencyLevels[i]].ComputePassCount;
			}
		}

		////Debug Print.
		//for (uint32_t i = 0u; i < m_DependencyLevels.size(); ++i) {
		//	OutputDebugString((L"Dependency Level [" + std::to_wstring(i) + L"]\t").c_str());
		//	for (uint32_t j = 0u; j < m_DependencyLevels[i].RenderPassIDs.size(); ++j) {
		//		const std::string& name = m_RenderPasses[m_DependencyLevels[i].RenderPassIDs[j]]->GetName();
		//		OutputDebugString((std::wstring(name.begin(), name.end()) + L'\t').c_str());
		//	}
		//	OutputDebugString(L"\n");
		//}
		

		//Compute Transitions.
		for (uint32_t i = 0u; i < m_DependencyLevels.size(); ++i) {
			for (uint32_t j = 0u; j < m_DependencyLevels[i].RenderPassIDs.size(); ++j) {
				const FrameGraphRenderPassIO& current_io = m_RenderPasses[m_DependencyLevels[i].RenderPassIDs[j]]->GetIO();
				for (uint32_t k = 0u; k < current_io.m_render_targets.size(); ++k) {
					if (current_io.m_render_targets[k]->TextureRef->FinalState != RHI::TextureState::RENDER_TARGET) {
						RHI::ResourceBarrier barrier;
						barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
						barrier.TransitionBarrierTexture.Texture = current_io.m_render_targets[k]->TextureRef->RHIHandle;
						barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
						barrier.TransitionBarrierTexture.InitialState = current_io.m_render_targets[k]->TextureRef->FinalState;
						barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::RENDER_TARGET;
						current_io.m_render_targets[k]->TextureRef->FinalState = RHI::TextureState::RENDER_TARGET;
						m_DependencyLevels[i].StartBarriers.emplace_back(barrier);
					}
				}
				for (uint32_t k = 0u; k < current_io.m_shader_resources.size(); ++k) {
					if (current_io.m_shader_resources[k] != nullptr && current_io.m_shader_resources[k]->TextureRef->FinalState != RHI::TextureState::PIXEL_SHADER_RESOURCE) {
						RHI::ResourceBarrier barrier;
						barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
						barrier.TransitionBarrierTexture.Texture = current_io.m_shader_resources[k]->TextureRef->RHIHandle;
						barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
						barrier.TransitionBarrierTexture.InitialState = current_io.m_shader_resources[k]->TextureRef->FinalState;
						barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::PIXEL_SHADER_RESOURCE;
						current_io.m_shader_resources[k]->TextureRef->FinalState = RHI::TextureState::PIXEL_SHADER_RESOURCE;
						m_DependencyLevels[i].StartBarriers.emplace_back(barrier);
					}
				}
				if (current_io.m_depth_buffer.has_value() && current_io.m_depth_buffer.value()->TextureRef->FinalState != RHI::TextureState::DEPTH_WRITE) {
					RHI::ResourceBarrier barrier;
					barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
					barrier.TransitionBarrierTexture.Texture = current_io.m_depth_buffer.value()->TextureRef->RHIHandle;
					barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
					barrier.TransitionBarrierTexture.InitialState = current_io.m_depth_buffer.value()->TextureRef->FinalState;
					barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::DEPTH_WRITE;
					current_io.m_depth_buffer.value()->TextureRef->FinalState = RHI::TextureState::DEPTH_WRITE;
					m_DependencyLevels[i].StartBarriers.emplace_back(barrier);
				}
				for (uint32_t k = 0u; k < current_io.m_unordered_access_view_read.size(); ++k) {
					if (current_io.m_unordered_access_view_read[k] != nullptr && current_io.m_unordered_access_view_read[k]->TextureRef->FinalState != RHI::TextureState::UNORDERED_ACCESS) {
						RHI::ResourceBarrier barrier;
						barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
						barrier.TransitionBarrierTexture.Texture = current_io.m_unordered_access_view_read[k]->TextureRef->RHIHandle;
						barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
						barrier.TransitionBarrierTexture.InitialState = current_io.m_unordered_access_view_read[k]->TextureRef->FinalState;
						barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::UNORDERED_ACCESS;
						current_io.m_shader_resources[k]->TextureRef->FinalState = RHI::TextureState::UNORDERED_ACCESS;
						m_DependencyLevels[i].StartBarriers.emplace_back(barrier);
					}
				}
				for (uint32_t k = 0u; k < current_io.m_unordered_access_view_write.size(); ++k) {
					if (current_io.m_unordered_access_view_write[k] != nullptr && current_io.m_unordered_access_view_write[k]->TextureRef->FinalState != RHI::TextureState::UNORDERED_ACCESS) {
						RHI::ResourceBarrier barrier;
						barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
						barrier.TransitionBarrierTexture.Texture = current_io.m_unordered_access_view_write[k]->TextureRef->RHIHandle;
						barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
						barrier.TransitionBarrierTexture.InitialState = current_io.m_unordered_access_view_write[k]->TextureRef->FinalState;
						barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::UNORDERED_ACCESS;
						current_io.m_unordered_access_view_write[k]->TextureRef->FinalState = RHI::TextureState::UNORDERED_ACCESS;
						m_DependencyLevels[i].StartBarriers.emplace_back(barrier);
					}
				}

				//Compute Cross Queue Syncrhonization.
				uint32_t render_pass_id = m_DependencyLevels[i].RenderPassIDs[j];
				FrameGraphRenderPass::PIPELINE_TYPE pipeline_type = m_RenderPasses[render_pass_id]->GetPipelineType();
				for (auto k : adjacency_list[render_pass_id]) {
					if (m_RenderPasses[k]->GetPipelineType() != pipeline_type) {
						if (m_RenderPasses[k]->GetPipelineType() == FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS) {
							m_DependencyLevels[m_RenderPassIDToDependencyLevels[k]].GraphicsWaitForDependencyLevel = (std::max)(m_DependencyLevels[m_RenderPassIDToDependencyLevels[k]].GraphicsWaitForDependencyLevel, (int32_t)i);
						}
						else {
							m_DependencyLevels[m_RenderPassIDToDependencyLevels[k]].ComputeWaitForDependencyLevel = (std::max)(m_DependencyLevels[m_RenderPassIDToDependencyLevels[k]].ComputeWaitForDependencyLevel, (int32_t)i);
						}
					}
				}
			}
		}

		for (uint32_t i = 0u; i < m_DependencyLevels.size(); ++i) {
			if (m_DependencyLevels[i].GraphicsWaitForDependencyLevel > -1) {
				m_DependencyLevels[m_DependencyLevels[i].GraphicsWaitForDependencyLevel].SignalComputeValue = ++m_CurrentFenceValue;
			}
			if (m_DependencyLevels[i].ComputeWaitForDependencyLevel > -1) {
				m_DependencyLevels[m_DependencyLevels[i].ComputeWaitForDependencyLevel].SignalGraphicsValue = ++m_CurrentFenceValue;
			}
		}
	}

	void FrameGraphBuilder::Execute() {
		AT::JobSystem::JobCounter* lvl_counter = nullptr;

		for (uint32_t i = 0; i < m_DependencyLevels.size(); ++i) {
			//Check if Cross Queue Wait is Needed.
			if (m_DependencyLevels[i].GraphicsWaitForDependencyLevel > -1) {
				m_Device->QueueWait(RHI::CommandType::DIRECT, m_Fence, m_DependencyLevels[m_DependencyLevels[i].GraphicsWaitForDependencyLevel].SignalComputeValue);
			}
			if (m_DependencyLevels[i].ComputeWaitForDependencyLevel > -1) {
				m_Device->QueueWait(RHI::CommandType::COMPUTE, m_Fence, m_DependencyLevels[m_DependencyLevels[i].ComputeWaitForDependencyLevel].SignalGraphicsValue);
			}

			if (i < m_DependencyLevels.size() - 1) {
				//Splite work by executing each RenderPass per job system thread.
				std::vector<std::function<void(uint32_t)>> m_jobs = std::vector<std::function<void(uint32_t)>>(m_DependencyLevels[i].RenderPassIDs.size());
				std::vector<RHI::CommandList>m_graphics_command_lists = std::vector<RHI::CommandList>(m_DependencyLevels[i].GraphicsPassCount);
				std::vector<RHI::CommandList>m_compute_command_lists = std::vector<RHI::CommandList>(m_DependencyLevels[i].ComputePassCount);
				
				AT::JobSystem::JobCounter* counter = nullptr;
				uint32_t graphics_id = 0u, compute_id = 0u;
				if (m_graphics_command_lists.size() == 0u) {
					m_graphics_command_lists.push_back(RHI_NULL_HANDLE);
					m_jobs.push_back([=, &m_graphics_command_lists, &m_jobs](uint32_t thread_ID) {
						BuildComputeResourceBarrierCommandList(m_RenderThreadCommandObjects[thread_ID], m_DependencyLevels[i].StartBarriers, m_jobs, m_graphics_command_lists);
					});
					++graphics_id;
				}
				for (uint32_t j = 0u; j < m_DependencyLevels[i].RenderPassIDs.size(); ++j) {
					if (m_RenderPasses[m_DependencyLevels[i].RenderPassIDs[j]]->GetPipelineType() == FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS) {
						m_jobs[j] = [=, &m_jobs, &m_graphics_command_lists](uint32_t thread_ID) {
							BuildCommandListAsyncJob(thread_ID, i, j, m_Device, m_RenderPasses[m_DependencyLevels[i].RenderPassIDs[j]], m_RHIRenderPasses[m_DependencyLevels[i].RenderPassIDs[j]], m_RenderThreadCommandObjects[thread_ID], m_DependencyLevels[i].StartBarriers, m_jobs, m_graphics_command_lists, graphics_id);
						};
						++graphics_id;
					}
					else {
						m_jobs[j] = [=, &m_jobs, &m_compute_command_lists](uint32_t thread_ID) {
							BuildCommandListAsyncJob(thread_ID, i, j, m_Device, m_RenderPasses[m_DependencyLevels[i].RenderPassIDs[j]], m_RHIRenderPasses[m_DependencyLevels[i].RenderPassIDs[j]], m_RenderThreadCommandObjects[thread_ID], m_DependencyLevels[i].StartBarriers, m_jobs, m_compute_command_lists, compute_id);
						};
						++compute_id;
					}
				}
				JobSystem::Execute(m_jobs, &counter);
				JobSystem::WaitForCounter(counter, 0);
				delete counter;
				m_Device->ExecuteCommandList(RHI::CommandType::DIRECT, m_graphics_command_lists.size(), m_graphics_command_lists.data());
				if (m_DependencyLevels[i].SignalGraphicsValue > 0) {
					m_Device->SignalQueue(RHI::CommandType::DIRECT, m_Fence, m_DependencyLevels[i].SignalGraphicsValue);
				}
				if (m_compute_command_lists.size() > 0) {
					m_Device->ExecuteCommandList(RHI::CommandType::COMPUTE, m_compute_command_lists.size(), m_compute_command_lists.data());
					if (m_DependencyLevels[i].SignalComputeValue > 0) {
						m_Device->SignalQueue(RHI::CommandType::COMPUTE, m_Fence, m_DependencyLevels[i].SignalComputeValue);
					}
				}
			}
			else {
				m_CommandList->ResourceBarrier(m_DependencyLevels[i].StartBarriers.size(), m_DependencyLevels[i].StartBarriers.data());
				for (uint32_t j = 0u; j < m_DependencyLevels[i].RenderPassIDs.size(); ++j) {
					uint32_t id = m_DependencyLevels[i].RenderPassIDs[j];
					BuildRenderPassCommandList(m_Device, m_CommandList, m_RenderPasses[id], m_RHIRenderPasses[id]);
				}
			}

			//Single Threaded Execution Implementation.
			/*m_CommandList->ResourceBarrier(m_DependencyLevels[i].StartBarriers.size(), m_DependencyLevels[i].StartBarriers.data());
			for (uint32_t j = 0u; j < m_DependencyLevels[i].RenderPassIDs.size(); ++j) {
				uint32_t id = m_DependencyLevels[i].RenderPassIDs[j];
				BuildRenderPassCommandList(m_Device, m_CommandList, m_RenderPasses[id], m_RHIRenderPasses[id]);
			}*/
		}
	}
}