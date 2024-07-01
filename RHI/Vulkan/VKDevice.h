#ifndef _RHI_VK_DEVICE_H_
#define _RHI_VK_DEVICE_H_

#include "../IDevice.h"
#include <vulkan/vulkan.h>
#include <fstream>
#include <unordered_map>
#include <deque>

#include "VKGraphics.h"
#include "VKTexture.h"
#include "VKConstantBufferView.h"
#include "VKDepthStencilView.h"
#include "VKRenderTargetView.h"
#include "VKShaderResourceView.h"
#include "VKUnorderedAccessView.h"
#include "VKShader.h"
#include "VKRootSignature.h"
#include "VKFrameBuffer.h"
#include "VKCommandAllocator.h"
#include "VKCommandList.h"
#include "VKResourceHeap.h"
#include "VKBuffer.h"

#include "VKCommandQueue.h"
#include "VKDescriptorHeap.h"
#include "VKFence.h"

namespace RHI::VK {
	struct VKDescriptorSetLayoutCacheInfo {
		RootDescriptorTable table;
		std::vector<StaticSamplerDescription> static_samplers;
	};

	template <class T>
	inline void hash_combine(std::size_t& s, const T& v)
	{
		std::hash<T> h;
		s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
	}

	static inline auto DescriptorTableHashFunction = [](const VKDescriptorSetLayoutCacheInfo& table) {
		size_t hash = 0;
		hash_combine(hash, table.table.Ranges.size());
		for (uint32_t i = 0u; i < table.table.Ranges.size(); ++i) {
			hash_combine(hash, table.table.Ranges[i].RangeType);
			hash_combine(hash, table.table.Ranges[i].DescriptorCount);
			hash_combine(hash, table.table.Ranges[i].RegisterSpace);
		}

		hash_combine(hash, table.static_samplers.size());
		for (uint32_t i = 0u; i < table.static_samplers.size(); ++i) {
			hash_combine(hash, table.static_samplers[i].Filter);
		}
		return hash;
	};

	static inline auto DescriptorTableEqualFunction = [](const VKDescriptorSetLayoutCacheInfo& table1, const VKDescriptorSetLayoutCacheInfo& table2) {
		if (table1.table.Ranges.size() != table2.table.Ranges.size()) {
			return false;
		}
		for (uint32_t i = 0u; i < table1.table.Ranges.size(); ++i) {
			if (table1.table.Ranges[i].RangeType != table2.table.Ranges[i].RangeType) {
				return false;
			}
			if (table1.table.Ranges[i].DescriptorCount != table2.table.Ranges[i].DescriptorCount) {
				return false;
			}
			if (table1.table.Ranges[i].RegisterSpace != table2.table.Ranges[i].RegisterSpace) {
				return false;
			}
		}

		if (table1.static_samplers.size() != table2.static_samplers.size()) {
			return false;
		}

		for (uint32_t i = 0u; i < table1.static_samplers.size(); ++i) {
			if (table1.static_samplers[i].Filter != table2.static_samplers[i].Filter) {
				return false;
			}
		}
		return true;
	};

	static inline auto static_sampler_hash_function = [](const StaticSamplerDescription& description) {
		std::size_t hash = 0;
		hash_combine(hash, description.Filter);
		hash_combine(hash, description.AddressU);
		hash_combine(hash, description.AddressV);
		hash_combine(hash, description.AddressW);
		hash_combine(hash, description.MinLOD);
		hash_combine(hash, description.MaxLOD);
		hash_combine(hash, description.MaxAnisotropy);
		hash_combine(hash, description.ShaderRegister);
		hash_combine(hash, description.RegisterSpace);
		return hash;
	};

