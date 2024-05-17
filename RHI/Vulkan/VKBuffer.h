#ifndef _RHI_VK_BUFFER_H_
#define _RHI_VK_BUFFER_H_
#include "../IBuffer.h"
#include <vulkan/vulkan.h>
#include "VKResourceHeap.h"

namespace RHI::VK {
	class VKBuffer : public RHI::IBuffer {
	public:
		VKBuffer(const BufferDescription& description, VkDevice vk_device, VKResourceHeap& resource_heap, uint64_t offset, bool is_dedicated, VkBuffer vk_buffer);
		~VKBuffer() override;
		void Map() override;
		void CopyData(uint64_t offset, const void* data, uint64_t size) override;
		void Unmap() override;
		inline VkBuffer GetNative() const { return m_VKBuffer; }
	private:
		VkBuffer m_VKBuffer;
		VkDevice m_VKDevice;
		VKResourceHeap* m_ResourceHeap;
		uint64_t m_Offset;
		bool m_IsDedicated;
	};

	constexpr VkAccessFlagBits VKConvertBufferStateToAccessFlag(BufferState buffer_state) {
		switch (buffer_state) {
		case BufferState::COMMON:
			return (VkAccessFlagBits)0;

		case BufferState::UNORDERED_ACCESS:
			return (VkAccessFlagBits)(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);

		case BufferState::COPY_DEST:
			return VK_ACCESS_TRANSFER_WRITE_BIT;

		case BufferState::COPY_SOURCE:
			return VK_ACCESS_TRANSFER_READ_BIT;
#undef GENERIC_READ
		case BufferState::GENERIC_READ:
			return VK_ACCESS_MEMORY_READ_BIT;

			//Buffer Exclusives.
		case BufferState::VERTEX_BUFFER:
			return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;

		case BufferState::CONSTANT_BUFFER:
			return VK_ACCESS_UNIFORM_READ_BIT;

		case BufferState::INDEX_BUFFER:
			return VK_ACCESS_INDEX_READ_BIT;

		}
	}

	inline VkBufferCreateInfo VKConvertBufferDescription(const RHI::BufferDescription& description) {
		VkBufferCreateInfo vk_buffer_create_info = {};
		vk_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vk_buffer_create_info.size = description.Size;
		vk_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		vk_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (has_flag(description.UsageFlags, RHI::BufferUsageFlag::UNIFORM_BUFFER)) {
			vk_buffer_create_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (has_flag(description.UsageFlags, RHI::BufferUsageFlag::UNORDERED_ACCESS)) {
			vk_buffer_create_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		return vk_buffer_create_info;
	}
}
#endif