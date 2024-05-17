#ifndef _RHI_VK_SHADER_H_
#define _RHI_VK_SHADER_H_
#include "../IShader.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
    class VKShader : public IShader {
    public:
        VKShader(const ShaderDescription& description, VkDevice vk_device, VkShaderModule vk_shader_module) :
            RHI::IShader(description),
            m_VKShaderModule(vk_shader_module),
            m_VKDevice(vk_device)
        {

        }
        ~VKShader() override {
            vkDestroyShaderModule(m_VKDevice, m_VKShaderModule, VK_NULL_HANDLE);
        }

        inline VkPipelineShaderStageCreateInfo GetNative() {
            VkPipelineShaderStageCreateInfo native_shader = {};
            native_shader.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            switch (m_Description.ShaderType) {
            case RHI::ShaderType::VERTEX:
                native_shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case RHI::ShaderType::PIXEL:
                native_shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            case RHI::ShaderType::COMPUTE:
                native_shader.stage = VK_SHADER_STAGE_COMPUTE_BIT;
                break;
            }
            native_shader.module = m_VKShaderModule;
            native_shader.pName = m_Description.EntryPoint.c_str();
            return native_shader;
        }
    private:
        VkShaderModule m_VKShaderModule;
        VkDevice m_VKDevice;
    };
}
#endif