	static inline auto static_sampler_equality_function = [](const StaticSamplerDescription& description1, const StaticSamplerDescription& description2) {
		return (description1.Filter == description2.Filter) &&
			(description1.AddressU == description2.AddressU) &&
			(description1.AddressV == description2.AddressV) &&
			(description1.AddressW == description2.AddressW) &&
			(description1.MinLOD == description2.MinLOD) &&
			(description1.MaxLOD == description2.MaxLOD) &&
			(description1.MaxAnisotropy == description2.MaxAnisotropy) &&
			(description1.ShaderRegister == description2.ShaderRegister) &&
			(description1.RegisterSpace == description2.RegisterSpace);
	};

	class VKDevice : public IDevice {
		friend class VKRenderPlatform;
	public:
		VKDevice(VkDevice vk_device, VkPhysicalDevice vk_physical_device);
		~VKDevice() override;

		//Object Creation Interface.
		RHI::Result CreateResourceHeap(const RHI::ResourceHeapDescription& description, RHI::ResourceHeap& resource_heap) const override;
		RHI::Result CreateBuffer(RHI::ResourceHeap resource_heap, uint64_t offset, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const override;
		RHI::Result CreateCommittedBuffer(RHI::ResourceHeapType resource_heap_type, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const override;
		RHI::Result CreateTexture(RHI::ResourceHeap resource_heap, uint64_t offset, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const override;
		RHI::Result CreateCommittedTexture(RHI::ResourceHeapType resource_heap_type, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const override;
		RHI::Result CreateRootSignature(const RHI::RootSignatureDescription& description, RHI::RootSignature& root_signature) override;
		RHI::Result CreateShader(const RHI::ShaderDescription& description, RHI::Shader& shader) const override;
		RHI::Result CreateRenderPass(const RHI::RenderPassDescription& description, RHI::RenderPass& render_pass) const override;
		RHI::Result CreateFence(uint64_t initial_value, RHI::Fence& fence) const override;
		RHI::Result CreateFrameBuffer(const RHI::FrameBufferDescription& description, RHI::FrameBuffer& frame_buffer) const override;
		RHI::Result CreateGraphicsPipelineState(const RHI::GraphicsPipelineStateDescription& description, RHI::PipelineState& pipeline_state) const override;
		RHI::Result CreateComputePipelineState(const RHI::ComputePipelineStateDescription& description, RHI::PipelineState& pipeline_state) const override;
		RHI::Result CreateConstantBufferView(const RHI::ConstantBufferViewDescription& description, RHI::ConstantBufferView& constant_buffer_view) override;
		RHI::Result CreateDepthStencilView(const RHI::Texture texture, const RHI::DepthStencilViewDescription& description, RHI::DepthStencilView& depth_stencil_view) override;
		RHI::Result CreateRenderTargetView(const RHI::Buffer buffer, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) override;
		RHI::Result CreateRenderTargetView(const RHI::Texture texture, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) override;
		RHI::Result CreateSampler(const RHI::SamplerDescription& description, RHI::Sampler& sampler) override;
		RHI::Result CreateShaderResourceView(const RHI::Buffer buffer, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) override;
		RHI::Result CreateShaderResourceView(const RHI::Texture texture, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) override;
		RHI::Result CreateUnorderedAccessView(const RHI::Buffer buffer, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) override;
		RHI::Result CreateUnorderedAccessView(const RHI::Texture texture, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) override;
		RHI::Result CreateCommandAllocator(RHI::CommandType command_type, RHI::CommandAllocator& command_allocator) const override;
		RHI::Result CreateCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, RHI::CommandList& command_list) const override;

		//Execution Interface.
		void ExecuteCommandList(RHI::CommandType command_type, uint32_t command_list_count, const RHI::CommandList* p_command_lists) override;
		void SignalQueue(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) override;
		void HostWait(RHI::Fence fence, uint64_t fence_value) override;
		void QueueWait(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) override;

		//Other Interfaces.
		RHI::AllocationInfo GetResourceAllocationInfo(const RHI::BufferDescription& buffer_description) const override;
		RHI::AllocationInfo GetResourceAllocationInfo(const RHI::TextureDescription& texture_description) const override;
		uint32_t GetTexturePitchAlignment() const override;

		void AllocateDescriptorTable(const RHI::RootSignature root_signature, uint32_t parameter_index, RHI::DescriptorTable& table) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ConstantBufferView* p_cbv) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::Sampler* p_sampler) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ShaderResourceView* p_srv) override;
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::UnorderedAccessView* p_uav) override;

