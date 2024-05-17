#ifndef _AT_RENDER_SYSTEM_FRAME_GRAPH_BUILDER_H_
#define _AT_RENDER_SYSTEM_FRAME_GRAPH_BUILDER_H_
#define COMMAND_LISTS_PER_POOL 10
#define CBUFFER_PER_POOL 4500
#include "../RHI/RHI.h"
#include "RenderPass.h"
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <assert.h>
#include <vector>
#include <unordered_set>
#include "GPUShader.h"
#include "GPUConstantBuffer.h"
#include "FrameGraphRHIResourceAllocator.h"
#include "../JobSystem/JobSystem.h"

namespace AT {

	struct FrameGraphRenderThreadCommandObjects {
		~FrameGraphRenderThreadCommandObjects() {
			GraphicsCommandListQueue.clear();
			ComputeCommandListQueue.clear();
			for (uint32_t i = 0u; i < COMMAND_LISTS_PER_POOL; ++i) {
				delete GraphicsCommandListPool[i];
				delete ComputeCommandListPool[i];
			}
			delete GraphicsCommandAllocator;
			delete ComputeCommandAllocator;
		}
		void Reset() {
			GraphicsCommandAllocator->Reset();
			ComputeCommandAllocator->Reset();
			GraphicsCommandListQueue.clear();
			ComputeCommandListQueue.clear();
			for (uint32_t i = 0u; i < COMMAND_LISTS_PER_POOL; ++i) {
				GraphicsCommandListQueue.push_back(GraphicsCommandListPool[i]);
				ComputeCommandListQueue.push_back(ComputeCommandListPool[i]);
			}
		}

		RHI::CommandList GetCommandList(FrameGraphRenderPass::PIPELINE_TYPE command_type) {
			RHI::CommandList command_list = RHI_NULL_HANDLE;
			if (command_type == FrameGraphRenderPass::PIPELINE_TYPE::GRAPHICS) {
				if (!GraphicsCommandListQueue.empty()) {
					command_list = GraphicsCommandListQueue.front();
					GraphicsCommandListQueue.pop_front();
				}
			} else {
				if (!ComputeCommandListQueue.empty()) {
					command_list = ComputeCommandListQueue.front();
					ComputeCommandListQueue.pop_front();
				}
			}
			return command_list;
		}

		RHI::CommandAllocator GraphicsCommandAllocator;
		RHI::CommandAllocator ComputeCommandAllocator;
		std::array<RHI::CommandList, COMMAND_LISTS_PER_POOL> GraphicsCommandListPool;
		std::array<RHI::CommandList, COMMAND_LISTS_PER_POOL> ComputeCommandListPool;
		std::deque<RHI::CommandList> GraphicsCommandListQueue;
		std::deque<RHI::CommandList> ComputeCommandListQueue;
	};
	
	template <size_t buffer_size, size_t capacity> struct FrameGraphCBufferPool {
		template <typename T, size_t capacity>
		class RingBuffer
		{
		public:
			// Get an item if there are any
			//  Returns true if succesful
			//  Returns false if there are no items
			~RingBuffer() {
				for (uint32_t i = 0u; i < capacity; ++i) {
					delete m_Data[i];
				}
			}
			void Set(uint64_t index, T value) {
				m_Data[index] = value;
			}

			inline T Get() {
				T return_value;
				m_Lock.lock();
				return_value = m_Data[m_Tail];
				m_Tail = (m_Tail + 1) % capacity;
				m_Lock.unlock();
				return return_value;
			}

		private:
			T m_Data[capacity];
			size_t m_Head = 0;
			size_t m_Tail = 0;
			std::mutex m_Lock;
		};

		FrameGraphCBufferPool(FrameGraphRHIResourceAllocator& rhi_resource_allocator, RHI::Device device) {
			for (uint32_t i = 0; i < capacity; ++i) {
				BufferPool[i] = rhi_resource_allocator.CreateUploadBuffer(buffer_size);
				GPUConstantBuffer* cbuffer = new GPUConstantBuffer(device, BufferPool[i]);
				CBufferQueue.Set(i, cbuffer);
			}
		}

		~FrameGraphCBufferPool() {
			for (uint32_t i = 0; i < capacity; ++i) {

			}
		}
		RingBuffer<GPUConstantBuffer*, capacity> CBufferQueue;
		std::array< RHI::Buffer, capacity> BufferPool;
	};

