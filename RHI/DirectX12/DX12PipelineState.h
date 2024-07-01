#ifndef _RHI_DX12_PIPELINE_STATE_H_
#define _RHI_DX12_PIPELINE_STATE_H_
#include "../IPipelineState.h"
#include "./Headers/dx12.h"
#include "DX12Graphics.h"
#include "DX12Shader.h"
#include "DX12RootSignature.h"

namespace RHI::DX12 {
	class DX12PipelineState : public RHI::IPipelineState {
	public:
		DX12PipelineState(const RHI::GraphicsPipelineStateDescription& description, ID3D12PipelineState* dx12_pipeline_state);
		DX12PipelineState(const RHI::ComputePipelineStateDescription& description, ID3D12PipelineState* dx12_pipeline_state) :
			RHI::IPipelineState(description),
			m_DX12PipelineState(dx12_pipeline_state) {}
		~DX12PipelineState() override;

		ID3D12PipelineState* GetNative() const { return m_DX12PipelineState; };
	private:
		ID3D12PipelineState* m_DX12PipelineState;
		ID3D12StateObject* m_DX12State;
	};

	inline D3D12_GRAPHICS_PIPELINE_STATE_DESC DX12ConvertGraphicsPipelineStateDescription(const RHI::GraphicsPipelineStateDescription& description) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_description = {};
		ZeroMemory(&pipeline_state_description, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		pipeline_state_description.pRootSignature = static_cast<DX12RootSignature*>(description.RootSignature)->GetNative();
		if (description.VertexShader != RHI_NULL_HANDLE) {
			pipeline_state_description.VS = static_cast<DX12Shader*>(description.VertexShader)->GetByteCode();
		}
		if (description.PixelShader != RHI_NULL_HANDLE) {
			pipeline_state_description.PS = static_cast<DX12Shader*>(description.PixelShader)->GetByteCode();
		}
		if (description.DomainShader != RHI_NULL_HANDLE) {
			pipeline_state_description.DS = static_cast<DX12Shader*>(description.DomainShader)->GetByteCode();
		}
		if (description.HullShader != RHI_NULL_HANDLE) {
			pipeline_state_description.HS = static_cast<DX12Shader*>(description.HullShader)->GetByteCode();
		}
		if (description.GeometryShader != RHI_NULL_HANDLE) {
			pipeline_state_description.GS = static_cast<DX12Shader*>(description.GeometryShader)->GetByteCode();
		}

		//fill out Blend State.
		pipeline_state_description.BlendState = {};
		pipeline_state_description.BlendState.AlphaToCoverageEnable = description.BlendDescription.AlphaToCoverageEnable ? TRUE : FALSE;
		pipeline_state_description.BlendState.IndependentBlendEnable = description.BlendDescription.IndependentBlendEnable ? TRUE : FALSE;
		for (uint32_t i = 0u; i < 8u; ++i) {
			pipeline_state_description.BlendState.RenderTarget[i] = {};
			pipeline_state_description.BlendState.RenderTarget[i].BlendEnable = description.BlendDescription.RenderTargetBlendDescription[i].BlendEnable ? TRUE : FALSE;
			pipeline_state_description.BlendState.RenderTarget[i].LogicOpEnable = description.BlendDescription.RenderTargetBlendDescription[i].BlendLogicEnable ? TRUE : FALSE;
			pipeline_state_description.BlendState.RenderTarget[i].SrcBlend = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].SourceBlend);
			pipeline_state_description.BlendState.RenderTarget[i].DestBlend = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend);
			pipeline_state_description.BlendState.RenderTarget[i].BlendOp = DX12ConvertBlendOperation(description.BlendDescription.RenderTargetBlendDescription[i].BlendOperation);
			pipeline_state_description.BlendState.RenderTarget[i].SrcBlendAlpha = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha);
			pipeline_state_description.BlendState.RenderTarget[i].DestBlendAlpha = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha);
			pipeline_state_description.BlendState.RenderTarget[i].BlendOpAlpha = DX12ConvertBlendOperation(description.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha);
			pipeline_state_description.BlendState.RenderTarget[i].LogicOp = DX12ConvertLogicOperation(description.BlendDescription.RenderTargetBlendDescription[i].LogicOperation);
			pipeline_state_description.BlendState.RenderTarget[i].RenderTargetWriteMask = DX12ConvertColorWriteMask(description.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask);
		}

		pipeline_state_description.SampleMask = UINT_MAX;

		//fill out Rasterizer State.
		pipeline_state_description.RasterizerState = {};
		pipeline_state_description.RasterizerState.FillMode = DX12ConvertFillMode(description.RasterizerDescription.FillMode);
		pipeline_state_description.RasterizerState.CullMode = DX12ConvertCullMode(description.RasterizerDescription.CullMode);
		pipeline_state_description.RasterizerState.FrontCounterClockwise = description.RasterizerDescription.FrontCounterClockwise ? TRUE : FALSE;
		pipeline_state_description.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		pipeline_state_description.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		pipeline_state_description.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		pipeline_state_description.RasterizerState.DepthClipEnable = TRUE;
		pipeline_state_description.RasterizerState.MultisampleEnable = FALSE;
		pipeline_state_description.RasterizerState.AntialiasedLineEnable = FALSE;
		pipeline_state_description.RasterizerState.ForcedSampleCount = 0u;
		pipeline_state_description.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		pipeline_state_description.DepthStencilState = {};
		pipeline_state_description.DepthStencilState.DepthEnable = description.DepthStencilDescription.DepthEnabled ? TRUE : FALSE;
		pipeline_state_description.DepthStencilState.DepthWriteMask = DX12ConvertDepthWriteMask(description.DepthStencilDescription.DepthWriteMask);
		pipeline_state_description.DepthStencilState.DepthFunc = DX12ConvertComparisonFunction(description.DepthStencilDescription.ComparisonFunction);
		pipeline_state_description.DepthStencilState.StencilEnable = description.DepthStencilDescription.StencilEnabled ? TRUE : FALSE;
		pipeline_state_description.DepthStencilState.StencilReadMask = description.DepthStencilDescription.StencilReadMask;
		pipeline_state_description.DepthStencilState.StencilWriteMask = description.DepthStencilDescription.StencilWriteMask;
		pipeline_state_description.DepthStencilState.FrontFace = {}; //TODO
		pipeline_state_description.DepthStencilState.BackFace = {}; //TODO

		//fill out Input Layout.
		static std::vector<D3D12_INPUT_ELEMENT_DESC> dx12_input_layout = std::vector<D3D12_INPUT_ELEMENT_DESC>(description.InputLayout.InputElements.size());
		for (uint32_t i = 0u; i < description.InputLayout.InputElements.size(); ++i) {
			dx12_input_layout[i] = { description.InputLayout.InputElements[i].Name.c_str(), 0, DX12ConvertFormat(description.InputLayout.InputElements[i].Format), 0, description.InputLayout.InputElements[i].Offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		}
		pipeline_state_description.InputLayout = { dx12_input_layout.data(), (UINT)dx12_input_layout.size() };
		pipeline_state_description.PrimitiveTopologyType = DX12ConvertPrimitiveTopologyType(description.PrimitiveTopology);
		pipeline_state_description.NumRenderTargets = description.NumberOfRenderTargets;
		for (uint32_t i = 0u; i < description.NumberOfRenderTargets; ++i) {
			pipeline_state_description.RTVFormats[i] = DX12ConvertFormat(description.RenderTargetFormats[i]);
		}
		pipeline_state_description.DSVFormat = DX12ConvertFormat(description.DepthFormat);
		pipeline_state_description.SampleDesc = {};
		pipeline_state_description.SampleDesc.Count = description.SampleDescription.Count;
		pipeline_state_description.SampleDesc.Quality = description.SampleDescription.Quality;
		pipeline_state_description.NodeMask = description.NodeMask;
		pipeline_state_description.CachedPSO = {};
		pipeline_state_description.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		return pipeline_state_description;
	}

	inline D3D12_COMPUTE_PIPELINE_STATE_DESC DX12ConvertComputePipelineStateDescription(const RHI::ComputePipelineStateDescription& description) {
		D3D12_COMPUTE_PIPELINE_STATE_DESC dx12_compute_pipeline_state_desc = {};
		dx12_compute_pipeline_state_desc.pRootSignature = static_cast<DX12RootSignature*>(description.RootSignature)->GetNative();
		dx12_compute_pipeline_state_desc.CS = static_cast<DX12Shader*>(description.ComputeShader)->GetByteCode();
		dx12_compute_pipeline_state_desc.NodeMask = description.NodeMask;
		dx12_compute_pipeline_state_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		return dx12_compute_pipeline_state_desc;
	}
}
#endif