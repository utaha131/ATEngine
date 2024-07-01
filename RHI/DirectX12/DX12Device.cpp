#include "DX12Device.h"

namespace RHI::DX12 {
	DX12Device::DX12Device(ID3D12Device5* dx12_device) :
		m_DX12Device(dx12_device)
	{
		DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&m_DXLibrary));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_DXCompiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_DXUtils));
		m_DXUtils->CreateDefaultIncludeHandler(&m_DXIncludeHandler);

		//Create Offline Descriptor Heaps.
		m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = new DX12ViewAllocator(m_DX12Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256ull);
		m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = new DX12ViewAllocator(m_DX12Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256ull);
		m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = new DX12ViewAllocator(m_DX12Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024ull);
		m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = new DX12ViewAllocator(m_DX12Device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 128ull);

		//Create Online Descriptor Heaps.
		ID3D12DescriptorHeap* gpu_heap;
		D3D12_DESCRIPTOR_HEAP_DESC gpu_descriptor_heap_desc;
		gpu_descriptor_heap_desc.NodeMask = 0u;
		gpu_descriptor_heap_desc.NumDescriptors = 1000000u;
		gpu_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		gpu_descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_DX12Device->CreateDescriptorHeap(&gpu_descriptor_heap_desc, IID_PPV_ARGS(&gpu_heap));
		unsigned int gpu_heap_increment_size = m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_GPUDescriptorHeap = new DX12DescriptorHeap(gpu_heap_increment_size, gpu_heap, 1000000u);

		ID3D12DescriptorHeap* gpu_sampler_heap;
		D3D12_DESCRIPTOR_HEAP_DESC gpu_sampler_descriptor_heap_desc;
		gpu_sampler_descriptor_heap_desc.NodeMask = 0u;
		gpu_sampler_descriptor_heap_desc.NumDescriptors = 2048u;
		gpu_sampler_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		gpu_sampler_descriptor_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		m_DX12Device->CreateDescriptorHeap(&gpu_sampler_descriptor_heap_desc, IID_PPV_ARGS(&gpu_sampler_heap));
		gpu_heap_increment_size = m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		m_GPUSamplerDescriptorHeap = new DX12DescriptorHeap(gpu_heap_increment_size, gpu_sampler_heap, 2048u);

		//Create Command Queues.
		D3D12_COMMAND_QUEUE_DESC graphics_command_queue_desc = {};
		graphics_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		graphics_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		m_DX12Device->CreateCommandQueue(&graphics_command_queue_desc, IID_PPV_ARGS(&m_DX12CommandQueues[(uint32_t)RHI::CommandType::DIRECT]));

		D3D12_COMMAND_QUEUE_DESC copy_command_queue_desc = {};
		copy_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		copy_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		m_DX12Device->CreateCommandQueue(&copy_command_queue_desc, IID_PPV_ARGS(&m_DX12CommandQueues[(uint32_t)RHI::CommandType::COPY]));

		D3D12_COMMAND_QUEUE_DESC compute_command_queue_desc = {};
		compute_command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		compute_command_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		m_DX12Device->CreateCommandQueue(&compute_command_queue_desc, IID_PPV_ARGS(&m_DX12CommandQueues[(uint32_t)RHI::CommandType::COMPUTE]));
	}

	DX12Device::~DX12Device() {
		delete m_GPUDescriptorHeap;
		delete m_GPUSamplerDescriptorHeap;
		for (uint32_t i = 0u; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			delete m_ViewAllocator[i];
		}
		for (uint32_t i = 0u; i < (uint32_t)RHI::CommandType::NUM_TYPES; ++i) {
			m_DX12CommandQueues[i]->Release();
		}
		m_DX12Device->Release();
	}

	//Object Creation Interface.
	RHI::Result DX12Device::CreateResourceHeap(const RHI::ResourceHeapDescription& description, RHI::ResourceHeap& resource_heap) const {
		CD3DX12_HEAP_DESC dx12_heap_description;
		switch (description.ResourceHeapType) {
		case RHI::ResourceHeapType::DEFAULT:
			dx12_heap_description = CD3DX12_HEAP_DESC(description.Size, D3D12_HEAP_TYPE_DEFAULT);
			break;
		case RHI::ResourceHeapType::UPLOAD:
			dx12_heap_description = CD3DX12_HEAP_DESC(description.Size, D3D12_HEAP_TYPE_UPLOAD);
			break;
		}
		
		ID3D12Heap* dx12_heap;
		if (FAILED(m_DX12Device->CreateHeap(&dx12_heap_description, IID_PPV_ARGS(&dx12_heap)))) {
			return RHI::Result::E_CREATE_RESOURCE_HEAP;
		}
		resource_heap = new DX12ResourceHeap(description, dx12_heap);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateBuffer(RHI::ResourceHeap resource_heap, uint64_t offset, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const {
		ID3D12Resource* dx12_resource;
		m_DX12Device->CreatePlacedResource(static_cast<DX12ResourceHeap*>(resource_heap)->GetNative(), offset, &DX12ConvertBufferDescription(description), DX12ConvertBufferState(initial_resource_state), nullptr, IID_PPV_ARGS(&dx12_resource));
		buffer = new DX12Buffer(description, dx12_resource);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateCommittedBuffer(RHI::ResourceHeapType resource_heap_type, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const {
		D3D12_HEAP_TYPE dx12_heap_type;
		switch (resource_heap_type) {
		case RHI::ResourceHeapType::DEFAULT:
			dx12_heap_type = D3D12_HEAP_TYPE_DEFAULT;
			break;
		case RHI::ResourceHeapType::UPLOAD:
			dx12_heap_type = D3D12_HEAP_TYPE_UPLOAD;
			break;
		}
		ID3D12Resource* dx12_resource;
		m_DX12Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(dx12_heap_type), D3D12_HEAP_FLAG_NONE, &DX12ConvertBufferDescription(description), DX12ConvertBufferState(initial_resource_state), nullptr, IID_PPV_ARGS(&dx12_resource));
		buffer = new DX12Buffer(description, dx12_resource);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateTexture(RHI::ResourceHeap resource_heap, uint64_t offset, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const {
		ID3D12Resource* dx12_resource;
		D3D12_CLEAR_VALUE dx12_clear_value;
		D3D12_CLEAR_VALUE* p_dx12_clear_value = nullptr;
		if (clear_value.has_value()) {
			dx12_clear_value = DX12ConvertClearValue(clear_value.value(), description.Format);
			p_dx12_clear_value = &dx12_clear_value;
		}
		m_DX12Device->CreatePlacedResource(static_cast<DX12ResourceHeap*>(resource_heap)->GetNative(), offset, &DX12ConvertTextureDescription(description), D3D12_RESOURCE_STATE_COMMON, p_dx12_clear_value, IID_PPV_ARGS(&dx12_resource));
		texture = new DX12Texture(description, dx12_resource);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateCommittedTexture(RHI::ResourceHeapType resource_heap_type, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const {
		D3D12_HEAP_TYPE dx12_heap_type;
		switch (resource_heap_type) {
		case RHI::ResourceHeapType::DEFAULT:
			dx12_heap_type = D3D12_HEAP_TYPE_DEFAULT;
			break;
		case RHI::ResourceHeapType::UPLOAD:
			dx12_heap_type = D3D12_HEAP_TYPE_UPLOAD;
			break;
		}
		D3D12_CLEAR_VALUE dx12_clear_value;
		D3D12_CLEAR_VALUE* p_dx12_clear_value = nullptr;
		if (clear_value.has_value()) {
			dx12_clear_value = DX12ConvertClearValue(clear_value.value(), description.Format);
			p_dx12_clear_value = &dx12_clear_value;
		}
		ID3D12Resource* dx12_resource;
		m_DX12Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(dx12_heap_type), D3D12_HEAP_FLAG_NONE, &DX12ConvertTextureDescription(description), D3D12_RESOURCE_STATE_COMMON, p_dx12_clear_value, IID_PPV_ARGS(&dx12_resource));
		texture = new DX12Texture(description, dx12_resource);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateRootSignature(const RHI::RootSignatureDescription& description, RHI::RootSignature& root_signature) {
		std::vector<CD3DX12_ROOT_PARAMETER> dx12_root_parameters = std::vector<CD3DX12_ROOT_PARAMETER>(description.Parameters.size());
		std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>> dx12_descriptor_range_cache = std::vector<std::vector<CD3DX12_DESCRIPTOR_RANGE>>();
		for (uint32_t i = 0u; i < description.Parameters.size(); ++i) {
			switch (description.Parameters[i].RootParameterType) {
			case RHI::RootParameterType::CONSTANT:
				//TODO Implement Root Constant.
				break;
			case RHI::RootParameterType::DESCRIPTOR_TABLE:
			{
				dx12_descriptor_range_cache.emplace_back(std::vector<CD3DX12_DESCRIPTOR_RANGE>(description.Parameters[i].RootDescriptorTable.Ranges.size()));
				dx12_root_parameters[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				for (uint32_t j = 0u; j < description.Parameters[i].RootDescriptorTable.Ranges.size(); ++j) {
					dx12_descriptor_range_cache.back()[j].Init(DX12ConvertDescriptorRangeType(description.Parameters[i].RootDescriptorTable.Ranges[j].RangeType), description.Parameters[i].RootDescriptorTable.Ranges[j].DescriptorCount, description.Parameters[i].RootDescriptorTable.Ranges[j].ShaderRegister, description.Parameters[i].RootDescriptorTable.Ranges[j].RegisterSpace, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
				}
				dx12_root_parameters[i].InitAsDescriptorTable(dx12_descriptor_range_cache.back().size(), dx12_descriptor_range_cache.back().data(), DX12ConvertShaderVisibility(description.Parameters[i].ShaderVisibility));
			}
			break;
			}
		}
		std::vector<D3D12_STATIC_SAMPLER_DESC> dx12_static_sampler_descriptions = std::vector<D3D12_STATIC_SAMPLER_DESC>(description.StaticSamplers.size());
		for (uint32_t i = 0u; i < description.StaticSamplers.size(); ++i) {
			dx12_static_sampler_descriptions[i] = DX12ConvertStaticSamplerDescription(description.StaticSamplers[i]);
		}
		CD3DX12_ROOT_SIGNATURE_DESC dx12_root_signature_description;
		dx12_root_signature_description.Init(dx12_root_parameters.size(), dx12_root_parameters.data(), dx12_static_sampler_descriptions.size(), dx12_static_sampler_descriptions.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED);

		Microsoft::WRL::ComPtr<ID3DBlob> dx12_serialized_root_signature;
		Microsoft::WRL::ComPtr<ID3DBlob> dx12_error_blob;

		if (FAILED(D3D12SerializeRootSignature(&dx12_root_signature_description, D3D_ROOT_SIGNATURE_VERSION_1, dx12_serialized_root_signature.GetAddressOf(), dx12_error_blob.GetAddressOf()))) {
			if (dx12_error_blob != nullptr) {
				::OutputDebugStringA((char*)dx12_error_blob->GetBufferPointer());
			}
			return RHI::Result::E_CREATE_ROOT_SIGNATURE;
		}

		ID3D12RootSignature* dx12_root_signature;

		if (FAILED(m_DX12Device->CreateRootSignature(0, dx12_serialized_root_signature->GetBufferPointer(), dx12_serialized_root_signature->GetBufferSize(), IID_PPV_ARGS(&dx12_root_signature)))) {
			return RHI::Result::E_CREATE_ROOT_SIGNATURE;
		}

		root_signature = new DX12RootSignature(description, dx12_root_signature);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateShader(const RHI::ShaderDescription& description, RHI::Shader& shader) const {
		std::wstring target = L"vs_6_3";
		switch (description.ShaderType) {
		case RHI::ShaderType::PIXEL:
			target = L"ps_6_3";
			break;
		case RHI::ShaderType::COMPUTE:
			target = L"cs_6_3";
			break;
		case RHI::ShaderType::LIBRARY:
			target = L"lib_6_6";
			break;
		}

		IDxcBlob* p_native_shader = nullptr;
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> errors;
		std::wstring source_path = std::wstring(description.SourcePath.begin(), description.SourcePath.end());
		IDxcBlobEncoding* source_blob;
		uint32_t code_page = CP_UTF8;
		HRESULT hr;

		hr = m_DXLibrary->CreateBlobFromFile(source_path.c_str(), &code_page, &source_blob);
		if (FAILED(hr)) {
			std::cout << " fialed to load file" << std::endl;
		}

		const wchar_t* arguments[5] = {
			 L"-I", 
			 L"./shaders/hlsl/",
			 DXC_ARG_OPTIMIZATION_LEVEL3,
			 L"-HV",
			 L"2021"
		};

		Microsoft::WRL::ComPtr<IDxcOperationResult> result;
		hr = m_DXCompiler->Compile(
			*(&source_blob), // pSource
			source_path.c_str(), // pSourceName
			std::wstring(description.EntryPoint.begin(), description.EntryPoint.end()).c_str(),
			target.c_str(), // pTargetProfile
			arguments, 5,//NULL, 0, // pArguments, argCount
			NULL, 0, // pDefines, defineCount
			m_DXIncludeHandler.Get(), // pIncludeHandler
			&result
		);
		if (SUCCEEDED(hr))
			result->GetStatus(&hr);
		if (FAILED(hr)) {
			if (result) {
				Microsoft::WRL::ComPtr<IDxcBlobEncoding> errorsBlob;
				hr = result->GetErrorBuffer(&errorsBlob);
				if (SUCCEEDED(hr) && errorsBlob) {
					auto c = (const char*)errorsBlob->GetBufferPointer();
					std::string s = std::string(c);
					OutputDebugString((L"Compilation failed with errors:\n%hs\n" + std::wstring(s.begin(), s.end())).c_str());
					;
				}
			}
		}
		result->GetResult(&p_native_shader);
		shader = new DX12Shader(description, *(&p_native_shader));
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateRenderPass(const RHI::RenderPassDescription& description, RHI::RenderPass& render_pass) const {
		render_pass = new IRenderPass(description);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateFence(uint64_t initial_value, RHI::Fence& fence) const {
		ID3D12Fence* dx12_fence;
		m_DX12Device->CreateFence(0ull, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12_fence));
		fence = new DX12Fence(dx12_fence);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateFrameBuffer(const RHI::FrameBufferDescription& description, RHI::FrameBuffer& frame_buffer) const {
		frame_buffer = new DX12FrameBuffer(description);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateGraphicsPipelineState(const RHI::GraphicsPipelineStateDescription& description, RHI::PipelineState& pipeline_state) const {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC dx12_graphics_pipeline_state_description;

		//Create Pipeline State Description
		ZeroMemory(&dx12_graphics_pipeline_state_description, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		dx12_graphics_pipeline_state_description.pRootSignature = static_cast<DX12RootSignature*>(description.RootSignature)->GetNative();
		if (description.VertexShader != RHI_NULL_HANDLE) {
			dx12_graphics_pipeline_state_description.VS = static_cast<DX12Shader*>(description.VertexShader)->GetByteCode();
		}
		if (description.PixelShader != RHI_NULL_HANDLE) {
			dx12_graphics_pipeline_state_description.PS = static_cast<DX12Shader*>(description.PixelShader)->GetByteCode();
		}
		if (description.DomainShader != RHI_NULL_HANDLE) {
			dx12_graphics_pipeline_state_description.DS = static_cast<DX12Shader*>(description.DomainShader)->GetByteCode();
		}
		if (description.HullShader != RHI_NULL_HANDLE) {
			dx12_graphics_pipeline_state_description.HS = static_cast<DX12Shader*>(description.HullShader)->GetByteCode();
		}
		if (description.GeometryShader != RHI_NULL_HANDLE) {
			dx12_graphics_pipeline_state_description.GS = static_cast<DX12Shader*>(description.GeometryShader)->GetByteCode();
		}

		//fill out Blend State.
		dx12_graphics_pipeline_state_description.BlendState = {};
		dx12_graphics_pipeline_state_description.BlendState.AlphaToCoverageEnable = description.BlendDescription.AlphaToCoverageEnable ? TRUE : FALSE;
		dx12_graphics_pipeline_state_description.BlendState.IndependentBlendEnable = description.BlendDescription.IndependentBlendEnable ? TRUE : FALSE;
		for (uint32_t i = 0u; i < 8u; ++i) {
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i] = {};
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].BlendEnable = description.BlendDescription.RenderTargetBlendDescription[i].BlendEnable ? TRUE : FALSE;
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].LogicOpEnable = description.BlendDescription.RenderTargetBlendDescription[i].BlendLogicEnable ? TRUE : FALSE;
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].SrcBlend = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].SourceBlend);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].DestBlend = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].BlendOp = DX12ConvertBlendOperation(description.BlendDescription.RenderTargetBlendDescription[i].BlendOperation);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].SrcBlendAlpha = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].DestBlendAlpha = DX12ConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].BlendOpAlpha = DX12ConvertBlendOperation(description.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].LogicOp = DX12ConvertLogicOperation(description.BlendDescription.RenderTargetBlendDescription[i].LogicOperation);
			dx12_graphics_pipeline_state_description.BlendState.RenderTarget[i].RenderTargetWriteMask = DX12ConvertColorWriteMask(description.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask);
		}

		dx12_graphics_pipeline_state_description.SampleMask = UINT_MAX;

		//fill out Rasterizer State.
		dx12_graphics_pipeline_state_description.RasterizerState = {};
		dx12_graphics_pipeline_state_description.RasterizerState.FillMode = DX12ConvertFillMode(description.RasterizerDescription.FillMode);
		dx12_graphics_pipeline_state_description.RasterizerState.CullMode = DX12ConvertCullMode(description.RasterizerDescription.CullMode);
		dx12_graphics_pipeline_state_description.RasterizerState.FrontCounterClockwise = description.RasterizerDescription.FrontCounterClockwise ? TRUE : FALSE;
		dx12_graphics_pipeline_state_description.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		dx12_graphics_pipeline_state_description.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		dx12_graphics_pipeline_state_description.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		dx12_graphics_pipeline_state_description.RasterizerState.DepthClipEnable = TRUE;
		dx12_graphics_pipeline_state_description.RasterizerState.MultisampleEnable = FALSE;
		dx12_graphics_pipeline_state_description.RasterizerState.AntialiasedLineEnable = FALSE;
		dx12_graphics_pipeline_state_description.RasterizerState.ForcedSampleCount = 0u;
		dx12_graphics_pipeline_state_description.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		dx12_graphics_pipeline_state_description.DepthStencilState = {};
		dx12_graphics_pipeline_state_description.DepthStencilState.DepthEnable = description.DepthStencilDescription.DepthEnabled ? TRUE : FALSE;
		dx12_graphics_pipeline_state_description.DepthStencilState.DepthWriteMask = DX12ConvertDepthWriteMask(description.DepthStencilDescription.DepthWriteMask);
		dx12_graphics_pipeline_state_description.DepthStencilState.DepthFunc = DX12ConvertComparisonFunction(description.DepthStencilDescription.ComparisonFunction);
		dx12_graphics_pipeline_state_description.DepthStencilState.StencilEnable = description.DepthStencilDescription.StencilEnabled ? TRUE : FALSE;
		dx12_graphics_pipeline_state_description.DepthStencilState.StencilReadMask = description.DepthStencilDescription.StencilReadMask;
		dx12_graphics_pipeline_state_description.DepthStencilState.StencilWriteMask = description.DepthStencilDescription.StencilWriteMask;
		dx12_graphics_pipeline_state_description.DepthStencilState.FrontFace = {}; //TODO
		dx12_graphics_pipeline_state_description.DepthStencilState.BackFace = {}; //TODO

		//fill out Input Layout.
		std::vector<D3D12_INPUT_ELEMENT_DESC> dx12_input_layout = std::vector<D3D12_INPUT_ELEMENT_DESC>(description.InputLayout.InputElements.size());
		for (uint32_t i = 0u; i < dx12_input_layout.size(); ++i) {
			dx12_input_layout[i] = { description.InputLayout.InputElements[i].Name.c_str(), 0, DX12ConvertFormat(description.InputLayout.InputElements[i].Format), 0, description.InputLayout.InputElements[i].Offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		}
		dx12_graphics_pipeline_state_description.InputLayout = { dx12_input_layout.data(), (UINT)dx12_input_layout.size() };
		dx12_graphics_pipeline_state_description.PrimitiveTopologyType = DX12ConvertPrimitiveTopologyType(description.PrimitiveTopology);
		dx12_graphics_pipeline_state_description.NumRenderTargets = description.NumberOfRenderTargets;
		for (uint32_t i = 0u; i < description.NumberOfRenderTargets; ++i) {
			dx12_graphics_pipeline_state_description.RTVFormats[i] = DX12ConvertFormat(description.RenderTargetFormats[i]);
		}
		dx12_graphics_pipeline_state_description.DSVFormat = DX12ConvertFormat(description.DepthFormat);
		dx12_graphics_pipeline_state_description.SampleDesc = {};
		dx12_graphics_pipeline_state_description.SampleDesc.Count = description.SampleDescription.Count;
		dx12_graphics_pipeline_state_description.SampleDesc.Quality = description.SampleDescription.Quality;
		dx12_graphics_pipeline_state_description.NodeMask = description.NodeMask;
		dx12_graphics_pipeline_state_description.CachedPSO = {};
		dx12_graphics_pipeline_state_description.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		//
		ID3D12PipelineState* dx12_pipeline_state;
		if (FAILED(m_DX12Device->CreateGraphicsPipelineState(&dx12_graphics_pipeline_state_description, IID_PPV_ARGS(&dx12_pipeline_state)))) {
			return RHI::Result::E_CREATE_PIPELINE_STATE;
		}
		pipeline_state = new DX12PipelineState(description, dx12_pipeline_state);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateComputePipelineState(const RHI::ComputePipelineStateDescription& description, RHI::PipelineState& pipeline_state) const {
		D3D12_COMPUTE_PIPELINE_STATE_DESC dx12_compute_pipeline_state_desc = DX12ConvertComputePipelineStateDescription(description);
		ID3D12PipelineState* dx12_pipeline_state;
		m_DX12Device->CreateComputePipelineState(&dx12_compute_pipeline_state_desc, IID_PPV_ARGS(&dx12_pipeline_state));
		pipeline_state = new DX12PipelineState(description, dx12_pipeline_state);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateConstantBufferView(const RHI::ConstantBufferViewDescription& description, RHI::ConstantBufferView& constant_buffer_view) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC dx12_constant_buffer_view_description = DX12ConvertConstantBufferViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->AllocateHandle();
		m_DX12Device->CreateConstantBufferView(&dx12_constant_buffer_view_description, dx12_cpu_handle);
		constant_buffer_view = new DX12ConstantBufferView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], dx12_cpu_handle);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateDepthStencilView(const RHI::Texture texture, const RHI::DepthStencilViewDescription& description, RHI::DepthStencilView& depth_stencil_view) {
		D3D12_DEPTH_STENCIL_VIEW_DESC dx12_depth_stencil_view_description = DX12ConvertDepthStencilViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_DSV]->AllocateHandle();
		m_DX12Device->CreateDepthStencilView(static_cast<DX12Texture*>(texture)->GetNative(), &dx12_depth_stencil_view_description, dx12_cpu_handle);
		depth_stencil_view = new DX12DepthStencilView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_DSV], dx12_cpu_handle, static_cast<DX12Texture*>(texture));
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateRenderTargetView(const RHI::Buffer buffer, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) {
		D3D12_RENDER_TARGET_VIEW_DESC dx12_render_target_view_description = DX12ConvertRenderTargetViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->AllocateHandle();
		m_DX12Device->CreateRenderTargetView(static_cast<DX12Buffer*>(buffer)->GetNative(), &dx12_render_target_view_description, dx12_cpu_handle);
		render_target_view = new DX12RenderTargetView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], dx12_cpu_handle, RHI_NULL_HANDLE);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateRenderTargetView(const RHI::Texture texture, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) {
		D3D12_RENDER_TARGET_VIEW_DESC dx12_render_target_view_description = DX12ConvertRenderTargetViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_RTV]->AllocateHandle();
		m_DX12Device->CreateRenderTargetView(static_cast<DX12Texture*>(texture)->GetNative(), &dx12_render_target_view_description, dx12_cpu_handle);
		render_target_view = new DX12RenderTargetView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_RTV], dx12_cpu_handle, static_cast<DX12Texture*>(texture));
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateSampler(const RHI::SamplerDescription& description, RHI::Sampler& sampler) {
		D3D12_SAMPLER_DESC dx12_sampler_description = DX12ConvertSamplerDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]->AllocateHandle();
		m_DX12Device->CreateSampler(&dx12_sampler_description, dx12_cpu_handle);
		sampler = new DX12Sampler(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER], dx12_cpu_handle);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateShaderResourceView(const RHI::Buffer buffer, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) {
		D3D12_SHADER_RESOURCE_VIEW_DESC dx12_shader_resource_view_description = DX12ConvertShaderResourceViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->AllocateHandle();
		m_DX12Device->CreateShaderResourceView(static_cast<DX12Buffer*>(buffer)->GetNative(), &dx12_shader_resource_view_description, dx12_cpu_handle);
		shader_resource_view = new DX12ShaderResourceView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], dx12_cpu_handle);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateShaderResourceView(const RHI::Texture texture, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) {
		D3D12_SHADER_RESOURCE_VIEW_DESC dx12_shader_resource_view_description = DX12ConvertShaderResourceViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->AllocateHandle();
		m_DX12Device->CreateShaderResourceView(static_cast<DX12Texture*>(texture)->GetNative(), &dx12_shader_resource_view_description, dx12_cpu_handle);
		shader_resource_view = new DX12ShaderResourceView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], dx12_cpu_handle);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateUnorderedAccessView(const RHI::Buffer buffer, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC dx12_unordered_access_view_description = DX12ConvertUnorderedAccessViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->AllocateHandle();
		m_DX12Device->CreateUnorderedAccessView(static_cast<DX12Buffer*>(buffer)->GetNative(), nullptr, &dx12_unordered_access_view_description, dx12_cpu_handle);
		unordered_access_view = new DX12UnorderedAccessView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], dx12_cpu_handle);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateUnorderedAccessView(const RHI::Texture texture, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC dx12_unordered_access_view_description = DX12ConvertUnorderedAccessViewDescription(description);
		D3D12_CPU_DESCRIPTOR_HANDLE dx12_cpu_handle = m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->AllocateHandle();
		m_DX12Device->CreateUnorderedAccessView(static_cast<DX12Texture*>(texture)->GetNative(), nullptr, &dx12_unordered_access_view_description, dx12_cpu_handle);
		unordered_access_view = new DX12UnorderedAccessView(description, *m_ViewAllocator[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], dx12_cpu_handle);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateCommandAllocator(RHI::CommandType command_type, RHI::CommandAllocator& command_allocator) const {
		ID3D12CommandAllocator* dx12_command_allocator;
		D3D12_COMMAND_LIST_TYPE dx12_command_type;
		switch (command_type) {
		case RHI::CommandType::DIRECT:
			dx12_command_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			break;
		case RHI::CommandType::COPY:
			dx12_command_type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		case RHI::CommandType::COMPUTE:
			dx12_command_type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		}
		m_DX12Device->CreateCommandAllocator(dx12_command_type, IID_PPV_ARGS(&dx12_command_allocator));
		command_allocator = new DX12CommandAllocator(command_type, dx12_command_allocator);
		return RHI::Result::SUCCESS;
	};

	RHI::Result DX12Device::CreateCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, RHI::CommandList& command_list) const {
		ID3D12GraphicsCommandList4* dx12_command_list;
		D3D12_COMMAND_LIST_TYPE dx12_command_type;
		switch (command_type) {
		case RHI::CommandType::DIRECT:
			dx12_command_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			break;
		case RHI::CommandType::COPY:
			dx12_command_type = D3D12_COMMAND_LIST_TYPE_COPY;
			break;
		case RHI::CommandType::COMPUTE:
			dx12_command_type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
			break;
		}
		m_DX12Device->CreateCommandList(0u, dx12_command_type, static_cast<DX12CommandAllocator*>(command_allocator)->GetNative(), nullptr, IID_PPV_ARGS(&dx12_command_list));
		dx12_command_list->Close();
		command_list = new DX12CommandList(command_type, command_allocator, dx12_command_list, { m_GPUDescriptorHeap->GetNative(), m_GPUSamplerDescriptorHeap->GetNative() });
		return RHI::Result::SUCCESS;
	};

	//Execution Interface.
	void DX12Device::ExecuteCommandList(RHI::CommandType command_type, uint32_t command_list_count, const RHI::CommandList* p_command_lists) {
		std::vector<ID3D12CommandList*> dx12_command_lists = std::vector<ID3D12CommandList*>(command_list_count);
		for (uint32_t i = 0u; i < command_list_count; ++i) {
			dx12_command_lists[i] = static_cast<DX12CommandList*>(p_command_lists[i])->GetNative();
		}
		m_DX12CommandQueues[(uint32_t)command_type]->ExecuteCommandLists(command_list_count, dx12_command_lists.data());
	};

	void DX12Device::SignalQueue(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) {
		m_DX12CommandQueues[(uint32_t)command_type]->Signal(static_cast<DX12Fence*>(fence)->GetNative(), fence_value);
	};

	void DX12Device::HostWait(RHI::Fence fence, uint64_t fence_value) {
		if (static_cast<DX12Fence*>(fence)->GetCompletedValue() < fence_value) {
			HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(static_cast<DX12Fence*>(fence)->SetEventOnCompletion(fence_value, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}
	};

	void DX12Device::QueueWait(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) {
		m_DX12CommandQueues[(uint32_t)command_type]->Wait(static_cast<DX12Fence*>(fence)->GetNative(), fence_value);
	};

	//Other Interfaces.
	RHI::AllocationInfo DX12Device::GetResourceAllocationInfo(const RHI::BufferDescription& buffer_description) const {
		D3D12_RESOURCE_ALLOCATION_INFO dx12_allocation_info = m_DX12Device->GetResourceAllocationInfo(0u, 1u, &DX12ConvertBufferDescription(buffer_description));
		RHI::AllocationInfo allocation_info;
		allocation_info.Size = dx12_allocation_info.SizeInBytes;
		allocation_info.Alignment = dx12_allocation_info.Alignment;
		return allocation_info;
	};

	RHI::AllocationInfo DX12Device::GetResourceAllocationInfo(const RHI::TextureDescription& texture_description) const {
		D3D12_RESOURCE_ALLOCATION_INFO dx12_allocation_info = m_DX12Device->GetResourceAllocationInfo(0u, 1u, &DX12ConvertTextureDescription(texture_description));
		RHI::AllocationInfo allocation_info;
		allocation_info.Size = dx12_allocation_info.SizeInBytes;
		allocation_info.Alignment = dx12_allocation_info.Alignment;
		return allocation_info;
	};

	uint32_t DX12Device::GetTexturePitchAlignment() const {
		return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
	};

	void DX12Device::AllocateDescriptorTable(const RHI::RootSignature root_signature, uint32_t parameter_index, RHI::DescriptorTable& table) {
		table = m_GPUDescriptorHeap->AllocateTable(root_signature->GetDescription().Parameters[parameter_index].RootDescriptorTable);
	};

	void DX12Device::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ConstantBufferView* p_cbv) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE dx12_destination_cpu_handle{ static_cast<DX12DescriptorTable*>(descriptor_table)->GetCPUHandle() };
		dx12_destination_cpu_handle.Offset(offset, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		for (uint32_t i = 0u; i < count; ++i) {
			m_DX12Device->CopyDescriptorsSimple(1u, dx12_destination_cpu_handle, static_cast<DX12ConstantBufferView*>(p_cbv[i])->GetNative(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			dx12_destination_cpu_handle.Offset(1, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		}
	};

	void DX12Device::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::Sampler* p_sampler) {
		//TODO Sampler Support.
	};

	void DX12Device::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ShaderResourceView* p_srv) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE dx12_destination_cpu_handle{ static_cast<DX12DescriptorTable*>(descriptor_table)->GetCPUHandle() };
		dx12_destination_cpu_handle.Offset(offset, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		for (uint32_t i = 0u; i < count; ++i) {
			m_DX12Device->CopyDescriptorsSimple(1u, dx12_destination_cpu_handle, static_cast<DX12ShaderResourceView*>(p_srv[i])->GetNative(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			dx12_destination_cpu_handle.Offset(1, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		}
	};

	void DX12Device::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::UnorderedAccessView* p_uav) {
		CD3DX12_CPU_DESCRIPTOR_HANDLE dx12_destination_cpu_handle{ static_cast<DX12DescriptorTable*>(descriptor_table)->GetCPUHandle() };
		dx12_destination_cpu_handle.Offset(offset, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		for (uint32_t i = 0u; i < count; ++i) {
			m_DX12Device->CopyDescriptorsSimple(1u, dx12_destination_cpu_handle, static_cast<DX12UnorderedAccessView*>(p_uav[i])->GetNative(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			dx12_destination_cpu_handle.Offset(1, m_DX12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		}
	};
}