		//RT
		void CreateRayTracingPipelineState(const RHI::RayTracingPipelineStateDescription& description, RHI::IRayTracingPipeline*& pipeline) const override {
			VkRayTracingPipelineCreateInfoKHR vk_ray_tracing_pipeline_create_info = {};
			vk_ray_tracing_pipeline_create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
			vk_ray_tracing_pipeline_create_info.pNext = VK_NULL_HANDLE;
			vk_ray_tracing_pipeline_create_info;
			vk_ray_tracing_pipeline_create_info.maxPipelineRayRecursionDepth = description.MaxTraceRecursionDepth;
		}

		void CreateRayTracingBottomLevelAccelerationStructure(const RHI::Buffer buffer, IRayTracingBottomLevelAccelerationStructure*& bottom_level_acceleration_structure) const override {

		}

		void CreateRayTracingTopLevelAccelerationStructure(const RHI::Buffer buffer, IRayTracingTopLevelAccelerationStructure*& top_level_acceleration_structure) const override {

		}

		void CreateRayTracingInstanceBuffer(RHI::Buffer buffer, uint64_t capacity, IRayTracingInstanceBuffer*& instance_buffer) const override {

		}

		void GetRayTracingBottomLevelAccelerationStructureMemoryInfo(const RayTracingBottomLevelAccelerationStructureDescription bottom_level_acceleration_structure_description, RayTracingAccelerationStructureMemoryInfo& memory_info) const override {
		}

		void GetRayTracingTopLevelAccelerationStructureMemoryInfo(const RayTracingTopLevelAccelerationStructureDescription top_level_acceleration_structure_description, RayTracingAccelerationStructureMemoryInfo& memory_info) const override {
		}
		
