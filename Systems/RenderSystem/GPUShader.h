#ifndef _AT_RENDER_SYSTEM_GPU_SHADER_H_
#define _AT_RENDER_SYSTEM_GPU_SHADER_H_
#include <unordered_map>
#include <string>
#include <vector>
#include "../RHI/RHI.h"
#include "GPUConstantBuffer.h"
#include "GPURootSignatureManager.h"

namespace AT {
	class FrameGraphBuilder;

	class GPUShader {
	public:
		struct CBuffer {
			RHI::ConstantBufferView cbv;
		};

		struct RWTexture2D {
			RHI::UnorderedAccessView uav;
		};

		struct Texture2D {
			RHI::ShaderResourceView srv;
		};
		
		struct Texture2DArray {
			RHI::ShaderResourceView srv;
		};
		
		struct TextureCube {
			RHI::ShaderResourceView srv;
		};

		typedef RHI::StaticSamplerDescription StaticSampler;

		class ShaderParameters {
		public:
			ShaderParameters() {};
			virtual ~ShaderParameters() {};
		protected:
		};

		class ShaderParameterGroup {
		public:
			friend FrameGraphBuilder;
			ShaderParameterGroup() = default;
			
			~ShaderParameterGroup() {}
			struct TypeInfo {
				std::vector<RHI::RootDescriptorTableRange> Ranges;
			};
			
			RHI::DescriptorTable m_descriptor_table = RHI_NULL_HANDLE;
			friend class FrameGraphBuilder;
		private:
		};

		GPUShader(RHI::Device device) :
			m_Device(device),
			m_RootSignature(RHI_NULL_HANDLE) {}
		virtual void SetParameters(RHI::CommandList command_list, ShaderParameters* parameters) const = 0;
		RHI::RootSignature GetRootSignature() const { return m_RootSignature; }
	protected:
		RHI::Device m_Device;
		RHI::RootSignature m_RootSignature;
	};
}

struct DescriptorIndexCounter {
	uint32_t cbv = 0u;
	uint32_t srv = 0u;
	uint32_t uav = 0u;
};

