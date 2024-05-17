#ifndef _RHI_I_ROOT_SIGNATURE_H_
#define _RHI_I_ROOT_SIGNATURE_H_
#include <vector>
#include <variant>
#include "RHICore.h"
#include "ISampler.h"

namespace RHI {
	typedef struct RootConstant {
		uint32_t ConstantCount = 0u;
		uint32_t ShaderRegister = 0u;
		uint32_t RegisterSpace = 0u;
	} RootConstant;

	typedef struct RootDescriptor {
		uint32_t ShaderRegister = 0u;
		uint32_t RegisterSpace = 0u;
	} RootDescriptor;

	enum class DescriptorRangeType {
		CONSTANT_BUFFER_VIEW,
		SAMPLER_VIEW,
		SHADER_RESOURCE_VIEW_BUFFER,
		SHADER_RESOURCE_VIEW_TEXTURE,
		UNORDERED_ACCESS_VIEW_BUFFER,
		UNORDERED_ACCESS_VIEW_TEXTURE
	};
	typedef struct RootDescriptorTableRange {
		DescriptorRangeType RangeType = DescriptorRangeType::CONSTANT_BUFFER_VIEW;
		uint32_t DescriptorCount = 0u;
		uint32_t ShaderRegister = 0u;
		uint32_t RegisterSpace = 0u;
		bool IsArray = false;
	} RootDescriptorTableRange;

#undef DOMAIN
	enum class ShaderVisibility {
		ALL,
		VERTEX,
		HULL,
		DOMAIN,
		GEOMETRY,
		PIXEL,
	};

	typedef struct StaticSamplerDescription {
		RHI::Filter Filter;
		RHI::TextureAddressMode AddressU;
		RHI::TextureAddressMode AddressV;
		RHI::TextureAddressMode AddressW;
		float MipLODBias;
		uint32_t MaxAnisotropy;
		RHI::ComparisonFunction ComparisonFunction;
		enum class StaticBorderColor {
			TRANSPARENT_BLACK = 0,
			OPAQUE_BLACK,
			OPAQUE_WHITE,
		} BorderColor;
		float MinLOD;
		float MaxLOD;
		uint32_t ShaderRegister;
		uint32_t RegisterSpace;
		ShaderVisibility ShaderVisibility = ShaderVisibility::ALL;
	} StaticSamplerDescription;

	typedef struct RootDescriptorTable {
		std::vector<RootDescriptorTableRange> Ranges = std::vector<RootDescriptorTableRange>();
		std::vector<StaticSamplerDescription> StaticSamplers = std::vector<StaticSamplerDescription>();//todo remove
	} RootDescriptorTable;

	enum class RootParameterType {
		CONSTANT,
		DESCRIPTOR_TABLE
	};

	typedef struct RootParameter {
		RootParameterType RootParameterType = RootParameterType::CONSTANT;
		RootDescriptorTable RootDescriptorTable;
		union {
			RootConstant RootConstant = {};
			RootDescriptor RootDescriptor;
		};
		ShaderVisibility ShaderVisibility = ShaderVisibility::ALL;
		//RootParameter() {}
		//RootParameter() {}
	} RootParameter;

	typedef struct RootSignatureDescription {
		std::vector<RootParameter> Parameters = std::vector<RootParameter>();
		std::vector<StaticSamplerDescription> StaticSamplers;
	} RootSignatureDescription;

	class IRootSignature {
	public:
		virtual ~IRootSignature() = default;
		const RootSignatureDescription& GetDescription() const { return m_Description; }
	protected:
		RootSignatureDescription m_Description;
		IRootSignature(const RootSignatureDescription& description) :
			m_Description(description) {}
	};
}
#endif