	inline bool operator==(const RHI::TextureDescription& description1, const RHI::TextureDescription& description2) {
		return
			(description1.TextureType == description2.TextureType) &&
			(description1.Format == description2.Format) &&
			(description1.Width == description2.Width) &&
			(description1.Height == description2.Height) &&
			(description1.DepthOrArray == description2.DepthOrArray) &&
			(description1.MipLevels == description2.MipLevels) &&
			(description1.UsageFlags == description2.UsageFlags);
	};

	class FrameGraphBuilder {
	public:
		FrameGraphBuilder(RHI::Device device, RHI::CommandList command_list);

		~FrameGraphBuilder() {
			Reset();
			for (uint32_t i = 0u; i < m_RenderThreadCommandObjects.size(); ++i) {
				delete m_RenderThreadCommandObjects[i];
			}

			/*for (uint32_t i = 0u; i < CBUFFER_PER_POOL; ++i) {
				delete m_cbuffer_pool_2048[i];
				delete m_cbuffer_pool_256[i];
			}*/
			delete m_Fence;
			m_RHIResourceAllocator.Clear();
		}

		void Reset() {
			for (uint32_t i = 0u; i < m_SRVs.size(); ++i) {
				delete m_SRVs[i];
			}
			m_SRVs.clear();

			for (uint32_t i = 0u; i < m_RTVs.size(); ++i) {
				delete m_RTVs[i];
			}
			m_RTVs.clear();

			for (uint32_t i = 0u; i < m_DSVs.size(); ++i) {
				delete m_DSVs[i];
			}
			m_DSVs.clear();

			for (uint32_t i = 0u; i < m_UAVs.size(); ++i) {
				delete m_UAVs[i];
			}
			m_UAVs.clear();

			
			for (auto texture : m_Textures) {
				m_texture_cache.push_back(texture);
			}
			m_Textures.clear();

			for (uint32_t i = 0u; i < m_ExternalReadTextures.size(); ++i) {
				delete m_ExternalReadTextures[i];
			}
			m_ExternalReadTextures.clear();

			for (uint32_t i = 0u; i < m_ExternalWriteTextures.size(); ++i) {
				delete m_ExternalWriteTextures[i];
			}
			m_ExternalWriteTextures.clear();

			{
				std::scoped_lock locker(m_lock);
				for (uint32_t i = 0u; i < m_descriptor_tables.size(); ++i) {
					delete m_descriptor_tables[i];
				}
				m_descriptor_tables.clear();
			}

			for (uint32_t i = 0u; i < m_shader_parameter_groups.size(); ++i) {
				delete m_shader_parameter_groups[i];
			}
			m_shader_parameter_groups.clear();

			for (uint32_t i = 0u; i < m_shader_parameters.size(); ++i) {
				delete m_shader_parameters[i];
			}
			m_shader_parameters.clear();

			for (uint32_t i = 0u; i < m_RenderPasses.size(); ++i) {
				delete m_RenderPasses[i];
				delete m_RHIRenderPasses[i];
			}
			m_RHIRenderPasses.clear();
			m_RenderPasses.clear();

			for (uint32_t i = 0u; i < m_RenderThreadCommandObjects.size(); ++i) {
				m_RenderThreadCommandObjects[i]->Reset();
			}

			m_RenderPassIDToDependencyLevels.clear();
			m_DependencyLevels.clear();
			m_sorted_order.clear();
			m_RenderPassNameToRenderPassID.clear();
		}