#define BEGIN_SHADER_PARAMETER_GROUP(GroupName) \
	class GroupName : public AT::GPUShader::ShaderParameterGroup \
	{ \
	public: \
		inline static RHI::RootParameter root_parameter; \
		inline static bool inited = false; \
		GroupName () { \
		} \
		inline static RHI::RootParameter GetRootParameter(uint32_t register_space) { \
			if (!inited) { \
				_ThisStruct s; \
				DescriptorIndexCounter counter; \
				root_parameter.RootParameterType = RHI::RootParameterType::DESCRIPTOR_TABLE; \
				_MembersFollowingToDescriptorRange(_FirstMemberId(), s, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
				inited = true; \
			} \
			return root_parameter; \
		} \
	private: \
		typedef GroupName _ThisStruct; \
		struct _FirstMemberId {}; \
		typedef _FirstMemberId 
#define SHADER_PARAMETER(ParameterType, ParameterName) \
		_ParameterId_##ParameterName; \
	public: \
		ParameterType ParameterName; \
	private: \
		struct _NextParameterId_##ParameterName {}; \
		static void _MembersFollowingToDescriptorRange(_ParameterId_##ParameterName, const _ThisStruct& Struct, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) \
		{ \
			ToRHIRange(Struct.ParameterName, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
			_MembersFollowingToDescriptorRange(_NextParameterId_##ParameterName(), Struct, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
		} \
		typedef _NextParameterId_##ParameterName
#define SHADER_PARAMETER_ARRAY(ParameterType, ParameterName, size) \
		_ParameterId_##ParameterName; \
	public: \
		ParameterType ParameterName[size]; \
	private: \
		struct _NextParameterId_##ParameterName {}; \
		static void _MembersFollowingToDescriptorRange(_ParameterId_##ParameterName, const _ThisStruct& Struct, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) \
		{ \
			ToRHIRange(Struct.ParameterName, size, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
			_MembersFollowingToDescriptorRange(_NextParameterId_##ParameterName(), Struct, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
		} \
		typedef _NextParameterId_##ParameterName

#define END_SHADER_PARAMETER_GROUP(GroupName) \
		_LastParameterId; \
		static void _MembersFollowingToDescriptorRange(_LastParameterId, const _ThisStruct& Struct, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) { return; } \
	};

#define BEGIN_SHADER_PARAMETERS(ParametersName) \
	class ParametersName : public AT::GPUShader::ShaderParameters \
	{ \
	public: \
		inline static RHI::RootSignatureDescription root_signature_description; \
		inline static bool inited = false; \
		ParametersName () { \
			if (!inited) { \
				_MembersFollowingToRootSignatureDescription(_FirstGroupId(), *this, root_signature_description); \
				inited = true; \
			} \
		} \
		~ParametersName() override {} \
	private: \
		typedef ParametersName _ThisStruct; \
		struct _FirstGroupId {}; \
		typedef _FirstGroupId
#define SHADER_PARAMETER_GROUP(ShaderParameterGroupType, GroupName) \
	_GroupId_##GroupName; \
	public: \
		ShaderParameterGroupType * GroupName; \
	private: \
		struct _NextGroupId_##GroupName {}; \
		static void _MembersFollowingToRootSignatureDescription(_GroupId_##GroupName, const _ThisStruct& Struct, RHI::RootSignatureDescription& description) \
		{ \
			description.Parameters.push_back(Struct.GroupName->GetRootParameter(description.Parameters.size())); \
			_MembersFollowingToRootSignatureDescription(_NextGroupId_##GroupName(), Struct, description); \
		}\
		typedef _NextGroupId_##GroupName
#define BEGIN_STATIC_SAMPLER(SamplerName) \
	_GroupId_##SamplerName; \
	public: \
		StaticSampler SamplerName = { 
#define END_STATIC_SAMPLER(SamplerName) \
	}; \
	private: \
		struct _NextGroupId_##SamplerName {}; \
		static void _MembersFollowingToRootSignatureDescription(_GroupId_##SamplerName, const _ThisStruct& Struct, RHI::RootSignatureDescription& description) \
		{ \
			description.StaticSamplers.push_back(Struct.SamplerName); \
			description.StaticSamplers.back().ShaderRegister = description.StaticSamplers.size() - 1; \
			description.StaticSamplers.back().RegisterSpace = 0; \
			description.Parameters[0].RootDescriptorTable.StaticSamplers = description.StaticSamplers; \
			_MembersFollowingToRootSignatureDescription(_NextGroupId_##SamplerName(), Struct, description); \
		}\
		typedef _NextGroupId_##SamplerName
#define END_SHADER_PARAMETERS(ParametersName) \
		_LastGroupId; \
		static void _MembersFollowingToRootSignatureDescription(_LastGroupId, const _ThisStruct& Struct, RHI::RootSignatureDescription& description) { return; } \
		private: \
	};
#define BEGIN_CONSTANTS _ParameterId_constants; \
	public: \
		struct Constants {
#define DEFINE_CONSTANT(ConstantType, ConstantName) ConstantType ConstantName;
#define END_CONSTANTS } constants; \
		AT::GPUConstantBuffer* constant_buffer; \
	private: \
		struct _NextParameterId_constants {}; \
		static void _MembersFollowingToDescriptorRange(_ParameterId_constants, const _ThisStruct& Struct, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) \
		{ \
			ToRHIRange(&Struct.constants, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
			_MembersFollowingToDescriptorRange(_NextParameterId_constants(), Struct, root_parameter.RootDescriptorTable.Ranges, register_space, counter); \
		} \
		typedef _NextParameterId_##constants

inline void ToRHIRange(const void* constants, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) {
	if (ranges.size() > 0 && !ranges.back().IsArray && ranges.back().RangeType == RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW) {
		++ranges.back().DescriptorCount;
		++counter.cbv;
	}
	else {
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW;
		range.DescriptorCount = 1;
		range.ShaderRegister = counter.cbv++;
		range.RegisterSpace = register_space;
		range.IsArray = false;
		ranges.push_back(range);
	}
}

inline void ToRHIRange(const AT::GPUShader::RWTexture2D& rw_texture2d, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) {
	if (ranges.size() > 0 && !ranges.back().IsArray && ranges.back().RangeType == RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE) {
		++ranges.back().DescriptorCount;
		++counter.uav;
	}
	else {
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::UNORDERED_ACCESS_VIEW_TEXTURE;
		range.DescriptorCount = 1;
		range.ShaderRegister = counter.uav++;
		range.RegisterSpace = register_space;
		range.IsArray = false;
		ranges.push_back(range);
	}
}

inline void ToRHIRange(const AT::GPUShader::TextureCube& texture_cube, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) {
	if (ranges.size() > 0 && !ranges.back().IsArray && ranges.back().RangeType == RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE) {
		++ranges.back().DescriptorCount;
		++counter.srv;
	}
	else {
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE;
		range.DescriptorCount = 1;
		range.ShaderRegister = counter.srv++;
		range.RegisterSpace = register_space;
		range.IsArray = false;
		ranges.push_back(range);
	}
}

inline void ToRHIRange(const AT::GPUShader::Texture2D& texture2d, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) {
	if (ranges.size() > 0 && !ranges.back().IsArray && ranges.back().RangeType == RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE) {
		++ranges.back().DescriptorCount;
		++counter.srv;
	}
	else {
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE;
		range.DescriptorCount = 1;
		range.ShaderRegister = counter.srv++;
		range.RegisterSpace = register_space;
		range.IsArray = false;
		ranges.push_back(range);
	}
}

inline void ToRHIRange(const AT::GPUShader::Texture2D* texture2d, uint32_t size, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) {
	RHI::RootDescriptorTableRange range;
	range.RangeType = RHI::DescriptorRangeType::SHADER_RESOURCE_VIEW_TEXTURE;
	range.DescriptorCount = size;
	range.ShaderRegister = counter.srv;
	counter.srv += size;
	range.RegisterSpace = register_space;
	range.IsArray = true;
	ranges.push_back(range);
}

inline void ToRHIRange(const AT::GPUShader::CBuffer& cbuffer, std::vector<RHI::RootDescriptorTableRange>& ranges, uint32_t register_space, DescriptorIndexCounter& counter) {
	if (ranges.size() > 0 && !ranges.back().IsArray && ranges.back().RangeType == RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW) {
		++ranges.back().DescriptorCount;
		++counter.cbv;
	}
	else {
		RHI::RootDescriptorTableRange range;
		range.RangeType = RHI::DescriptorRangeType::CONSTANT_BUFFER_VIEW;
		range.DescriptorCount = 1;
		range.ShaderRegister = counter.cbv++;
		range.RegisterSpace = register_space;
		range.IsArray = false;
		ranges.push_back(range);
	}
}
#endif