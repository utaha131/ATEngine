#ifndef _AT_RENDER_SYSTEM_GPU_RESOURCE_MANAGER_H_
#define _AT_RENDER_SYSTEM_GPU_RESOURCE_MANAGER_H_
#include "GPUResources.h"
#include "GPUMemoryAllocator.h"
#include "../JobSystem/JobSystem.h"
#include "../RenderSystem/ShaderPrograms/GenerateMipChain2DShader.h"
#include "GPUShaderManager.h"
#include <mutex>
#include <algorithm>
#define _1_GB_ 536870912 * 2

namespace AT {

	//GPU Upload Structures.
	struct GPUResourceUpload {
		enum TYPE {
			BUFFER,
			TEXTURE
		} Type;
		union {
			struct BufferUpload {
				GPUBufferPtr Buffer;
				GPUBufferPtr StagingBuffer;
			} BufferUpload;
			struct TextureUpload {
				GPUTexturePtr Texture;
				GPUBufferPtr StagingBuffer;
			} TextureUpload;
		};
	};
	class GPUResourceUploadBatch {
	public:
		void AddBufferUpload(GPUBufferPtr dest_buffer, GPUBufferPtr src_buffer) {
			GPUResourceUpload upload = {};
			upload.Type = GPUResourceUpload::TYPE::BUFFER;
			upload.BufferUpload.Buffer = dest_buffer;
			upload.BufferUpload.StagingBuffer = src_buffer;
			m_Uploads.push_back(upload);
		}
		void AddTextureUpload(GPUTexturePtr dest_texture, GPUBufferPtr src_buffer) {
			GPUResourceUpload upload = {};
			upload.Type = GPUResourceUpload::TYPE::TEXTURE;
			upload.TextureUpload.Texture = dest_texture;
			upload.TextureUpload.StagingBuffer = src_buffer;
			m_Uploads.push_back(upload);
		}
	private:
		friend class GPUResourceManager;
		std::vector<GPUResourceUpload> m_Uploads;
	};

	//GPU State Transition Structures.
	struct GPUResourceTransition {
		enum TYPE {
			BUFFER,
			TEXTURE
		} Type;
		union {
			struct BufferTransition {
				GPUBufferPtr Buffer;
				RHI::BufferState FinalState;
			} BufferTransition;
			struct TextureTransition {
				GPUTexturePtr Texture;
				RHI::TextureState FinalState;
			} TextureTransition;
		};
	};
	class GPUResourceTransitionBatch {
	public:
		void AddBufferTransition(GPUBufferPtr buffer, RHI::BufferState final_state) {
			GPUResourceTransition transition;
			transition.Type = GPUResourceTransition::TYPE::BUFFER;
			transition.BufferTransition.Buffer = buffer;
			transition.BufferTransition.FinalState = final_state;
			m_transitions.emplace_back(transition);
		}
		void AddTextureTransition(GPUTexturePtr texture, RHI::TextureState final_state) {
			GPUResourceTransition transition = {};
			transition.Type = GPUResourceTransition::TYPE::TEXTURE;
			transition.TextureTransition.Texture = texture;
			transition.TextureTransition.FinalState = final_state;
			m_transitions.emplace_back(transition);
		}
	private:
		friend class GPUResourceManager;
		std::vector<GPUResourceTransition> m_transitions;
	};

	struct GPUGenerateMipsBatch {
		void AddTexture(GPUTexturePtr texture) {
			m_source_textures.push_back(texture);
		}
	private:
		friend class GPUResourceManager;
		std::vector<GPUTexturePtr> m_source_textures;
	};

	class GPURayTracingAccelerationStructureBuildBatch {
		void AddBuild() {

		}
	};

