#ifndef _VK_DESCRIPTOR_HEAP_H_
#define _VK_DESCRIPTOR_HEAP_H_
#define POOL_SIZE 256
#include <vector>
#include <queue>
#include <array>
#include <windows.h>
#include <vulkan/vulkan.h>
#include "../IDescriptorTable.h"

namespace RHI::VK {
	class VKDescriptorTable : public IDescriptorTable {
	public:
		VKDescriptorTable(std::queue<uint64_t>& free_queue, uint64_t index, VkDescriptorSet vk_descriptor_set) :
			m_FreeQueue(free_queue),
			m_Index(index),
			m_VKDescriptorSet(vk_descriptor_set)
		{

		}
		~VKDescriptorTable() override {
			m_FreeQueue.push(m_Index);
		}
		VkDescriptorSet GetNative() const { return m_VKDescriptorSet; }
	private:
		uint64_t m_Index;
		VkDescriptorSet m_VKDescriptorSet;
		std::queue<uint64_t>& m_FreeQueue;
	};

	class VKDescriptorHeap {
	public:
		VKDescriptorHeap(VkDevice vk_device, const std::vector<VkDescriptorPoolSize>& vk_descriptor_pool_sizes, VkDescriptorSetLayout vk_descriptor_set_layout);
		~VKDescriptorHeap();
		VKDescriptorTable* AllocateTable() {
			if (m_FreeQueue.empty()) {
				AllocatePool();
				AllocateDescriptors();
			}
			uint64_t index = m_FreeQueue.front();
			m_FreeQueue.pop();
			return new VKDescriptorTable(m_FreeQueue, index, m_VKDescriptors[index]);
		}
	protected:
		void AllocatePool();
		void AllocateDescriptors() {
			VkDescriptorSetAllocateInfo allocate_info = {
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.descriptorPool = m_VKDescriptorPools.back(),
					.descriptorSetCount = POOL_SIZE,
					.pSetLayouts = m_VKDescriptorSetLayouts.data(),
			};
			std::vector <VkDescriptorSet> descriptors = std::vector <VkDescriptorSet>(256);
			vkAllocateDescriptorSets(m_VKDevice, &allocate_info, descriptors.data());

			for (uint32_t i = 0u; i < POOL_SIZE; ++i) {
				m_VKDescriptors.push_back(descriptors[i]);
			}

			uint32_t start = ((m_VKDescriptors.size() / POOL_SIZE) - 1) * POOL_SIZE;
			for (uint32_t i = 0u; i < POOL_SIZE; ++i) {
				m_FreeQueue.push(start + i);
			}
		}
	private:
		VkDevice m_VKDevice;
		std::array<VkDescriptorSetLayout, POOL_SIZE> m_VKDescriptorSetLayouts;
		VkDescriptorPoolCreateInfo m_VKDescriptorPoolCreateInfo;
		std::vector<VkDescriptorPool> m_VKDescriptorPools;
		std::vector<VkDescriptorSet> m_VKDescriptors;
		std::queue<uint64_t> m_FreeQueue;
		std::vector<VkDescriptorPoolSize> m_VKDescriptorPoolSizes;
	};
}
#endif