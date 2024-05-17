#ifndef _DX12_SAMPLER_H_
#define _DX12_SAMPLER_H_
#include "../ISampler.h"
#include "./Headers/dx12.h"
#include "DX12Graphics.h"
#include "DX12ViewAllocator.h"

namespace RHI::DX12 {
	class DX12Sampler : public RHI::ISampler {
	public:
		DX12Sampler(const RHI::SamplerDescription& description, DX12ViewAllocator& view_allocator, D3D12_CPU_DESCRIPTOR_HANDLE dx12_handle) :
			RHI::ISampler(description),
			m_ViewAllocator(view_allocator),
			m_DX12Handle(dx12_handle)
		{

		}
		~DX12Sampler() override {
			m_ViewAllocator.FreeHandle(m_DX12Handle);
		}
		inline D3D12_CPU_DESCRIPTOR_HANDLE GetNative() { return m_DX12Handle; };
	private:
		DX12ViewAllocator& m_ViewAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE m_DX12Handle;
	};

	inline D3D12_SAMPLER_DESC DX12ConvertSamplerDescription(const RHI::SamplerDescription& description) {
		D3D12_SAMPLER_DESC dx12_sampler_description;
		dx12_sampler_description.Filter = DX12ConvertFilter(description.Filter);
		dx12_sampler_description.AddressU = DX12ConvertTextureAddressMode(description.AddressU);
		dx12_sampler_description.AddressV = DX12ConvertTextureAddressMode(description.AddressV);
		dx12_sampler_description.AddressW = DX12ConvertTextureAddressMode(description.AddressW);
		dx12_sampler_description.MipLODBias = description.MipLODBias;
		dx12_sampler_description.MaxAnisotropy = description.MaxAnisotropy;
		dx12_sampler_description.ComparisonFunc = DX12ConvertComparisonFunction(description.ComparisonFunction);
		dx12_sampler_description.MinLOD = description.MinLOD;
		dx12_sampler_description.MaxLOD = description.MaxLOD;
		dx12_sampler_description.BorderColor[0] = 0.0f;
		dx12_sampler_description.BorderColor[1] = 0.0f;
		dx12_sampler_description.BorderColor[2] = 0.0f;
		dx12_sampler_description.BorderColor[3] = 0.0f;
		return dx12_sampler_description;
	}
}
#endif