	//GPU Resource Manager.
	class GPUResourceManager {
	public:
		GPUResourceManager(RHI::Device device, AT::GPUShaderManager& shader_manager, AT::GPURootSignatureManager& root_signature_manager) :
			m_Device(device),
			m_CurrentFenceValue(0ull)
		{
			m_GenerateMipChainShader = new GenerateMipChain2DShader(shader_manager.LoadRHIShader("GenerateMipChain", RHI::ShaderType::COMPUTE), root_signature_manager);//new GenerateMipsShader(shader_manager.LoadRHIShader("GenerateMips", RHI_SHADER_TYPE_COMPUTE), root_signature_manager);
			RHI::ResourceHeapDescription default_heap_description;
			default_heap_description.ResourceHeapType = RHI::ResourceHeapType::DEFAULT;
			default_heap_description.Size = _1_GB_;
			RHI::ResourceHeap default_heap;
			m_Device->CreateResourceHeap(default_heap_description, default_heap);
			m_DefaultAllocator = GPUMemoryAllocator::FreeListAllocator(default_heap, default_heap_description.Size);
			m_ResourceHeaps.push_back(default_heap);

			RHI::ResourceHeapDescription upload_heap_description;
			upload_heap_description.ResourceHeapType = RHI::ResourceHeapType::UPLOAD;
			upload_heap_description.Size = _1_GB_;
			RHI::ResourceHeap upload_heap;
			m_Device->CreateResourceHeap(upload_heap_description, upload_heap);
			m_UploadAllocator = GPUMemoryAllocator::FreeListAllocator(upload_heap, upload_heap_description.Size);
			m_ResourceHeaps.push_back(upload_heap);

			m_Device->CreateCommandAllocator(RHI::CommandType::DIRECT, m_GraphicsCommandAllocator);
			m_Device->CreateCommandList(RHI::CommandType::DIRECT, m_GraphicsCommandAllocator, m_GraphicsCommandList);

			m_Device->CreateCommandAllocator(RHI::CommandType::COMPUTE, m_ComputeCommandAllocator);
			m_Device->CreateCommandList(RHI::CommandType::COMPUTE, m_ComputeCommandAllocator, m_ComputeCommandList);

			{
				m_CopyCommandAllocators = std::vector<RHI::CommandAllocator>(JobSystem::Threads());
				m_CopyCommandLists = std::vector<RHI::CommandList>(JobSystem::Threads());
				for (uint32_t i = 0; i < JobSystem::Threads(); ++i) {
					device->CreateCommandAllocator(RHI::CommandType::COPY, m_CopyCommandAllocators[i]);
					device->CreateCommandList(RHI::CommandType::COPY, m_CopyCommandAllocators[i], m_CopyCommandLists[i]);
				}
			}

			{
				RHI::ComputePipelineStateDescription compute_pso;
				compute_pso.RootSignature = m_GenerateMipChainShader->GetRootSignature();
				compute_pso.ComputeShader = m_GenerateMipChainShader->GetComputeShader();
				compute_pso.NodeMask = 0;
				m_Device->CreateComputePipelineState(compute_pso, m_GenerateMipChainPSO);
			}
			m_Device->CreateFence(0, m_Fence);
		}

		~GPUResourceManager() {
			m_Device->HostWait(m_Fence, m_CurrentFenceValue);
		
			for (uint32_t i = 0; i < m_UAVs.size(); ++i) {
				delete m_UAVs[i];
			}

			for (uint32_t i = 0u; i < m_SRVs.size(); ++i) {
				delete m_SRVs[i];
			}

			for (auto buffer : m_Buffers) {
				delete buffer->m_RHI;
				if (buffer->m_MemoryType == GPUBuffer::MEMORY_TYPE::DEVICE) {
					m_DefaultAllocator.Free(buffer->m_Allocation);
				}
				else {
					m_UploadAllocator.Free(buffer->m_Allocation);
				}
				delete buffer;
			}

			for (auto texture : m_Textures) {
				delete texture->m_RHI;
				m_DefaultAllocator.Free(texture->m_Allocation);
				delete texture;
			}

			delete m_GenerateMipChainShader;
			delete m_GenerateMipChainPSO;
			//release graphics command objects.
			delete m_GraphicsCommandList;
			delete m_GraphicsCommandAllocator;
			//release compute command objects.
			delete m_ComputeCommandList;
			delete m_ComputeCommandAllocator;
			//release copy command objects.
			for (uint32_t i = 0; i < m_CopyCommandLists.size(); ++i) {
				delete m_CopyCommandLists[i];
				delete m_CopyCommandAllocators[i];
			}
			delete m_Fence;

			for (uint32_t i = 0u; i < m_ResourceHeaps.size(); ++i) {
				delete m_ResourceHeaps[i];
			}
		}

