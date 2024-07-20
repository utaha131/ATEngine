#ifndef _AT_RENDER_SYSTEM_GPU_SHADER_MANAGER_H_
#define _AT_RENDER_SYSTEM_GPU_SHADER_MANAGER_H_
#include <functional>
#include "GPUShader.h"
#include "../Util/HashUtil.h"
#include <mutex>

static auto rhi_shader_description_equality_function = [](const RHI::ShaderDescription& description1, const RHI::ShaderDescription& description2) {
	return (description1.ShaderType == description2.ShaderType) && (description1.SourcePath == description2.SourcePath);
};

static auto rhi_shader_description_hash_function = [](const RHI::ShaderDescription& description){
	std::size_t hash = 0;
	AT::Util::Hash::hash_combine(hash, description.ShaderType);
	AT::Util::Hash::hash_combine(hash, description.SourcePath);
	return hash;
};

namespace AT {
	class GPUShaderManager {
	public:
		GPUShaderManager(RHI::Device device, RHI::RenderAPI api) :
			m_device(device),
			m_api(api)
		{

		}

		~GPUShaderManager() {
			for (auto pair : m_rhi_shader_cache) {
				delete pair.second;
			}
		}

		GPUShader* RegisterShader(const std::string& name, GPUShader* shader) {
			if (m_shader_map.find(name) == m_shader_map.end()) {
				m_shader_map[name] = shader;
			}
		}

		GPUShader* GetShader(const std::string& name) {
			if (m_shader_map.find(name) != m_shader_map.end()) {
				return m_shader_map[name];
			}
			return nullptr;
		}

		RHI::Shader LoadRHIShader(const std::string& path, RHI::ShaderType shader_type) {
			if (m_api == RHI::RenderAPI::DIRECTX) {
				RHI::ShaderDescription description = ShaderDescriptionFromNameDX(path, shader_type);
				if (m_rhi_shader_cache.find(description) == m_rhi_shader_cache.end()) {
					RHI::Shader rhi_shader;
					m_device->CreateShader(description, rhi_shader);
					m_rhi_shader_cache[description] = rhi_shader;
				}
				assert(description.SourcePath == m_rhi_shader_cache[description]->GetDescription().SourcePath);
				return m_rhi_shader_cache[description];
			} else {
				RHI::ShaderDescription description = ShaderDescriptionFromNameVK(path, shader_type);
				if (m_rhi_shader_cache.find(description) == m_rhi_shader_cache.end()) {
					RHI::Shader rhi_shader;
					m_device->CreateShader(description, rhi_shader);
					m_rhi_shader_cache[description] = rhi_shader;
				}
				return m_rhi_shader_cache[description];
			}
		}
	private:
		RHI::ShaderDescription ShaderDescriptionFromNameDX(const std::string& name, RHI::ShaderType shader_type) {
			RHI::ShaderDescription description;
			description.ShaderType = shader_type;
			description.SourcePath = "./shaders/hlsl/" + name + ".hlsl";
			switch (shader_type) {
			case RHI::ShaderType::VERTEX:
				description.EntryPoint = "VS";
				break;
			case RHI::ShaderType::PIXEL:
				description.EntryPoint = "PS";
				break;
			case RHI::ShaderType::COMPUTE:
				description.EntryPoint = "main";
				break;
			}
			return description;
		}

		RHI::ShaderDescription ShaderDescriptionFromNameVK(const std::string& name, RHI::ShaderType shader_type) {
			RHI::ShaderDescription description;
			description.ShaderType = shader_type;
			switch (shader_type) {
			case RHI::ShaderType::VERTEX:
				description.SourcePath = "./shaders/spirv/" + name + ".vs.spv";
				description.EntryPoint = "VS";
				break;
			case RHI::ShaderType::PIXEL:
				description.SourcePath = "./shaders/spirv/" + name + ".ps.spv";
				description.EntryPoint = "PS";
				break;
			case RHI::ShaderType::COMPUTE:
				description.SourcePath = "./shaders/spirv/" + name + ".cs.spv";
				description.EntryPoint = "main";
				break;
			}
			return description;
		}

		std::unordered_map<std::string, GPUShader*> m_shader_map;

		std::unordered_map<RHI::ShaderDescription, RHI::Shader, decltype(rhi_shader_description_hash_function), decltype(rhi_shader_description_equality_function)> m_rhi_shader_cache;
		RHI::RenderAPI m_api;
		RHI::Device m_device;
		std::mutex m_mutex;
	};
}
#endif