		void WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, IRayTracingTopLevelAccelerationStructure** p_top_level_acceleration_structures) override {
		}

		VKCommandQueue* GetQueue(RHI::CommandType command_type) {
			return m_VKCommandQueues[(uint32_t)command_type];
		}

		inline VkDevice GetNative() const { return m_VkDevice; };

		std::unordered_map<VkDescriptorSetLayout, VKDescriptorHeap*> m_DescriptorHeaps;
		std::unordered_map<VKDescriptorSetLayoutCacheInfo, VkDescriptorSetLayout, decltype(DescriptorTableHashFunction), decltype(DescriptorTableEqualFunction)> m_DescriptorLayoutCache;

	private:
		VkDevice m_VkDevice;
		VKCommandQueue* m_VKCommandQueues[(uint32_t)RHI::CommandType::NUM_TYPES];
		VkPhysicalDevice m_VkPhysicalDevice;
		uint32_t m_command_queue_family_indices[(uint32_t)RHI::CommandType::NUM_TYPES];
		std::unordered_map<RHI::ResourceHeapType, uint32_t> m_ResourceHeapTypes;
		std::unordered_map<RHI::StaticSamplerDescription, VkSampler, decltype(static_sampler_hash_function), decltype(static_sampler_equality_function)> m_VkStaticSamplerCache;

		VkDescriptorSetLayout VKConvertRootDescriptorTableToSetLayout(const RootDescriptorTable& root_descriptor_table, const std::vector<StaticSamplerDescription>& static_samplers) {
			std::vector<VkDescriptorSetLayoutBinding> bindings = std::vector<VkDescriptorSetLayoutBinding>();
			std::unordered_map<VkDescriptorType, uint32_t> pool_size_map = std::unordered_map<VkDescriptorType, uint32_t>();
			uint32_t binding_index = 0u;
			for (uint32_t j = 0u; j < root_descriptor_table.Ranges.size(); ++j) {
				const RHI::RootDescriptorTableRange& range = root_descriptor_table.Ranges[j];
				if (range.IsArray) {
					VkDescriptorSetLayoutBinding native_range = {
							.binding = binding_index,
							.descriptorType = VKConvertDescriptorRangeType(range.RangeType),
							.descriptorCount = range.DescriptorCount,
							.stageFlags = VK_SHADER_STAGE_ALL
					};
					bindings.push_back(native_range);
					binding_index += range.DescriptorCount;
				}
				else {
					for (uint32_t k = 0u; k < range.DescriptorCount; ++k) {
						VkDescriptorSetLayoutBinding native_range = {
							.binding = binding_index,
							.descriptorType = VKConvertDescriptorRangeType(range.RangeType),
							.descriptorCount = 1,
							.stageFlags = VK_SHADER_STAGE_ALL
						};
						bindings.push_back(native_range);
						++binding_index;
					}
				}
			}
			std::vector<VkSampler> immutable_samplers = std::vector<VkSampler>(static_samplers.size());

			for (uint32_t i = 0u; i < static_samplers.size(); ++i) {
				if (m_VkStaticSamplerCache.find(static_samplers[i]) == m_VkStaticSamplerCache.end()) {
					VkSamplerCreateInfo create_info = VKConvertStaticSamplerDescription(static_samplers[i]);
					vkCreateSampler(m_VkDevice, &create_info, VK_NULL_HANDLE, &immutable_samplers[i]);
					m_VkStaticSamplerCache[static_samplers[i]] = immutable_samplers[i];
				}
				else {
					immutable_samplers[i] = m_VkStaticSamplerCache[static_samplers[i]];
				}
				VkDescriptorSetLayoutBinding sampler_binding = {};
				sampler_binding.binding = binding_index;
				sampler_binding.descriptorCount = 1;
				sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				sampler_binding.stageFlags = VK_SHADER_STAGE_ALL;
				sampler_binding.pImmutableSamplers = &immutable_samplers[i];
				bindings.push_back(sampler_binding);
				++binding_index;
			}

			VkDescriptorSetLayoutCreateInfo descriptor_set_layout_description = {
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = static_cast<uint32_t>(bindings.size()),
				.pBindings = bindings.data(),
			};
			descriptor_set_layout_description.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV;
			VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
			VKDescriptorSetLayoutCacheInfo cache_info;
			cache_info.table = root_descriptor_table;
			cache_info.static_samplers = static_samplers;
			if (m_DescriptorLayoutCache.find(cache_info) == m_DescriptorLayoutCache.end()) {
				vkCreateDescriptorSetLayout(m_VkDevice, &descriptor_set_layout_description, VK_NULL_HANDLE, &descriptor_set_layout);
				m_DescriptorLayoutCache[cache_info] = descriptor_set_layout;
			}
			else {
				descriptor_set_layout = m_DescriptorLayoutCache[cache_info];
			}

			//create new allocator for table.
			std::vector<VkDescriptorPoolSize> pool_sizes = std::vector<VkDescriptorPoolSize>();
			for (std::pair<VkDescriptorType, unsigned int> pair : pool_size_map) {
				VkDescriptorPoolSize pool_size = {
					.type = pair.first,
					.descriptorCount = pair.second,
				};
				pool_sizes.push_back(pool_size);
			}
			if (m_DescriptorHeaps.find(descriptor_set_layout) == m_DescriptorHeaps.end()) {
				m_DescriptorHeaps[descriptor_set_layout] = new VKDescriptorHeap(m_VkDevice, pool_sizes, descriptor_set_layout);
			}
			return descriptor_set_layout;
		}
	};
}
#endif