		GPUBufferPtr CreateBuffer(const RHI::BufferDescription& description, RHI::BufferState buffer_state = RHI::BufferState::COMMON) {
			RHI::AllocationInfo allocation_info = m_Device->GetResourceAllocationInfo(description);
			GPUMemoryAllocator::FreeListAllocator::Allocation allocation = m_DefaultAllocator.Allocate(allocation_info.Size, allocation_info.Alignment);
			GPUBufferPtr buffer = new GPUBuffer{};
			buffer->m_MemoryType = GPUBuffer::MEMORY_TYPE::DEVICE;
			buffer->m_Allocation = allocation;
			m_Device->CreateBuffer(m_DefaultAllocator.GetResourceHeap(), allocation.offset + allocation.padding, buffer_state, description, buffer->m_RHI);
			return m_Buffers.emplace_back(buffer);
		}

		GPUBufferPtr CreateUploadBuffer(const RHI::BufferDescription& description, RHI::BufferState buffer_state = RHI::BufferState::COMMON) {
			RHI::AllocationInfo allocation_info = m_Device->GetResourceAllocationInfo(description);
			GPUMemoryAllocator::FreeListAllocator::Allocation allocation = m_UploadAllocator.Allocate(allocation_info.Size, allocation_info.Alignment);
			GPUBufferPtr buffer = new GPUBuffer{};
			buffer->m_MemoryType = GPUBuffer::MEMORY_TYPE::HOST;
			buffer->m_Allocation = allocation;
			m_Device->CreateBuffer(m_UploadAllocator.GetResourceHeap(), allocation.offset + allocation.padding, buffer_state, description, buffer->m_RHI);
			return m_Buffers.emplace_back(buffer);
		}

		//TODO implement create constant buffer function.

		GPUTexturePtr CreateTexture(const RHI::TextureDescription& description) {
			RHI::AllocationInfo allocation_info = m_Device->GetResourceAllocationInfo(description);
			GPUMemoryAllocator::FreeListAllocator::Allocation allocation = m_DefaultAllocator.Allocate(allocation_info.Size, allocation_info.Alignment);
			GPUTexturePtr texture = new GPUTexture();
			texture->m_Allocation = allocation;
			m_Device->CreateTexture(m_DefaultAllocator.GetResourceHeap(), allocation.offset + allocation.padding, {}, description, texture->m_RHI);
			return m_Textures.emplace_back(texture);
		}

		void FreeBuffer(GPUBufferPtr buffer) {
			m_Buffers.remove(buffer);
			delete buffer->m_RHI;
			m_UploadAllocator.Free(buffer->m_Allocation);
			delete buffer;
		}

		void FreeTexture(GPUTexturePtr texture) {
			m_Textures.remove(texture);
			delete texture;
		}

