#ifndef _AT_RENDER_SYSTEM_GPU_PIPELINE_STATE_MANAGER_H_
#define _AT_RENDER_SYSTEM_GPU_PIPELINE_STATE_MANAGER_H_
#include "../RHI/RHI.h"
#include "../Util/HashUtil.h"

namespace AT {
	class GPUGraphicsPipelineStateBuilder {
	public:
		GPUGraphicsPipelineStateBuilder() {
			m_Description.RootSignature = RHI_NULL_HANDLE;
			m_Description.VertexShader = RHI_NULL_HANDLE;
			m_Description.DomainShader = RHI_NULL_HANDLE;
			m_Description.HullShader = RHI_NULL_HANDLE;
			m_Description.GeometryShader = RHI_NULL_HANDLE;
			m_Description.PixelShader = RHI_NULL_HANDLE;
			m_Description.BlendDescription.AlphaToCoverageEnable = false;
			m_Description.BlendDescription.IndependentBlendEnable = false;
			for (uint32_t i = 0u; i < 8; ++i) {
				m_Description.BlendDescription.RenderTargetBlendDescription[i].BlendEnable = false;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].BlendLogicEnable = false;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].BlendOperation = RHI::BlendOperation::ADD;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha = RHI::BlendOperation::ADD;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend = RHI::Blend::DESTINATION_COLOR;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha = RHI::Blend::DESTINATION_ALPHA;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].LogicOperation = RHI::LogicOperation::AND;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask = RHI::ColorWriteEnable::ALL;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].SourceBlend = RHI::Blend::SOURCE_COLOR;
				m_Description.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha = RHI::Blend::SOURCE_ALPHA;
			}
			m_Description.RasterizerDescription.CullMode = RHI::CullMode::NONE;
			m_Description.RasterizerDescription.FillMode = RHI::FillMode::SOLID;
			m_Description.RasterizerDescription.FrontCounterClockwise = false;

			m_Description.DepthStencilDescription.ComparisonFunction = RHI::ComparisonFunction::LESS_EQUAL;
			m_Description.DepthStencilDescription.DepthEnabled = false;
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.ComparisonFunction = RHI::ComparisonFunction::ALWAYS;
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.StencilDepthFailOperation = RHI::StencilOperation::ZERO;
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.StencilFailOperation = RHI::StencilOperation::ZERO;
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.StencilPassOperation = RHI::StencilOperation::ZERO;
			m_Description.DepthStencilDescription.DepthWriteMask = RHI::DepthWriteMask::ZERO;
			m_Description.DepthStencilDescription.StencilEnabled = false;
			m_Description.DepthStencilDescription.StencilReadMask = 0u;
			m_Description.DepthStencilDescription.StencilWriteMask = 0u;

			m_Description.InputLayout = {};
			m_Description.PrimitiveTopology = RHI::PrimitiveTopology::TRIANGLE_LIST;
			m_Description.NumberOfRenderTargets = 0;
			for (uint32_t i = 0u; i < 8; ++i) {
				m_Description.RenderTargetFormats[i] = RHI::Format::UNKNOWN;
			}

			m_Description.DepthFormat = RHI::Format::UNKNOWN;
			m_Description.SampleDescription.Count = 1;
			m_Description.SampleDescription.Quality = 0;
			m_Description.NodeMask = 0;
		}

		GPUGraphicsPipelineStateBuilder& SetRootSignature(RHI::RootSignature root_signature) {
			m_Description.RootSignature = root_signature;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetVertexShader(RHI::Shader vertex_shader) {
			m_Description.VertexShader = vertex_shader;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetDomainShader(RHI::Shader domain_shader) {
			m_Description.DomainShader = domain_shader;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetHullShader(RHI::Shader hull_shader) {
			m_Description.HullShader = hull_shader;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetGeometryShader(RHI::Shader geometry_shader) {
			m_Description.GeometryShader = geometry_shader;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetPixelShader(RHI::Shader pixel_shader) {
			m_Description.PixelShader = pixel_shader;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& EnableAlphaToCoverage(bool value) {
			m_Description.BlendDescription.AlphaToCoverageEnable = value;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& EnableIndependentBlend(bool value) {
			m_Description.BlendDescription.IndependentBlendEnable = value;
			return *this;
		}
		//TODO Implement Set Render Target Blend Functions.
		GPUGraphicsPipelineStateBuilder& SetCullMode(RHI::CullMode cull_mode) {
			m_Description.RasterizerDescription.CullMode = cull_mode;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetFillMode(RHI::FillMode fill_mode) {
			m_Description.RasterizerDescription.FillMode = fill_mode;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetFrontCounterClockwise(bool value) {
			m_Description.RasterizerDescription.FrontCounterClockwise = value;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetDepthStencilComparisonFunction(RHI::ComparisonFunction function) {
			m_Description.DepthStencilDescription.ComparisonFunction = function;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetDepthEnabled(bool value) {
			m_Description.DepthStencilDescription.DepthEnabled = value;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilComparisonFunction(RHI::ComparisonFunction function) {
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.ComparisonFunction = function;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilDepthFailOP(RHI::StencilOperation op) {
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.StencilDepthFailOperation = op;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilFailOP(RHI::StencilOperation op) {
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.StencilFailOperation = op;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilPassOP(RHI::StencilOperation op) {
			m_Description.DepthStencilDescription.DepthStencilOperationDescription.StencilPassOperation = op;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetDepthWriteMask(RHI::DepthWriteMask depth_mask) {
			m_Description.DepthStencilDescription.DepthWriteMask = depth_mask;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilEnabled(bool value) {
			m_Description.DepthStencilDescription.StencilEnabled = value;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilReadMask(uint8_t mask) {
			m_Description.DepthStencilDescription.StencilReadMask = mask;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetStencilWriteMask(uint8_t mask) {
			m_Description.DepthStencilDescription.StencilWriteMask = mask;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetInputLayout(RHI::InputLayout& input_layout) {
			m_Description.InputLayout = input_layout;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetPrimitiveTopology(RHI::PrimitiveTopology primitive_topology) {
			m_Description.PrimitiveTopology = primitive_topology;
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetRenderTargetFormats(const std::vector<RHI::Format>& render_target_formats) {
			m_Description.NumberOfRenderTargets = render_target_formats.size();
			for (uint32_t i = 0u; i < render_target_formats.size(); ++i) {
				m_Description.RenderTargetFormats[i] = render_target_formats[i];
			}
			return *this;
		}

		GPUGraphicsPipelineStateBuilder& SetDepthFormat(RHI::Format depth_format) {
			m_Description.DepthFormat = depth_format;
			return *this;
		}

		//TODO enable sample settings in the future.

		const RHI::GraphicsPipelineStateDescription& ToRHIDescription() const {
			return m_Description;
		}
	private:
		RHI::GraphicsPipelineStateDescription m_Description;
	};


	class GPUComputePipelineStateBuilder {
	public:
		GPUComputePipelineStateBuilder() {
			m_Description.RootSignature = RHI_NULL_HANDLE;
			m_Description.ComputeShader = RHI_NULL_HANDLE;
			m_Description.NodeMask = 0u;
		}

		GPUComputePipelineStateBuilder& SetRootSignature(RHI::RootSignature root_signature) {
			m_Description.RootSignature = root_signature;
			return *this;
		}

		GPUComputePipelineStateBuilder& SetComputeShader(RHI::Shader compute_shader) {
			m_Description.ComputeShader = compute_shader;
			return *this;
		}

		const RHI::ComputePipelineStateDescription ToRHIDescription() const {
			return m_Description;
		}
	private:
		RHI::ComputePipelineStateDescription m_Description;
	};

	auto rhi_graphics_pipeline_state_description_hash_function = [](const RHI::GraphicsPipelineStateDescription& description) {
		std::size_t hash = 0;
		AT::Util::Hash::hash_combine(hash, description.RootSignature);
		AT::Util::Hash::hash_combine(hash, description.VertexShader);
		AT::Util::Hash::hash_combine(hash, description.DomainShader);
		AT::Util::Hash::hash_combine(hash, description.HullShader);
		AT::Util::Hash::hash_combine(hash, description.GeometryShader);
		AT::Util::Hash::hash_combine(hash, description.PixelShader);
		AT::Util::Hash::hash_combine(hash, description.BlendDescription.AlphaToCoverageEnable);
		AT::Util::Hash::hash_combine(hash, description.BlendDescription.IndependentBlendEnable);
		for (uint32_t i = 0u; i < 8; ++i) {
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].BlendEnable);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].BlendLogicEnable);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].BlendOperation);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].LogicOperation);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].SourceBlend);
			AT::Util::Hash::hash_combine(hash, description.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha);
		}
		AT::Util::Hash::hash_combine(hash, description.RasterizerDescription.CullMode);
		AT::Util::Hash::hash_combine(hash, description.RasterizerDescription.FillMode);
		AT::Util::Hash::hash_combine(hash, description.RasterizerDescription.FrontCounterClockwise);

		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.ComparisonFunction);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.DepthEnabled);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.DepthStencilOperationDescription.ComparisonFunction);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.DepthStencilOperationDescription.StencilDepthFailOperation);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.DepthStencilOperationDescription.StencilFailOperation);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.DepthStencilOperationDescription.StencilPassOperation);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.DepthWriteMask);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.StencilEnabled);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.StencilReadMask);
		AT::Util::Hash::hash_combine(hash, description.DepthStencilDescription.StencilWriteMask);

		for (uint32_t i = 0u; i < description.InputLayout.InputElements.size(); ++i) {
			AT::Util::Hash::hash_combine(hash, description.InputLayout.InputElements[i].Format);
			AT::Util::Hash::hash_combine(hash, description.InputLayout.InputElements[i].Name);
			AT::Util::Hash::hash_combine(hash, description.InputLayout.InputElements[i].Offset);
		}
		AT::Util::Hash::hash_combine(hash, description.InputLayout.InputStride);
		AT::Util::Hash::hash_combine(hash, description.PrimitiveTopology);
		AT::Util::Hash::hash_combine(hash, description.NumberOfRenderTargets);
		for (uint32_t i = 0u; i < 8; ++i) {
			AT::Util::Hash::hash_combine(hash, description.RenderTargetFormats[i]);
		}

		AT::Util::Hash::hash_combine(hash, description.DepthFormat);
		AT::Util::Hash::hash_combine(hash, description.SampleDescription.Count);
		AT::Util::Hash::hash_combine(hash, description.SampleDescription.Quality);
		AT::Util::Hash::hash_combine(hash, description.NodeMask);
		return hash;
	};

	auto rhi_graphics_pipeline_state_desciption_equality_funciton = [](const RHI::GraphicsPipelineStateDescription& description1, const RHI::GraphicsPipelineStateDescription& description2) {
		if (
			(description1.RootSignature != description2.RootSignature) ||
			(description1.VertexShader != description2.VertexShader) ||
			(description1.DomainShader != description2.DomainShader) ||
			(description1.HullShader != description2.HullShader) ||
			(description1.GeometryShader != description2.GeometryShader) ||
			(description1.PixelShader != description2.PixelShader) ||
			(description1.BlendDescription.AlphaToCoverageEnable != description2.BlendDescription.AlphaToCoverageEnable) ||
			(description1.BlendDescription.IndependentBlendEnable != description2.BlendDescription.IndependentBlendEnable))
		{
			return false;
		}

		for (uint32_t i = 0u; i < 8; ++i) {
			if (
				(description1.BlendDescription.RenderTargetBlendDescription[i].BlendEnable != description2.BlendDescription.RenderTargetBlendDescription[i].BlendEnable) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].BlendLogicEnable != description2.BlendDescription.RenderTargetBlendDescription[i].BlendLogicEnable) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].BlendOperation != description2.BlendDescription.RenderTargetBlendDescription[i].BlendOperation) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha != description2.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend != description2.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha != description2.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].LogicOperation != description2.BlendDescription.RenderTargetBlendDescription[i].LogicOperation) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask != description2.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].SourceBlend != description2.BlendDescription.RenderTargetBlendDescription[i].SourceBlend) ||
				(description1.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha != description2.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha))
			{
				return false;
			}
		}

		if (
			(description1.RasterizerDescription.CullMode != description2.RasterizerDescription.CullMode) ||
			(description1.RasterizerDescription.FillMode != description2.RasterizerDescription.FillMode) ||
			(description1.RasterizerDescription.FrontCounterClockwise != description2.RasterizerDescription.FrontCounterClockwise) ||
			(description1.DepthStencilDescription.ComparisonFunction != description2.DepthStencilDescription.ComparisonFunction) ||
			(description1.DepthStencilDescription.DepthEnabled != description2.DepthStencilDescription.DepthEnabled) ||
			(description1.DepthStencilDescription.DepthStencilOperationDescription.ComparisonFunction != description2.DepthStencilDescription.DepthStencilOperationDescription.ComparisonFunction) ||
			(description1.DepthStencilDescription.DepthStencilOperationDescription.StencilDepthFailOperation != description2.DepthStencilDescription.DepthStencilOperationDescription.StencilDepthFailOperation) ||
			(description1.DepthStencilDescription.DepthStencilOperationDescription.StencilFailOperation != description2.DepthStencilDescription.DepthStencilOperationDescription.StencilFailOperation) ||
			(description1.DepthStencilDescription.DepthStencilOperationDescription.StencilPassOperation != description2.DepthStencilDescription.DepthStencilOperationDescription.StencilPassOperation) ||
			(description1.DepthStencilDescription.DepthWriteMask != description2.DepthStencilDescription.DepthWriteMask) ||
			(description1.DepthStencilDescription.StencilEnabled != description2.DepthStencilDescription.StencilEnabled) ||
			(description1.DepthStencilDescription.StencilReadMask != description2.DepthStencilDescription.StencilReadMask) ||
			(description1.DepthStencilDescription.StencilWriteMask != description2.DepthStencilDescription.StencilWriteMask)
		) 
		{
			return false;
		}

		for (uint32_t i = 0u; i < description1.InputLayout.InputElements.size(); ++i) {
			if (
				(description1.InputLayout.InputElements[i].Format != description2.InputLayout.InputElements[i].Format) ||
				(description1.InputLayout.InputElements[i].Name != description2.InputLayout.InputElements[i].Name) ||
				(description1.InputLayout.InputElements[i].Offset != description2.InputLayout.InputElements[i].Offset))
			{
				return false;
			}
		}

		if ((description1.InputLayout.InputStride != description2.InputLayout.InputStride) ||
			(description1.PrimitiveTopology != description2.PrimitiveTopology) ||
			(description1.NumberOfRenderTargets != description2.NumberOfRenderTargets))
		{
			return false;
		}
		for (uint32_t i = 0u; i < 8; ++i) {
			if (description1.RenderTargetFormats[i] != description2.RenderTargetFormats[i]) {
				return false;
			}
		}

		if (
			(description1.DepthFormat != description2.DepthFormat) ||
			(description1.SampleDescription.Count != description2.SampleDescription.Count) ||
			(description1.SampleDescription.Quality != description2.SampleDescription.Quality) ||
			(description1.NodeMask != description2.NodeMask))
		{
			return false;
		}
		return true;
	};

	auto rhi_compute_pipeline_state_description_hash_function = [](const RHI::ComputePipelineStateDescription& description) {
		std::size_t hash = 0;
		AT::Util::Hash::hash_combine(hash, description.RootSignature);
		AT::Util::Hash::hash_combine(hash, description.ComputeShader);
		AT::Util::Hash::hash_combine(hash, description.NodeMask);
		return hash;
	};

	auto rhi_compute_pipeline_state_desciption_equality_funciton = [](const RHI::ComputePipelineStateDescription& description1, const RHI::ComputePipelineStateDescription& description2) {
		return (description1.RootSignature == description2.RootSignature) &&
				(description1.ComputeShader == description2.ComputeShader) &&
				(description1.NodeMask == description2.NodeMask);
	};

	auto rhi_ray_tracing_pipeline_state_description_hash_function = [](const RHI::RayTracingPipelineStateDescription& description) {
		std::size_t hash = 0;
		//AT::Util::Hash::hash_combine(hash, description.ExportAssociations);
		AT::Util::Hash::hash_combine(hash, description.GlobalRootSignature);
		description.HitGroups;
		description.LocalRootSignatures;
		description.MaxTraceRecursionDepth;
		description.ShaderConfiguration;
		for (uint32_t i = 0u; i < description.ShaderLibraries.size(); ++i) {
			AT::Util::Hash::hash_combine(hash, description.ShaderLibraries[i].m_Shader);
		}
		return hash;
	};

	auto rhi_ray_tracing_pipeline_state_description_equality_function = [](const RHI::RayTracingPipelineStateDescription& description1, const RHI::RayTracingPipelineStateDescription& description2) {
		for (uint32_t i = 0u; i < description1.ShaderLibraries.size(); ++i) {
			if (description1.ShaderLibraries[i].m_Shader != description2.ShaderLibraries[i].m_Shader) {
				return false;
			}
		}
		return description1.GlobalRootSignature == description2.GlobalRootSignature;
	};

	class GPUPipelineStateManager {
	public:
		GPUPipelineStateManager(RHI::Device device) :
			m_Device(device)
		{

		}

		~GPUPipelineStateManager() {
			for (auto pair : m_RHIGraphicsPSOCache) {
				delete pair.second;
			}

			for (auto pair : m_RHIComputePSOCache) {
				delete pair.second;
			}
		}

		RHI::PipelineState CreateOrGetGraphicsPipelineState(const RHI::GraphicsPipelineStateDescription& description) {
			if (m_RHIGraphicsPSOCache.find(description) == m_RHIGraphicsPSOCache.end()) {
				RHI::PipelineState pipeline_state;
				m_Device->CreateGraphicsPipelineState(description, pipeline_state);
				m_RHIGraphicsPSOCache[description] = pipeline_state;
			}
			return m_RHIGraphicsPSOCache[description];
		}

		RHI::PipelineState CreateOrGetComputePipelineState(const RHI::ComputePipelineStateDescription& description) {
			if (m_RHIComputePSOCache.find(description) == m_RHIComputePSOCache.end()) {
				RHI::PipelineState pipeline_state;
				m_Device->CreateComputePipelineState(description, pipeline_state);
				m_RHIComputePSOCache[description] = pipeline_state;
			}
			return m_RHIComputePSOCache[description];
		}

		RHI::IRayTracingPipeline* CreateOrGetRayTracingPipelineState(const RHI::RayTracingPipelineStateDescription& description) {
			if (m_RHIRayTracingPSOCache.find(description) == m_RHIRayTracingPSOCache.end()) {
				RHI::IRayTracingPipeline* pipeline;
				m_Device->CreateRayTracingPipelineState(description, pipeline);
				m_RHIRayTracingPSOCache[description] = pipeline;
			}
			return m_RHIRayTracingPSOCache[description];
		}
	private:
		RHI::Device m_Device;
		std::unordered_map<RHI::GraphicsPipelineStateDescription, RHI::PipelineState, decltype(rhi_graphics_pipeline_state_description_hash_function), decltype(rhi_graphics_pipeline_state_desciption_equality_funciton)> m_RHIGraphicsPSOCache;
		std::unordered_map<RHI::ComputePipelineStateDescription, RHI::PipelineState, decltype(rhi_compute_pipeline_state_description_hash_function), decltype(rhi_compute_pipeline_state_desciption_equality_funciton)> m_RHIComputePSOCache;
		std::unordered_map<RHI::RayTracingPipelineStateDescription, RHI::IRayTracingPipeline*, decltype(rhi_ray_tracing_pipeline_state_description_hash_function), decltype(rhi_ray_tracing_pipeline_state_description_equality_function)> m_RHIRayTracingPSOCache;
	};
}
#endif