		template<typename F> void AddRenderPass(const std::string& render_pass_name, const FrameGraphRenderPassIO& render_pass_io, FrameGraphRenderPass::PIPELINE_TYPE pipeline_type, F&& execute_function) {
			assert(!m_RenderPassNameToRenderPassID.contains(render_pass_name));
			uint32_t id = m_RenderPasses.size();
			m_RenderPassNameToRenderPassID[render_pass_name] = id;
			m_RenderPasses.emplace_back(new FrameGraphLambdaRenderPass(render_pass_name, render_pass_io, pipeline_type, static_cast<F&&>(execute_function)));
		}

		template<typename T> T& AllocateShaderParameters() {
			std::scoped_lock lock(m_lock);
			T* parameter = new T{};
			m_shader_parameters.push_back(parameter);
			return *parameter;
		}

		template<typename T> std::vector<T*> AllocateShaderParameterGroup(RHI::RootSignature root_signature, uint32_t index, uint32_t count) {
			std::scoped_lock lock(m_lock);

			std::vector<T*> v = std::vector<T*>(count);

			for (uint32_t i = 0; i < count; ++i) {
				T* parameter_group = new T{};
				m_shader_parameter_groups.push_back(parameter_group);
				m_Device->AllocateDescriptorTable(root_signature, index, m_shader_parameter_groups.back()->m_descriptor_table);
				m_descriptor_tables.push_back(m_shader_parameter_groups.back()->m_descriptor_table);
				v[i] = parameter_group;
			}

			return v;
		}

		template<typename T> GPUConstantBuffer* AllocateConstantBuffer() {
			assert(sizeof(T) <= 2048);
			if (sizeof(T) <= 256) {
				return m_256_pool->CBufferQueue.Get();
			}
			else if (sizeof(T) <= 2048) {
				return m_2048_pool->CBufferQueue.Get();
			}

		}

		FrameGraphTextureRef RegisterWriteTexture(RHI::Texture texture) {
			FrameGraphTextureDescription description;
			description.Format = texture->GetDescription().Format;
			description.Width = texture->GetDescription().Width;
			description.Height = texture->GetDescription().Height;
			description.ArraySize = texture->GetDescription().DepthOrArray;
			return m_ExternalWriteTextures.emplace_back(new FrameGraphTexture{ .Description = description, .RHIHandle = texture });
		}

		FrameGraphTextureRef RegisterReadTexture(RHI::Texture texture, RHI::TextureState state) {
			FrameGraphTextureDescription description;
			description.Format = texture->GetDescription().Format;
			description.Width = texture->GetDescription().Width;
			description.Height = texture->GetDescription().Height;
			description.ArraySize = texture->GetDescription().DepthOrArray;
			FrameGraphTextureRef new_texture = new FrameGraphTexture{};
			new_texture->Description = description;
			new_texture->RHIHandle = texture;
			new_texture->FinalState = state;
			return m_ExternalReadTextures.emplace_back(new_texture);
		}

		FrameGraphTextureRef CreateTexture(const FrameGraphTextureDescription& description) {
			return m_Textures.emplace_back(new FrameGraphTexture{ .Description = description });
		}

		FrameGraphRTVRef CreateRTV(FrameGraphTextureRef texture, const FrameGraphRTVDescription& description) {
			texture->UsageFlags = (texture->UsageFlags | RHI::TextureUsageFlag::RENDER_TARGET);
			return m_RTVs.emplace_back(new FrameGraphRTV{ .Description = description, .TextureRef = texture });
		}

		FrameGraphSRVRef CreateSRV(FrameGraphTextureRef texture, const FrameGraphSRVDescription& description) {
			texture->UsageFlags = (texture->UsageFlags | RHI::TextureUsageFlag::SHADER_RESOURCE);
			return m_SRVs.emplace_back(new FrameGraphSRV{ .Description = description, .TextureRef = texture });
		}

		FrameGraphDSVRef CreateDSV(FrameGraphTextureRef texture, const FrameGraphDSVDescription& description) {
			texture->UsageFlags = (texture->UsageFlags | RHI::TextureUsageFlag::DEPTH_STENCIL);
			return m_DSVs.emplace_back(new FrameGraphDSV{ .Description = description, .TextureRef = texture });
		}