		void UploadBatches(const GPUResourceUploadBatch& batch) {
			m_Device->HostWait(m_Fence, m_CurrentFenceValue);
			AT::JobSystem::JobCounter* counter;
			std::vector<std::function<void(uint32_t thread_ID)>> jobs;
			for (uint32_t i = 0; i < JobSystem::Threads(); ++i) {
				m_CopyCommandAllocators[i]->Reset();
				m_CopyCommandLists[i]->Reset(RHI_NULL_HANDLE);
				jobs.push_back([=](uint32_t thread_ID) {
					std::vector<RHI::ResourceBarrier> start_barriers;
					std::vector<RHI::ResourceBarrier> end_barriers;
					for (uint32_t j = i; j < batch.m_Uploads.size(); j += JobSystem::Threads()) {
						if (batch.m_Uploads[j].Type == GPUResourceUpload::TYPE::BUFFER) {
							assert(batch.m_Uploads[j].BufferUpload.Buffer->m_State == RHI::BufferState::COMMON && "Destination buffer must be in common state.");
							assert(batch.m_Uploads[j].BufferUpload.StagingBuffer->m_State == RHI::BufferState::COMMON && "Source buffer must be in common state.");
							{
								RHI::ResourceBarrier barrier;
								barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_BUFFER;
								barrier.TransitionBarrierBuffer.Buffer = batch.m_Uploads[j].BufferUpload.Buffer->m_RHI;
								barrier.TransitionBarrierBuffer.InitialState = RHI::BufferState::COMMON;
								barrier.TransitionBarrierBuffer.FinalState = RHI::BufferState::COPY_DEST;
								start_barriers.push_back(barrier);
								barrier.TransitionBarrierBuffer.InitialState = RHI::BufferState::COPY_DEST;
								barrier.TransitionBarrierBuffer.FinalState = RHI::BufferState::COMMON;
								end_barriers.push_back(barrier);
							}
							{
								RHI::ResourceBarrier barrier;
								barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_BUFFER;
								barrier.TransitionBarrierBuffer.Buffer = batch.m_Uploads[j].BufferUpload.StagingBuffer->m_RHI;
								barrier.TransitionBarrierBuffer.InitialState = RHI::BufferState::COMMON;
								barrier.TransitionBarrierBuffer.FinalState = RHI::BufferState::COPY_SOURCE;
								start_barriers.push_back(barrier);
								barrier.TransitionBarrierBuffer.InitialState = RHI::BufferState::COPY_SOURCE;
								barrier.TransitionBarrierBuffer.FinalState = RHI::BufferState::COMMON;
								end_barriers.push_back(barrier);
							}
							
						}
						else {
							assert(batch.m_Uploads[j].TextureUpload.Texture->m_State == RHI::TextureState::COMMON && "Destination texture must be in common state.");
							assert(batch.m_Uploads[j].TextureUpload.StagingBuffer->m_State == RHI::BufferState::COMMON && "Source buffer must be in common state.");
							{
								RHI::ResourceBarrier barrier;
								barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
								barrier.TransitionBarrierTexture.Texture = batch.m_Uploads[j].TextureUpload.Texture->m_RHI;
								barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
								barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::COMMON;
								barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::COPY_DEST;
								start_barriers.push_back(barrier);
								barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::COPY_DEST;
								barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::COMMON;
								end_barriers.push_back(barrier);
							}
							{
								RHI::ResourceBarrier barrier;
								barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_BUFFER;
								barrier.TransitionBarrierBuffer.Buffer = batch.m_Uploads[j].TextureUpload.StagingBuffer->m_RHI;
								barrier.TransitionBarrierBuffer.InitialState = RHI::BufferState::COMMON;
								barrier.TransitionBarrierBuffer.FinalState = RHI::BufferState::COPY_SOURCE;
								start_barriers.push_back(barrier);
								barrier.TransitionBarrierBuffer.InitialState = RHI::BufferState::COPY_SOURCE;
								barrier.TransitionBarrierBuffer.FinalState = RHI::BufferState::COMMON;
								end_barriers.push_back(barrier);
							}
						}
					}
					
					m_CopyCommandLists[i]->ResourceBarrier(start_barriers.size(), start_barriers.data());

					for (uint32_t j = i; j < batch.m_Uploads.size(); j += JobSystem::Threads()) {
						if (batch.m_Uploads[j].Type == GPUResourceUpload::TYPE::BUFFER) {
							assert(batch.m_Uploads[j].BufferUpload.Buffer->m_State == RHI::BufferState::COMMON && "Destination buffer must be in common state.");
							assert(batch.m_Uploads[j].BufferUpload.StagingBuffer->m_State == RHI::BufferState::COMMON && "Source buffer must be in common state.");
							m_CopyCommandLists[i]->CopyBuffer(batch.m_Uploads[j].BufferUpload.Buffer->m_RHI, batch.m_Uploads[j].BufferUpload.StagingBuffer->m_RHI);
						}
						else {
							assert(batch.m_Uploads[j].TextureUpload.Texture->m_State == RHI::TextureState::COMMON && "Destination texture must be in common state.");
							assert(batch.m_Uploads[j].TextureUpload.StagingBuffer->m_State == RHI::BufferState::COMMON && "Source buffer must be in common state.");

							for (uint32_t k = 0; k < batch.m_Uploads[j].TextureUpload.Texture->m_RHI->GetDescription().DepthOrArray; ++k) {
								RHI::TextureCopyLocation texture_location;
								texture_location.Texture = batch.m_Uploads[j].TextureUpload.Texture->m_RHI;
								texture_location.Type = RHI::CopyLocationType::SUBRESOURCE;
								texture_location.Subresource.MipSlice = 0;
								texture_location.Subresource.MipLevels = 1;
								texture_location.Subresource.ArraySlice = k;

								RHI::BufferTextureCopyLocation buffer_location;
								buffer_location.Buffer = batch.m_Uploads[j].TextureUpload.StagingBuffer->m_RHI;
								buffer_location.Type = RHI::CopyLocationType::FOOTPRINT;
								buffer_location.Footprint.Format = batch.m_Uploads[j].TextureUpload.Texture->m_RHI->GetDescription().Format;
								buffer_location.Footprint.Width = batch.m_Uploads[j].TextureUpload.Texture->m_RHI->GetDescription().Width;
								buffer_location.Footprint.Height = batch.m_Uploads[j].TextureUpload.Texture->m_RHI->GetDescription().Height;
								buffer_location.Footprint.Depth = 1;// batch.m_uploads[j].Texture_Upload.Texture->rhi_texture->GetDescription().Depth_OR_Array;
								uint32_t b = m_Device->GetTexturePitchAlignment() - 1;
								buffer_location.Footprint.RowPitch = ((buffer_location.Footprint.Width * 4) + b) & ~b;
								buffer_location.Footprint.Offset = k * buffer_location.Footprint.RowPitch * buffer_location.Footprint.Height;
								m_CopyCommandLists[i]->CopyBufferToTexture(texture_location, buffer_location);
							}
						}
					}
					m_CopyCommandLists[i]->ResourceBarrier(end_barriers.size(), end_barriers.data());
					m_CopyCommandLists[i]->Close();
				});
				
			}
			JobSystem::Execute(jobs, &counter);
			JobSystem::WaitForCounter(counter, 0);
			delete counter;

			m_Device->QueueWait(RHI::CommandType::COPY, m_Fence, m_CurrentFenceValue);
			m_Device->ExecuteCommandList(RHI::CommandType::COPY, m_CopyCommandLists.size(), m_CopyCommandLists.data());
			m_Device->SignalQueue(RHI::CommandType::COPY, m_Fence, ++m_CurrentFenceValue);
		}

