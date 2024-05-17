#ifndef _AT_RENDER_SYSTEM_GPU_ROOT_SIGNATURE_MANAGER_H_
#define _AT_RENDER_SYSTEM_GPU_ROOT_SIGNATURE_MANAGER_H_
#include<unordered_map>
#include "../RHI/RHI.h"
#include "../Util/HashUtil.h"

namespace AT {
	static auto rhi_root_signature_description_hash_function = [](const RHI::RootSignatureDescription& description) {
		std::size_t hash = 0;
		AT::Util::Hash::hash_combine(hash, description.Parameters.size());
		for (uint32_t i = 0u; i < description.Parameters.size(); ++i) {
			switch (description.Parameters[i].RootParameterType) {
			case RHI::RootParameterType::CONSTANT:
			{
				AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootConstant.ConstantCount);
				AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootConstant.ShaderRegister);
				AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootConstant.RegisterSpace);
			}
				break;
			case RHI::RootParameterType::DESCRIPTOR_TABLE:
			{
				AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootDescriptorTable.Ranges.size());
				for (uint32_t j = 0u; j < description.Parameters[i].RootDescriptorTable.Ranges.size(); ++j) {
					AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootDescriptorTable.Ranges[j].RangeType);
					//AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorStartIndex);
					AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorCount);
					AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootDescriptorTable.Ranges[j].ShaderRegister);
					AT::Util::Hash::hash_combine(hash, description.Parameters[i].RootDescriptorTable.Ranges[j].RegisterSpace);
				}
			}
				break;
			}
		}
		AT::Util::Hash::hash_combine(hash, description.StaticSamplers.size());
		for (uint32_t i = 0u; i < description.StaticSamplers.size(); ++i) {
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].Filter);
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].AddressU);
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].AddressV);
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].AddressW);
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].MinLOD);
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].MaxLOD);
			AT::Util::Hash::hash_combine(hash, description.StaticSamplers[i].MaxAnisotropy);
		}
		return hash;
	};

	static auto rhi_root_signature_description_equality_function = [](const RHI::RootSignatureDescription& description1, const RHI::RootSignatureDescription& description2) {
		if (description1.Parameters.size() != description2.Parameters.size()) {
			return false;
		}

		for (uint32_t i = 0u; i < description1.Parameters.size(); ++i) {
			switch (description1.Parameters[i].RootParameterType) {
			case RHI::RootParameterType::CONSTANT:
			{
				if ((description1.Parameters[i].RootConstant.ConstantCount != description2.Parameters[i].RootConstant.ConstantCount) ||
					(description1.Parameters[i].RootConstant.ShaderRegister != description2.Parameters[i].RootConstant.ShaderRegister) ||
					(description1.Parameters[i].RootConstant.RegisterSpace != description2.Parameters[i].RootConstant.RegisterSpace))
				{
					return false;
				}
			}
			break;
			case RHI::RootParameterType::DESCRIPTOR_TABLE:
			{
				if (description1.Parameters[i].RootDescriptorTable.Ranges.size() != description2.Parameters[i].RootDescriptorTable.Ranges.size()) {
					return false;
				}
				for (uint32_t j = 0u; j < description1.Parameters[i].RootDescriptorTable.Ranges.size(); ++j) {
					if (
						(description1.Parameters[i].RootDescriptorTable.Ranges[j].RangeType != description2.Parameters[i].RootDescriptorTable.Ranges[j].RangeType) ||
						//(description1.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorStartIndex != description2.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorStartIndex) ||
						(description1.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorCount != description2.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorCount) ||
						(description1.Parameters[i].RootDescriptorTable.Ranges[j].ShaderRegister != description2.Parameters[i].RootDescriptorTable.Ranges[j].ShaderRegister) ||
						(description1.Parameters[i].RootDescriptorTable.Ranges[j].RegisterSpace != description2.Parameters[i].RootDescriptorTable.Ranges[j].RegisterSpace)
					) {
						return false;
					}
				}
			}
			break;
			}
		}
		if (description1.StaticSamplers.size() != description2.StaticSamplers.size()) {
			return false;
		}
		for (uint32_t i = 0u; i < description1.StaticSamplers.size(); ++i) {
			if (
				(description1.StaticSamplers[i].Filter != description2.StaticSamplers[i].Filter) ||
				(description1.StaticSamplers[i].AddressU != description2.StaticSamplers[i].AddressU) ||
				(description1.StaticSamplers[i].AddressV != description2.StaticSamplers[i].AddressV) ||
				(description1.StaticSamplers[i].AddressW != description2.StaticSamplers[i].AddressW) ||
				(description1.StaticSamplers[i].MinLOD != description2.StaticSamplers[i].MinLOD) ||
				(description1.StaticSamplers[i].MaxLOD != description2.StaticSamplers[i].MaxLOD) ||
				(description1.StaticSamplers[i].MaxAnisotropy != description2.StaticSamplers[i].MaxAnisotropy)
			) {
				return false;
			}
		}
		return true;
	};

	class GPURootSignatureManager {
	public:
		GPURootSignatureManager(RHI::Device device) :
			m_Device(device)
		{

		}

		~GPURootSignatureManager() {
			for (auto pair : m_RHIRootSignatureCache) {
				delete pair.second;
			}
		}

		RHI::RootSignature CreateOrGetRootSignature(const RHI::RootSignatureDescription& description) {
			RHI::RootSignature root_signature = RHI_NULL_HANDLE;
			if (m_RHIRootSignatureCache.find(description) == m_RHIRootSignatureCache.end()) {
				m_Device->CreateRootSignature(description, root_signature);
				m_RHIRootSignatureCache[description] = root_signature;
			}
			else {
				root_signature = m_RHIRootSignatureCache[description];
			}

			return root_signature;
		}

		RHI::Device GetDevice() const {
			return m_Device;
		}

	private:
		RHI::Device m_Device;
		std::unordered_map<RHI::RootSignatureDescription, RHI::RootSignature, decltype(rhi_root_signature_description_hash_function), decltype(rhi_root_signature_description_equality_function)> m_RHIRootSignatureCache;
	};
}
#endif