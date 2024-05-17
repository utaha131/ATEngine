#ifndef _RHI_VK_PIPELINE_STATE_H_
#define _RHI_VK_PIPELINE_STATE_H_
#include "../IPipelineState.h"
#include <vulkan/vulkan.h>
#include "VKRootSignature.h"

namespace RHI::VK {
	class VKPipelineState : public RHI::IPipelineState {
	public:
		VKPipelineState(const GraphicsPipelineStateDescription& description, VkDevice vk_device, VkPipeline vk_pipeline) :
			IPipelineState(description),
			m_VKPipeline(vk_pipeline),
			m_VKDevice(vk_device),
			m_VKPipelineBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS)
		{

		}

		VKPipelineState(const ComputePipelineStateDescription& description, VkDevice vk_device, VkPipeline vk_pipeline) :
			IPipelineState(description),
			m_VKDevice(vk_device),
			m_VKPipeline(vk_pipeline),
			m_VKPipelineBindPoint(VK_PIPELINE_BIND_POINT_COMPUTE) 
		{
		
		};

		~VKPipelineState() override {
			vkDestroyPipeline(m_VKDevice, m_VKPipeline, VK_NULL_HANDLE);
		}
		inline VkPipeline GetNative() const { return m_VKPipeline; }
		inline VkPipelineBindPoint GetBindPoint() const { return m_VKPipelineBindPoint; }
	private:
		VkPipelineBindPoint m_VKPipelineBindPoint;
		VkPipeline m_VKPipeline;
		VkDevice m_VKDevice;
	};
}
#endif