		void WaitForIdle() {
			m_Device->HostWait(m_Fence, m_CurrentFenceValue);
		}

		void ExecuteTransitions(GPUResourceTransitionBatch& transition_batch) {
			m_Device->HostWait(m_Fence, m_CurrentFenceValue);
			std::vector<RHI::ResourceBarrier> resource_barriers;
			for (uint32_t i = 0; i < transition_batch.m_transitions.size(); ++i) {
				if (transition_batch.m_transitions[i].Type == GPUResourceTransition::TYPE::BUFFER) {
					RHI::ResourceBarrier barrier;
					barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_BUFFER;
					barrier.TransitionBarrierBuffer.Buffer = transition_batch.m_transitions[i].BufferTransition.Buffer->m_RHI;
					barrier.TransitionBarrierBuffer.InitialState = transition_batch.m_transitions[i].BufferTransition.Buffer->m_State;
					/*if (m_initial_buffers.contains(transition_batch.m_transitions[i].Buffer_Transition.Buffer)) {
						barrier.Transition_Barrier_Buffer.Initial_State = RHI_BUFFER_STATE_INITIAL;
						m_initial_buffers.erase(transition_batch.m_transitions[i].Buffer_Transition.Buffer);
					}*/
					barrier.TransitionBarrierBuffer.FinalState = transition_batch.m_transitions[i].BufferTransition.FinalState;
					transition_batch.m_transitions[i].BufferTransition.Buffer->m_State = transition_batch.m_transitions[i].BufferTransition.FinalState;
					resource_barriers.emplace_back(barrier);
				}
				else {
					RHI::ResourceBarrier barrier;
					barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
					barrier.TransitionBarrierTexture.Texture = transition_batch.m_transitions[i].TextureTransition.Texture->m_RHI;
					//barrier.TransitionBarrierTexture.SubresourceIndex = RHI_SUBRESOURCE_INDEX_ALL;
					barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
					barrier.TransitionBarrierTexture.InitialState = transition_batch.m_transitions[i].TextureTransition.Texture->m_State;
					/*if (m_initial_textures.contains(transition_batch.m_transitions[i].Texture_Transition.Texture)) {
						barrier.Transition_Barrier_Texture.Initial_State = RHI_TEXTURE_STATE_INITIAL;
						m_initial_textures.erase(transition_batch.m_transitions[i].Texture_Transition.Texture);
					}*/
					barrier.TransitionBarrierTexture.FinalState = transition_batch.m_transitions[i].TextureTransition.FinalState;
					transition_batch.m_transitions[i].TextureTransition.Texture->m_State = transition_batch.m_transitions[i].TextureTransition.FinalState;
					resource_barriers.emplace_back(barrier);
				}
			}
			m_GraphicsCommandAllocator->Reset();
			m_GraphicsCommandList->Reset(RHI_NULL_HANDLE);
			m_GraphicsCommandList->ResourceBarrier(resource_barriers.size(), resource_barriers.data());
			m_GraphicsCommandList->Close();
			m_Device->QueueWait(RHI::CommandType::DIRECT, m_Fence, m_CurrentFenceValue);
			m_Device->ExecuteCommandList(RHI::CommandType::DIRECT, 1, &m_GraphicsCommandList);
			m_Device->SignalQueue(RHI::CommandType::DIRECT, m_Fence, ++m_CurrentFenceValue);
		}