		FrameGraphUAVRef CreateUAV(FrameGraphTextureRef texture, const FrameGraphUAVDescription& description) {
			texture->UsageFlags = (texture->UsageFlags | RHI::TextureUsageFlag::UNORDERED_ACCESS);
			return m_UAVs.emplace_back(new FrameGraphUAV{ .Description = description, .TextureRef = texture });
		}

		void Compile();
		void Execute();
		RHI::Device GetDevice() const { return m_Device; }
	private:
		void TopologicalSortHelper(uint32_t i, std::vector<std::unordered_set<uint32_t>>& adjacency_list, std::vector<bool>& visited, std::vector<bool>& on_stack, std::vector<uint32_t>& sorted_list) {
			visited[i] = true;
			on_stack[i] = true;
			for (auto j : adjacency_list[i]) {
				if (!visited[j]) {
					TopologicalSortHelper(j, adjacency_list, visited, on_stack, sorted_list);
				}
			}
			sorted_list.push_back(i);
			on_stack[i] = false;
		}

		void TopologicalSort(std::vector<std::unordered_set<uint32_t>>& adjacency_list, std::vector<bool>& visited, std::vector<bool>& on_stack, std::vector<uint32_t>& sorted_list) {
			for (uint32_t i = 0; i < visited.size(); ++i) {
				if (!visited[i]) {
					TopologicalSortHelper(i, adjacency_list, visited, on_stack, sorted_list);
				}
			}
			std::reverse(sorted_list.begin(), sorted_list.end());
		}

		void CreateResources();

		RHI::Device m_Device;
		FrameGraphRHIResourceAllocator m_RHIResourceAllocator;
		RHI::CommandList m_CommandList;
		RHI::Fence m_Fence;
		uint64_t m_CurrentFenceValue;

		std::vector<FrameGraphRenderPass*> m_RenderPasses;
		std::unordered_map<std::string, uint32_t> m_RenderPassNameToRenderPassID;

		//Resources
		std::vector<RHI::RenderPass> m_RHIRenderPasses;
		std::vector<GPUShader::ShaderParameters*> m_shader_parameters;
		std::vector<GPUShader::ShaderParameterGroup*> m_shader_parameter_groups;
		std::vector<RHI::DescriptorTable> m_descriptor_tables;
		std::vector<FrameGraphTextureRef> m_Textures;
		std::vector<FrameGraphTextureRef> m_ExternalReadTextures;
		std::vector<FrameGraphTextureRef> m_ExternalWriteTextures;
		std::vector<FrameGraphSRVRef> m_SRVs;
		std::vector<FrameGraphRTVRef> m_RTVs;
		std::vector<FrameGraphDSVRef> m_DSVs;
		std::vector<FrameGraphUAVRef> m_UAVs;

		//Execution variables.
		std::vector<uint32_t> m_sorted_order;
		struct DependencyLevel {
			std::vector<RHI::ResourceBarrier> StartBarriers;
			std::vector<uint32_t> RenderPassIDs;
			uint64_t SignalGraphicsValue = 0ull;
			uint64_t SignalComputeValue = 0ull;
			int32_t GraphicsWaitForDependencyLevel = -1;
			int32_t ComputeWaitForDependencyLevel = -1;
			uint32_t GraphicsPassCount = 0u;
			uint32_t ComputePassCount = 0u;
		};
		std::vector<uint32_t> m_RenderPassIDToDependencyLevels;
		std::vector<DependencyLevel> m_DependencyLevels;

		//std::mutex m_lock;
		AT::JobSystem::SpinLock m_lock;

		//Caches
		std::list<FrameGraphTextureRef> m_texture_cache;
		FrameGraphCBufferPool<256, 4500>* m_256_pool;
		FrameGraphCBufferPool<2048, 500>* m_2048_pool;
		std::vector<FrameGraphRenderThreadCommandObjects*> m_RenderThreadCommandObjects;
	};
}
#endif