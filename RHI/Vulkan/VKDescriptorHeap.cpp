#include "VKDescriptorHeap.h"

namespace RHI::VK {
	VKDescriptorHeap::VKDescriptorHeap(VkDevice vk_device, const std::vector<VkDescriptorPoolSize>& vk_descriptor_pool_sizes, VkDescriptorSetLayout vk_descriptor_set_layout) :
		m_VKDescriptorPools(std::vector<VkDescriptorPool>()),
		m_VKDevice(vk_device),
		m_VKDescriptors(std::vector<VkDescriptorSet>(POOL_SIZE, VK_NULL_HANDLE)),
		m_VKDescriptorPoolSizes(vk_descriptor_pool_sizes)
	{
		m_VKDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		m_VKDescriptorPoolCreateInfo.maxSets = POOL_SIZE;
		m_VKDescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(m_VKDescriptorPoolSizes.size());
		m_VKDescriptorPoolCreateInfo.pPoolSizes = m_VKDescriptorPoolSizes.data();

	/* = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = POOL_SIZE,
			.poolSizeCount = static_cast<uint32_t>(m_pool_sizes.size()),
			.pPoolSizes = m_pool_sizes.data(),
		};*/
		for (uint32_t i = 0u; i < POOL_SIZE; ++i) {
			m_VKDescriptorSetLayouts[i] = vk_descriptor_set_layout;
		}
		AllocatePool();
		AllocateDescriptors();
		/*for (uint32_t i = 0u; i < POOL_SIZE; ++i) {
			m_VKDescriptorSetLayouts[i] = vk_descriptor_set_layout;
		}

		VkDescriptorSetAllocateInfo allocate_info = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_pools.back(),
			.descriptorSetCount = POOL_SIZE,
			.pSetLayouts = m_layouts.data(),
		};

		vkAllocateDescriptorSets(m_device, &allocate_info, m_descriptors.data());
		uint32_t start = (m_descriptors.size() / POOL_SIZE) - 1;
		for (uint32_t i = 0; i < POOL_SIZE; ++i) {
			m_free_indices.push(start + i);
		}*/
	}

	VKDescriptorHeap::~VKDescriptorHeap() {
		for (unsigned int i = 0; i < m_VKDescriptorPools.size(); ++i) {
			vkDestroyDescriptorPool(m_VKDevice, m_VKDescriptorPools[i], VK_NULL_HANDLE);
		}

		vkDestroyDescriptorSetLayout(m_VKDevice, m_VKDescriptorSetLayouts[0], VK_NULL_HANDLE);
	}

	void VKDescriptorHeap::AllocatePool() {
		VkDescriptorPool new_pool;
		vkCreateDescriptorPool(m_VKDevice, &m_VKDescriptorPoolCreateInfo, VK_NULL_HANDLE, &new_pool);
		m_VKDescriptorPools.push_back(new_pool);
	}
}