		void GenerateMips(GPUGenerateMipsBatch& batch) {
			m_Device->HostWait(m_Fence, m_CurrentFenceValue);
			m_ComputeCommandAllocator->Reset();
			m_ComputeCommandList->Reset(RHI_NULL_HANDLE);
			uint32_t max_width = batch.m_source_textures[0]->m_RHI->GetDescription().Width;
			uint32_t max_height = batch.m_source_textures[0]->m_RHI->GetDescription().Height;
			for (uint32_t i = 0; i < batch.m_source_textures.size(); ++i) {
				if (batch.m_source_textures[i]->m_RHI->GetDescription().Width > max_width) {
					max_width = batch.m_source_textures[i]->m_RHI->GetDescription().Width;
				}
				if (batch.m_source_textures[i]->m_RHI->GetDescription().Height > max_height) {
					max_height = batch.m_source_textures[i]->m_RHI->GetDescription().Height;
				}
			}

			for (uint32_t i = 0; i < batch.m_source_textures.size(); ++i) {
				batch.m_source_textures[i] = GenerateMips(batch.m_source_textures[i], 11);
			}
			m_ComputeCommandList->Close();
			m_Device->QueueWait(RHI::CommandType::COMPUTE, m_Fence, m_CurrentFenceValue);
			m_Device->ExecuteCommandList(RHI::CommandType::COMPUTE, 1, &m_ComputeCommandList);
			m_Device->SignalQueue(RHI::CommandType::COMPUTE, m_Fence, ++m_CurrentFenceValue);
		}

		RHI::Device GetDevice() const {
			return m_Device;
		}
	protected:

		GPUTexturePtr GenerateMips(GPUTexturePtr src_texture, uint32_t mip_levels) {
			if (src_texture->m_RHI->GetDescription().Width < 5) {
				return src_texture;
			}
			{
				RHI::ResourceBarrier barrier;
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = src_texture->m_RHI;
				barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
				barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::COMMON;
				barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::UNORDERED_ACCESS;
				m_ComputeCommandList->ResourceBarrier(1, &barrier);
			}
			{
				RHI::ResourceBarrier barrier;
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = src_texture->m_RHI;
				barrier.TransitionBarrierTexture.Subresource.MipSlice = 0;
				barrier.TransitionBarrierTexture.Subresource.MipLevels = src_texture->GetRHIHandle()->GetDescription().MipLevels;
				barrier.TransitionBarrierTexture.Subresource.ArraySlice = 0;
				barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::UNORDERED_ACCESS;
				barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::NON_PIXEL_SHADER_RESOURCE;
				m_ComputeCommandList->ResourceBarrier(1, &barrier);
			}
			for (uint32_t i = 0u; i < mip_levels - 1; ++i) {
				uint32_t width = src_texture->m_RHI->GetDescription().Width >> (i + 1);
				uint32_t height = src_texture->m_RHI->GetDescription().Height >> (i + 1);
				RHI::ShaderResourceView srv;
				{
					RHI::ShaderResourceViewDescription srv_description;
					srv_description.Format = (src_texture->m_RHI->GetDescription().Format == RHI::Format::R8G8B8A8_TYPELESS) ? RHI::Format::R8G8B8A8_UNORM_SRGB : src_texture->m_RHI->GetDescription().Format;
					srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::TEXTURE_2D;
					srv_description.Texture2D.MostDetailedMip = i;
					srv_description.Texture2D.MipLevels = 1;
					srv_description.Texture2D.PlaneSlice = 0;
					srv_description.Texture2D.ResourceMinLODClamp = 0;
					m_Device->CreateShaderResourceView(src_texture->m_RHI, srv_description, srv);
					m_SRVs.push_back(srv);
				}
				RHI::UnorderedAccessView uav;
				{
					RHI::UnorderedAccessViewDescription uav_description;
					uav_description.Format = RHI::Format::R8G8B8A8_UNORM;
					uav_description.ViewDimension = RHI::UnorderedAccessViewViewDimension::TEXTURE_2D;
					uav_description.Texture2D.MipSlice = (i + 1);
					uav_description.Texture2D.PlaneSlice = 0u;
					m_Device->CreateUnorderedAccessView(src_texture->m_RHI, uav_description, uav);
					m_UAVs.push_back(uav);
				}

				AT::GenerateMipChain2DShader::Parameters parameters;
				parameters.Compute = new AT::GenerateMipChain2DShader::ComputeGroup{};
				parameters.Compute->constants.IsSRGB = (src_texture->m_RHI->GetDescription().Format == RHI::Format::R8G8B8A8_TYPELESS) ? true : false;
				parameters.Compute->constants.OutputResolution = { (float)width, (float)height };
				GPUBufferPtr upload_buffer;
				{
					RHI::BufferDescription buffer_description;
					buffer_description.Size = (sizeof(AT::GenerateMipChain2DShader::ComputeGroup) + 255) & ~255;
					buffer_description.UsageFlags = RHI::BufferUsageFlag::UNIFORM_BUFFER;
					upload_buffer = CreateUploadBuffer(buffer_description);
				}

				parameters.Compute->constant_buffer = new AT::GPUConstantBuffer(m_Device, upload_buffer->m_RHI);

				m_Device->AllocateDescriptorTable(m_GenerateMipChainShader->GetRootSignature(), 0, parameters.Compute->m_descriptor_table);
				parameters.Compute->SourceTexture.srv = srv;
				parameters.Compute->OutputTexture.uav = uav;

				m_ComputeCommandList->SetPipelineState(m_GenerateMipChainPSO);
				m_ComputeCommandList->SetComputeRootSignature(m_GenerateMipChainShader->GetRootSignature());
				m_GenerateMipChainShader->SetParameters(m_ComputeCommandList, &parameters);
				m_ComputeCommandList->Dispatch((std::ceil)(width / 8.0f), (std::ceil)(height / 8.0f), 1);
				{
					RHI::ResourceBarrier barrier;
					barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
					barrier.TransitionBarrierTexture.Texture = src_texture->m_RHI;
					barrier.TransitionBarrierTexture.Subresource.MipSlice = i + 1;
					barrier.TransitionBarrierTexture.Subresource.MipLevels = src_texture->GetRHIHandle()->GetDescription().MipLevels;
					barrier.TransitionBarrierTexture.Subresource.ArraySlice = 0;

					barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::UNORDERED_ACCESS;
					barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::NON_PIXEL_SHADER_RESOURCE;
					m_ComputeCommandList->ResourceBarrier(1, &barrier);
				}
			}
			{
				RHI::ResourceBarrier barrier;
				barrier.ResourceBarrierType = RHI::ResourceBarrierType::TRANSITION_BARRIER_TEXTURE;
				barrier.TransitionBarrierTexture.Texture = src_texture->m_RHI;
				barrier.TransitionBarrierTexture.Subresource = RHI::SUBRESOURCE_ALL;
				barrier.TransitionBarrierTexture.InitialState = RHI::TextureState::NON_PIXEL_SHADER_RESOURCE;
				barrier.TransitionBarrierTexture.FinalState = RHI::TextureState::COMMON;
				m_ComputeCommandList->ResourceBarrier(1, &barrier);
			}
		}
	private:
		RHI::Device m_Device;
		std::vector<RHI::ResourceHeap> m_ResourceHeaps;
		GPUMemoryAllocator::FreeListAllocator m_DefaultAllocator;
		GPUMemoryAllocator::FreeListAllocator m_UploadAllocator;
		std::list<GPUBufferPtr> m_Buffers;
		std::list<GPUTexturePtr> m_Textures;
		RHI::CommandList m_GraphicsCommandList;
		RHI::CommandAllocator m_GraphicsCommandAllocator;
		RHI::Fence m_Fence;
		uint64_t m_CurrentFenceValue;

		std::mutex mutex;
	
		std::vector<RHI::CommandAllocator> m_CopyCommandAllocators;
		std::vector<RHI::CommandList> m_CopyCommandLists;


		RHI::CommandList m_ComputeCommandList;
		RHI::CommandAllocator m_ComputeCommandAllocator;

		GenerateMipChain2DShader* m_GenerateMipChainShader;
		RHI::PipelineState m_GenerateMipChainPSO;
		std::vector<RHI::ShaderResourceView> m_SRVs;
		std::vector<RHI::UnorderedAccessView> m_UAVs;
	};
}
#endif