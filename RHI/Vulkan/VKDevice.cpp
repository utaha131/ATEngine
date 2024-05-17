#include "VKDevice.h"
namespace RHI::VK {
	std::vector<char> read_file(const std::string& file_name) {
		std::ifstream file = std::ifstream(file_name, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			std::cout << "Failed to Read File." << std::endl;
		}

		size_t file_size = (size_t)file.tellg();
		std::vector<char> buffer = std::vector<char>(file_size);
		file.seekg(0);
		file.read(buffer.data(), file_size);
		file.close();
		return buffer;
	}

	VKDevice::VKDevice(VkDevice vk_device, VkPhysicalDevice vk_physical_device) :
		m_VkDevice(vk_device),
		m_VkPhysicalDevice(vk_physical_device)
	{
		VkPhysicalDeviceMemoryProperties vk_memory_properties;
		vkGetPhysicalDeviceMemoryProperties(m_VkPhysicalDevice, &vk_memory_properties);
		
		for (uint32_t i = 0u; i < vk_memory_properties.memoryTypeCount; ++i) {
			if (vk_memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
				m_ResourceHeapTypes[RHI::ResourceHeapType::DEFAULT] = i;
				break;
			}
		}

		for (uint32_t i = 0u; i < vk_memory_properties.memoryTypeCount; ++i) {
			if (vk_memory_properties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
				m_ResourceHeapTypes[RHI::ResourceHeapType::UPLOAD] = i;
				break;
			}
		}

		for (uint32_t i = 0u; i < (uint32_t)RHI::CommandType::NUM_TYPES; ++i) {
			VkQueue vk_queue;
			vkGetDeviceQueue(m_VkDevice, 0, i, &vk_queue);

			VkSemaphoreTypeCreateInfo vk_timeline_semaphore_create_info = {};
			vk_timeline_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
			vk_timeline_semaphore_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
			vk_timeline_semaphore_create_info.initialValue = 0;
			VkSemaphoreCreateInfo vk_semaphore_create_info = {};
			vk_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			vk_semaphore_create_info.pNext = &vk_timeline_semaphore_create_info;
			VkSemaphore vk_semaphore;
			vkCreateSemaphore(m_VkDevice, &vk_semaphore_create_info, VK_NULL_HANDLE, &vk_semaphore);
			m_VKCommandQueues[i] = new VKCommandQueue(m_VkDevice, vk_semaphore, vk_queue);
			m_command_queue_family_indices[i] = 0;
		}
	}

	VKDevice::~VKDevice() {
		for (auto& i : m_DescriptorHeaps) {
			delete i.second;
		}
			
		for (uint32_t i = 0u; i < (uint32_t)RHI::CommandType::NUM_TYPES; ++i) {
			delete m_VKCommandQueues[i];
		}
			
		for (auto pair : m_VkStaticSamplerCache) {
			vkDestroySampler(m_VkDevice, pair.second, VK_NULL_HANDLE);
		}
		vkDestroyDevice(m_VkDevice, RHI_NULL_HANDLE);
	}

	RHI::Result VKDevice::CreateResourceHeap(const RHI::ResourceHeapDescription& description, RHI::ResourceHeap& resource_heap) const {
		VkMemoryRequirements vk_memory_requirements;
		VkMemoryAllocateInfo vk_memory_allocation_info = {};
		vk_memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vk_memory_allocation_info.allocationSize = description.Size;
		vk_memory_allocation_info.memoryTypeIndex = m_ResourceHeapTypes.at(description.ResourceHeapType);

		VkDeviceMemory vk_device_memory;
		if (vkAllocateMemory(m_VkDevice, &vk_memory_allocation_info, VK_NULL_HANDLE, &vk_device_memory) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_RESOURCE_HEAP;
		}

		resource_heap = new VKResourceHeap(description, m_VkDevice, vk_device_memory);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateBuffer(RHI::ResourceHeap resource_heap, uint64_t offset, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const {
		VkBufferCreateInfo vk_buffer_create_info = VKConvertBufferDescription(description);
		VkBuffer vk_buffer;
		if (vkCreateBuffer(m_VkDevice, &vk_buffer_create_info, VK_NULL_HANDLE, &vk_buffer) != VK_SUCCESS || vkBindBufferMemory(m_VkDevice, vk_buffer, static_cast<VKResourceHeap*>(resource_heap)->GetNative(), offset) != VK_SUCCESS) {
		    return RHI::Result::E_CREATE_BUFFER;
		}
		VKResourceHeap& test = *(static_cast<VKResourceHeap*>(resource_heap));
		buffer = new VKBuffer(description, m_VkDevice, test, offset, false, vk_buffer);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateCommittedBuffer(RHI::ResourceHeapType resource_heap_type, RHI::BufferState initial_resource_state, const RHI::BufferDescription& description, RHI::Buffer& buffer) const {
		VkBufferCreateInfo vk_buffer_create_info = VKConvertBufferDescription(description);
		VkBuffer vk_buffer;
		if (vkCreateBuffer(m_VkDevice, &vk_buffer_create_info, VK_NULL_HANDLE, &vk_buffer) != VK_SUCCESS) {
		    return RHI::Result::E_CREATE_BUFFER;
		}
		
		VkMemoryDedicatedRequirementsKHR vk_memory_dedicated_requirements = {};
		vk_memory_dedicated_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR;
		VkMemoryRequirements2 vk_memory_requirements = {};
		vk_memory_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		vk_memory_requirements.pNext = &vk_memory_dedicated_requirements;

		VkBufferMemoryRequirementsInfo2 vk_buffer_memory_requirements_info = {};
		vk_buffer_memory_requirements_info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
		vk_buffer_memory_requirements_info.buffer = vk_buffer;
		vkGetBufferMemoryRequirements2(m_VkDevice, &vk_buffer_memory_requirements_info, &vk_memory_requirements);
		
		VkMemoryDedicatedAllocateInfoKHR vk_memory_dedicated_allocate_info = {};
		vk_memory_dedicated_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
		vk_memory_dedicated_allocate_info.buffer = vk_buffer;
		
		VkMemoryAllocateInfo vk_memory_allocate_info = {};
		vk_memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		vk_memory_allocate_info.pNext = &vk_memory_dedicated_allocate_info;
		vk_memory_allocate_info.allocationSize = vk_memory_requirements.memoryRequirements.size;
		vk_memory_allocate_info.memoryTypeIndex = m_ResourceHeapTypes.at(resource_heap_type);
		VkDeviceMemory vk_device_memory;
		if (vkAllocateMemory(m_VkDevice, &vk_memory_allocate_info, NULL, &vk_device_memory) != VK_SUCCESS) {
		    return RHI::Result::E_CREATE_BUFFER;
		}
		if (vkBindBufferMemory(m_VkDevice, vk_buffer, vk_device_memory, 0) != VK_SUCCESS) {
		    return RHI::Result::E_CREATE_BUFFER;
		}

		RHI::ResourceHeapDescription resource_heap_description;
		resource_heap_description.ResourceHeapType = resource_heap_type;
		resource_heap_description.Size = vk_memory_allocate_info.allocationSize;
		VKResourceHeap* dedicated_heap = new VKResourceHeap(resource_heap_description, m_VkDevice, vk_device_memory);
		buffer = new VKBuffer(description, m_VkDevice, *dedicated_heap, 0, true, vk_buffer);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateTexture(RHI::ResourceHeap resource_heap, uint64_t offset, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const {
		VkImageCreateInfo vk_image_create_info = VKConvertTextureDescription(description);
		VkImage vk_image;
		if (vkCreateImage(m_VkDevice, &vk_image_create_info, VK_NULL_HANDLE, &vk_image) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_TEXTURE;
		}
		if (vkBindImageMemory(m_VkDevice, vk_image, static_cast<VKResourceHeap*>(resource_heap)->GetNative(), offset) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_TEXTURE;
		}
		texture = new VKTexture(description, m_VkDevice, *static_cast<VKResourceHeap*>(resource_heap), offset, false, vk_image);
		return RHI::Result::SUCCESS;	
	}

	RHI::Result VKDevice::CreateCommittedTexture(RHI::ResourceHeapType resource_heap_type, std::optional<RHI::TextureClearValue> clear_value, const RHI::TextureDescription& description, RHI::Texture& texture) const {
		VkImageCreateInfo vk_image_create_info = VKConvertTextureDescription(description);
		VkImage vk_image;
		if (vkCreateImage(m_VkDevice, &vk_image_create_info, VK_NULL_HANDLE, &vk_image) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_TEXTURE;
		}

		VkMemoryDedicatedRequirementsKHR vk_memory_dedicated_requirements = {};
		vk_memory_dedicated_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR;
		VkMemoryRequirements2 vk_memory_requirements = {};
		vk_memory_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		vk_memory_requirements.pNext = &vk_memory_dedicated_requirements;
		VkImageMemoryRequirementsInfo2 vk_image_memory_requirements = {};
		vk_image_memory_requirements.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
		vk_image_memory_requirements.image = vk_image;
		vkGetImageMemoryRequirements2(m_VkDevice, &vk_image_memory_requirements, &vk_memory_requirements);
		VkMemoryDedicatedAllocateInfoKHR vk_memory_dedicated_allocate_info = {};
		vk_memory_dedicated_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR;
		vk_memory_dedicated_allocate_info.image = vk_image;

		VkMemoryAllocateInfo vk_memory_allocation_info = {};
		vk_memory_allocation_info.sType;
		vk_memory_allocation_info.pNext = &vk_memory_dedicated_allocate_info;
		vk_memory_allocation_info.allocationSize = vk_memory_requirements.memoryRequirements.size;
		vk_memory_allocation_info.memoryTypeIndex = m_ResourceHeapTypes.at(resource_heap_type);
		VkDeviceMemory vk_device_memory;

		if (vkAllocateMemory(m_VkDevice, &vk_memory_allocation_info, VK_NULL_HANDLE, &vk_device_memory) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_TEXTURE;
		}
		if (vkBindImageMemory(m_VkDevice, vk_image, vk_device_memory, 0) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_TEXTURE;
		}
		RHI::ResourceHeapDescription resource_heap_description;
		resource_heap_description.ResourceHeapType = resource_heap_type;
		resource_heap_description.Size = vk_memory_allocation_info.allocationSize;
		VKResourceHeap* dedicated_heap = new VKResourceHeap(resource_heap_description, m_VkDevice, vk_device_memory);
		texture = new VKTexture(description, m_VkDevice, *dedicated_heap, 0, true, vk_image);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateRootSignature(const RHI::RootSignatureDescription& description, RHI::RootSignature& root_signature) {
		VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {};
		vk_pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkPushConstantRange> push_constants = std::vector<VkPushConstantRange>();
		std::vector<VkDescriptorSetLayout> vk_descriptor_set_layouts = std::vector<VkDescriptorSetLayout>();

		for (uint32_t i = 0; i < description.Parameters.size(); ++i) {
			const RHI::RootParameter& parameter = description.Parameters[i];
			unsigned int constant_offset = 0;
			switch (parameter.RootParameterType) {
			case RHI::RootParameterType::CONSTANT:
				{
					const RHI::RootConstant& root_constant = parameter.RootConstant;
					VkPushConstantRange push_constant_range = {};
					push_constant_range.offset = constant_offset;
					push_constant_range.size = root_constant.ConstantCount * 4;
					push_constant_range.stageFlags = VK_SHADER_STAGE_ALL;
					push_constants.push_back(push_constant_range);
					//constant_offset = push_constant_range.size;
				}
				break;
			case RHI::RootParameterType::DESCRIPTOR_TABLE:
				vk_descriptor_set_layouts.push_back(VKConvertRootDescriptorTableToSetLayout(description.Parameters[i].RootDescriptorTable, description.StaticSamplers));
				break;
			}
		}
		vk_pipeline_layout_create_info.pushConstantRangeCount = push_constants.size();
		vk_pipeline_layout_create_info.pPushConstantRanges = push_constants.data();
		vk_pipeline_layout_create_info.setLayoutCount = vk_descriptor_set_layouts.size();
		vk_pipeline_layout_create_info.pSetLayouts = vk_descriptor_set_layouts.data();

		VkPipelineLayout vk_pipeline_layout;
		if (vkCreatePipelineLayout(m_VkDevice, &vk_pipeline_layout_create_info, VK_NULL_HANDLE, &vk_pipeline_layout) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_ROOT_SIGNATURE;
		}

		root_signature = new VKRootSignature(description, m_VkDevice, vk_pipeline_layout);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateShader(const RHI::ShaderDescription& description, RHI::Shader& shader) const {
		std::vector<char> shader_source = read_file(description.SourcePath);

		VkShaderModuleCreateInfo vk_shader_module_create_info = {};
		vk_shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vk_shader_module_create_info.codeSize = shader_source.size();
		vk_shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader_source.data());

		VkShaderModule vk_shader_module;
		if (vkCreateShaderModule(m_VkDevice, &vk_shader_module_create_info, VK_NULL_HANDLE, &vk_shader_module) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_SHADER;
		}
		shader = new VKShader(description, m_VkDevice, vk_shader_module);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateRenderPass(const RHI::RenderPassDescription& description, RHI::RenderPass& render_pass) const {
		render_pass = new IRenderPass(description);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateFence(uint64_t initial_value, RHI::Fence& fence) const {
		VkSemaphoreTypeCreateInfo vk_semaphore_type_create_info = {};
		vk_semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		vk_semaphore_type_create_info.pNext = VK_NULL_HANDLE;
		vk_semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		vk_semaphore_type_create_info.initialValue = initial_value;
		VkSemaphoreCreateInfo vk_semaphore_create_info = {};
		vk_semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vk_semaphore_create_info.pNext = &vk_semaphore_type_create_info;
		vk_semaphore_create_info.flags = 0;

		VkSemaphore vk_semaphore;
		vkCreateSemaphore(m_VkDevice, &vk_semaphore_create_info, VK_NULL_HANDLE, &vk_semaphore);
		fence = new VKFence(m_VkDevice, vk_semaphore);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateFrameBuffer(const RHI::FrameBufferDescription& description, RHI::FrameBuffer& frame_buffer) const {
		frame_buffer = new VKFrameBuffer(description);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateGraphicsPipelineState(const RHI::GraphicsPipelineStateDescription& description, RHI::PipelineState& pipeline_state) const {
		std::vector<VkPipelineShaderStageCreateInfo> vk_pipeline_shader_stage_create_infos = std::vector<VkPipelineShaderStageCreateInfo>();
		if (description.VertexShader != VK_NULL_HANDLE) {
			vk_pipeline_shader_stage_create_infos.push_back(static_cast<VKShader*>(description.VertexShader)->GetNative());
		}
		if (description.PixelShader != VK_NULL_HANDLE) {
			vk_pipeline_shader_stage_create_infos.push_back(static_cast<VKShader*>(description.PixelShader)->GetNative());
		}
		if (description.DomainShader != VK_NULL_HANDLE) {
			vk_pipeline_shader_stage_create_infos.push_back(static_cast<VKShader*>(description.DomainShader)->GetNative());
		}
		if (description.HullShader != VK_NULL_HANDLE) {
			vk_pipeline_shader_stage_create_infos.push_back(static_cast<VKShader*>(description.HullShader)->GetNative());
		}
		if (description.GeometryShader != VK_NULL_HANDLE) {
			vk_pipeline_shader_stage_create_infos.push_back(static_cast<VKShader*>(description.GeometryShader)->GetNative());
		}
    
		//Assemble Input
		VkPipelineVertexInputStateCreateInfo vk_pipeline_vertex_input_state_create_info = {};
		vk_pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::vector<VkVertexInputAttributeDescription> vk_vertex_input_attribute_descriptions = std::vector<VkVertexInputAttributeDescription>(description.InputLayout.InputElements.size());
		for (uint32_t i = 0; i < description.InputLayout.InputElements.size(); ++i) {
			RHI::InputElement input_element = description.InputLayout.InputElements[i];
			vk_vertex_input_attribute_descriptions[i].binding = 0;
			vk_vertex_input_attribute_descriptions[i].location = i;
			vk_vertex_input_attribute_descriptions[i].format = VKConvertFormat(input_element.Format);
			vk_vertex_input_attribute_descriptions[i].offset = input_element.Offset;
		}
		vk_pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = vk_vertex_input_attribute_descriptions.size();
		vk_pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vk_vertex_input_attribute_descriptions.data();

		VkVertexInputBindingDescription input_binding_desc = {};
		input_binding_desc.binding = 0;
		input_binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		input_binding_desc.stride = description.InputLayout.InputStride;
		vk_pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = (vk_pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount > 0) ? 1 : 0; //TODO change this.
		vk_pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &input_binding_desc;

		VkPipelineInputAssemblyStateCreateInfo vk_pipline_input_assembly_state_create_info = {};
		vk_pipline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		vk_pipline_input_assembly_state_create_info.topology = VKConvertPrimitiveToplogy(description.PrimitiveTopology);
		vk_pipline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

		//Assemble Viewport
		VkPipelineViewportStateCreateInfo vk_pipeline_viewport_state_create_info = {};
		vk_pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vk_pipeline_viewport_state_create_info.viewportCount = 1;
		vk_pipeline_viewport_state_create_info.scissorCount = 1;

		//Assemble Rasterizer
		VkPipelineRasterizationStateCreateInfo vk_pipeline_rasterization_state_create_info =  {};
		vk_pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		vk_pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
		vk_pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
		vk_pipeline_rasterization_state_create_info.polygonMode = VKConvertFillMode(description.RasterizerDescription.FillMode);
		vk_pipeline_rasterization_state_create_info.lineWidth = 1.0f;
		vk_pipeline_rasterization_state_create_info.cullMode = VKConvertCullMode(description.RasterizerDescription.CullMode);
		vk_pipeline_rasterization_state_create_info.frontFace = description.RasterizerDescription.FrontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
		vk_pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;

		//Assemble Multi-Sampling
		VkPipelineMultisampleStateCreateInfo vk_pipeline_multisample_state_create_info = {};
		vk_pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		vk_pipeline_multisample_state_create_info.sampleShadingEnable = VK_FALSE;
		vk_pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		//Assemble Color-Blending
		VkPipelineColorBlendStateCreateInfo vk_pipeline_color_blend_state_create_info = {};
		vk_pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		VkPipelineColorBlendAttachmentState color_blend_attachments[8] = {};
		for (uint32_t i = 0; i < 8; ++i) {
			VkPipelineColorBlendAttachmentState color_blend_attachment = {};
			color_blend_attachment.blendEnable = description.BlendDescription.RenderTargetBlendDescription[i].BlendEnable ? VK_TRUE : VK_FALSE;
			color_blend_attachment.srcColorBlendFactor = VKConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].SourceBlend);
			color_blend_attachment.dstColorBlendFactor = VKConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlend);
			color_blend_attachment.colorBlendOp = VKConvertBlendOperation(description.BlendDescription.RenderTargetBlendDescription[i].BlendOperation);
			color_blend_attachment.srcAlphaBlendFactor = VKConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].SourceBlendAlpha);
			color_blend_attachment.dstAlphaBlendFactor = VKConvertBlend(description.BlendDescription.RenderTargetBlendDescription[i].DestinationBlendAlpha);
			color_blend_attachment.alphaBlendOp = VKConvertBlendOperation(description.BlendDescription.RenderTargetBlendDescription[i].BlendOperationAlpha);
			color_blend_attachment.colorWriteMask = VKConvertColorWriteMask(description.BlendDescription.RenderTargetBlendDescription[i].RenderTargetWriteMask);
			color_blend_attachments[i] = color_blend_attachment;
		}
		
		vk_pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;
		vk_pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
		vk_pipeline_color_blend_state_create_info.attachmentCount = description.NumberOfRenderTargets;
		vk_pipeline_color_blend_state_create_info.pAttachments = color_blend_attachments;
		vk_pipeline_color_blend_state_create_info.blendConstants[0] = 0.0f;
		vk_pipeline_color_blend_state_create_info.blendConstants[1] = 0.0f;
		vk_pipeline_color_blend_state_create_info.blendConstants[2] = 0.0f;
		vk_pipeline_color_blend_state_create_info.blendConstants[3] = 0.0f;

		//Set Dynamic States
		std::vector<VkDynamicState> vk_dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			//VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
		};
		VkPipelineDynamicStateCreateInfo vk_pipeline_dynamic_state_create_info = {};
		vk_pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		vk_pipeline_dynamic_state_create_info.dynamicStateCount = vk_dynamic_states.size();
		vk_pipeline_dynamic_state_create_info.pDynamicStates = vk_dynamic_states.data();

		VkPipelineDepthStencilStateCreateInfo vk_pipeline_depth_stencil_state_create_info = {};
		vk_pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		vk_pipeline_depth_stencil_state_create_info.depthTestEnable = description.DepthStencilDescription.DepthEnabled ? VK_TRUE : VK_FALSE;
		vk_pipeline_depth_stencil_state_create_info.depthWriteEnable = description.DepthStencilDescription.DepthWriteMask == RHI::DepthWriteMask::ALL ? VK_TRUE : VK_FALSE;
		vk_pipeline_depth_stencil_state_create_info.depthCompareOp = VKConvertComparisonFunction(description.DepthStencilDescription.ComparisonFunction);
		vk_pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		vk_pipeline_depth_stencil_state_create_info.stencilTestEnable = description.DepthStencilDescription.StencilEnabled ? VK_TRUE : VK_FALSE;
		vk_pipeline_depth_stencil_state_create_info.front = {}; // Optional
		vk_pipeline_depth_stencil_state_create_info.back = {}; // Optional
		vk_pipeline_depth_stencil_state_create_info.minDepthBounds = 0.0f; // Optional
		vk_pipeline_depth_stencil_state_create_info.maxDepthBounds = 1.0f; // Optional

		//Create Base PipelineInfo
		VkPipelineRenderingCreateInfo vk_pipeline_rendering_create_info = {};
		vk_pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		vk_pipeline_rendering_create_info.viewMask = 0;
		VkFormat vk_formats[8] = {};
		for (uint32_t i = 0; i < 8; ++i) {
			vk_formats[i] = VKConvertFormat(description.RenderTargetFormats[i]);
		}
		vk_pipeline_rendering_create_info.pColorAttachmentFormats = vk_formats;
		vk_pipeline_rendering_create_info.colorAttachmentCount = description.NumberOfRenderTargets;
		vk_pipeline_rendering_create_info.depthAttachmentFormat = VKConvertFormat(description.DepthFormat);
		//pipeline_rendering_info.stencilAttachmentFormat = pipeline_rendering_info.depthAttachmentFormat;

		//Create Pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &vk_pipeline_rendering_create_info;
		pipelineInfo.stageCount = static_cast<uint32_t>(vk_pipeline_shader_stage_create_infos.size());
		pipelineInfo.pStages = vk_pipeline_shader_stage_create_infos.data();
		pipelineInfo.pVertexInputState = &vk_pipeline_vertex_input_state_create_info;
		pipelineInfo.pInputAssemblyState = &vk_pipline_input_assembly_state_create_info;
		pipelineInfo.pViewportState = &vk_pipeline_viewport_state_create_info;
		pipelineInfo.pRasterizationState = &vk_pipeline_rasterization_state_create_info;
		pipelineInfo.pMultisampleState = &vk_pipeline_multisample_state_create_info;
		pipelineInfo.pColorBlendState = &vk_pipeline_color_blend_state_create_info;
		pipelineInfo.pDynamicState = &vk_pipeline_dynamic_state_create_info;
		pipelineInfo.pDepthStencilState = &vk_pipeline_depth_stencil_state_create_info;
		pipelineInfo.layout = static_cast<VKRootSignature*>(description.RootSignature)->GetNative();
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkPipeline vk_pipeline;
		if (vkCreateGraphicsPipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &vk_pipeline) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_PIPELINE_STATE;
		}
		pipeline_state = new VKPipelineState(description, m_VkDevice, vk_pipeline);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateComputePipelineState(const RHI::ComputePipelineStateDescription& description, RHI::PipelineState& pipeline_state) const {
		VkComputePipelineCreateInfo vk_compute_pipeline_create_info = {};
		vk_compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		vk_compute_pipeline_create_info.pNext = VK_NULL_HANDLE;
		vk_compute_pipeline_create_info.stage = static_cast<VKShader*>(description.ComputeShader)->GetNative();
		vk_compute_pipeline_create_info.layout = static_cast<VKRootSignature*>(description.RootSignature)->GetNative();
		VkPipeline vk_pipeline;
		vkCreateComputePipelines(m_VkDevice, VK_NULL_HANDLE, 1, &vk_compute_pipeline_create_info, VK_NULL_HANDLE, &vk_pipeline);
		pipeline_state = new VKPipelineState(description, m_VkDevice, vk_pipeline);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateConstantBufferView(const RHI::ConstantBufferViewDescription& description, RHI::ConstantBufferView& constant_buffer_view) {
		VkDescriptorBufferInfo vk_descriptor_buffer_info = {};
		vk_descriptor_buffer_info.buffer = static_cast<VKBuffer*>(description.Buffer)->GetNative();
		vk_descriptor_buffer_info.offset = 0;
		vk_descriptor_buffer_info.range = description.Size;
		constant_buffer_view = new VKConstantBufferView(description, vk_descriptor_buffer_info);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateDepthStencilView(const RHI::Texture texture, const RHI::DepthStencilViewDescription& description, RHI::DepthStencilView& depth_stencil_view) {
		VkImageViewCreateInfo vk_image_view_create_info = VKConvertDepthStencilViewDescription(description, texture);
		VkImageView vk_image_view;
		vkCreateImageView(m_VkDevice, &vk_image_view_create_info, VK_NULL_HANDLE, &vk_image_view);
		depth_stencil_view = new VKDepthStencilView(description, m_VkDevice, vk_image_view, static_cast<VKTexture*>(texture));
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateRenderTargetView(const RHI::Buffer buffer, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) {
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateRenderTargetView(const RHI::Texture texture, const RHI::RenderTargetViewDescription& description, RHI::RenderTargetView& render_target_view) {
		VkImageViewCreateInfo vk_image_view_create_info = VKConvertRenderTargetViewDescription(description, texture);
		VkImageView vk_image_view;
		if (vk_image_view_create_info.image == VK_NULL_HANDLE) {
			OutputDebugString(L"");
		}
		vkCreateImageView(m_VkDevice, &vk_image_view_create_info, VK_NULL_HANDLE, &vk_image_view);		
		render_target_view = new VKRenderTargetView(description, static_cast<VKTexture*>(texture), m_VkDevice, vk_image_view);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateSampler(const RHI::SamplerDescription& description, RHI::Sampler& sampler) {
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateShaderResourceView(const RHI::Buffer buffer, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) {
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateShaderResourceView(const RHI::Texture texture, const RHI::ShaderResourceViewDescription& description, RHI::ShaderResourceView& shader_resource_view) {
		VkImageViewCreateInfo vk_image_view_create_info = VKConvertShaderResourceViewDescription(description, texture);
		VkImageView vk_image_view;
		vkCreateImageView(m_VkDevice, &vk_image_view_create_info, VK_NULL_HANDLE, &vk_image_view);
		shader_resource_view = new VKShaderResourceView(description, m_VkDevice, vk_image_view);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateUnorderedAccessView(const RHI::Buffer buffer, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) {
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateUnorderedAccessView(const RHI::Texture texture, const RHI::UnorderedAccessViewDescription& description, RHI::UnorderedAccessView& unordered_access_view) {
		VkImageViewCreateInfo vk_image_view_create_info = VKConvertUnorderedAccessViewDescription(description, texture);
		VkImageView vk_image_view;
		vkCreateImageView(m_VkDevice, &vk_image_view_create_info, VK_NULL_HANDLE, &vk_image_view);
		unordered_access_view = new VKUnorderedAccessView(description, m_VkDevice, vk_image_view);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateCommandAllocator(RHI::CommandType command_type, RHI::CommandAllocator& command_allocator) const {
		VkCommandPoolCreateInfo vk_command_pool_create_info = {};
		vk_command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vk_command_pool_create_info.flags = 0;
		vk_command_pool_create_info.queueFamilyIndex = m_command_queue_family_indices[(uint32_t)command_type];
		vk_command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VkCommandPool vk_command_pool;
		if (vkCreateCommandPool(m_VkDevice, &vk_command_pool_create_info, VK_NULL_HANDLE, &vk_command_pool) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_COMMAND_ALLOCATOR;
		}
		command_allocator = new VKCommandAllocator(command_type, m_VkDevice, vk_command_pool);
		return RHI::Result::SUCCESS;
	}

	RHI::Result VKDevice::CreateCommandList(RHI::CommandType command_type, RHI::CommandAllocator command_allocator, RHI::CommandList& command_list) const {
		VkCommandBufferAllocateInfo vk_command_buffer_allocate_info = {};
		vk_command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vk_command_buffer_allocate_info.commandPool = static_cast<VKCommandAllocator*>(command_allocator)->GetNative();
		vk_command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vk_command_buffer_allocate_info.commandBufferCount = 1;
		VkCommandBuffer vk_command_buffer;
		if (vkAllocateCommandBuffers(m_VkDevice, &vk_command_buffer_allocate_info, &vk_command_buffer) != VK_SUCCESS) {
			return RHI::Result::E_CREATE_COMMAND_LIST;
		}
		command_list = new VKCommandList(command_type, command_allocator, vk_command_buffer);
		return RHI::Result::SUCCESS;
	}

	void VKDevice::ExecuteCommandList(RHI::CommandType command_type, uint32_t command_list_count, const RHI::CommandList* p_command_lists) {
		VkSubmitInfo vk_submit_info = {};
		vk_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		std::vector<VkCommandBuffer> vk_command_buffers = std::vector<VkCommandBuffer>(command_list_count);
		for (uint32_t i = 0; i < vk_command_buffers.size(); ++i) {
			vk_command_buffers[i] = static_cast<VKCommandList*>(p_command_lists[i])->GetNative();
		}
		vk_submit_info.commandBufferCount = command_list_count;
		vk_submit_info.pCommandBuffers = vk_command_buffers.data();
		m_VKCommandQueues[(uint32_t)command_type]->Submit(vk_submit_info);
	}

	void VKDevice::SignalQueue(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) {
		m_VKCommandQueues[(uint32_t)command_type]->Signal(*static_cast<VKFence*>(fence), fence_value);
	}

	void VKDevice::HostWait(RHI::Fence fence, uint64_t fence_value) {
		VkSemaphoreWaitInfo vk_semaphore_wait_info = {};
		vk_semaphore_wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		vk_semaphore_wait_info.semaphoreCount = 1;
		VkSemaphore vk_semaphore = static_cast<VKFence*>(fence)->GetNative();
		vk_semaphore_wait_info.pSemaphores = &vk_semaphore;
		vk_semaphore_wait_info.pValues = &fence_value;
		VkResult result;
		do {
			result = vkWaitSemaphores(m_VkDevice, &vk_semaphore_wait_info, UINT64_MAX);
		} while(result == VK_TIMEOUT);
	}

	void VKDevice::QueueWait(RHI::CommandType command_type, RHI::Fence fence, uint64_t fence_value) {
		m_VKCommandQueues[(uint32_t)command_type]->AddWaitSemaphore(static_cast<VKFence*>(fence)->GetNative(), fence_value);
	}

	RHI::AllocationInfo VKDevice::GetResourceAllocationInfo(const RHI::BufferDescription& buffer_description) const {
		VkMemoryRequirements2 vk_memory_requirements = {};
		vk_memory_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		VkDeviceBufferMemoryRequirements vk_device_buffer_memory_requirements = {};
		vk_device_buffer_memory_requirements.sType = VK_STRUCTURE_TYPE_DEVICE_BUFFER_MEMORY_REQUIREMENTS;
		VkBufferCreateInfo vk_buffer_create_info = VKConvertBufferDescription(buffer_description);
		vk_device_buffer_memory_requirements.pCreateInfo = &vk_buffer_create_info;
		vkGetDeviceBufferMemoryRequirements(m_VkDevice, &vk_device_buffer_memory_requirements, &vk_memory_requirements);
		RHI::AllocationInfo allocation_info = {};
		allocation_info.Alignment = vk_memory_requirements.memoryRequirements.alignment;
		allocation_info.Size = vk_memory_requirements.memoryRequirements.size;
		return allocation_info;
	}

	RHI::AllocationInfo VKDevice::GetResourceAllocationInfo(const RHI::TextureDescription& texture_description) const {
		VkMemoryRequirements2 vk_memory_requirements = {};
		vk_memory_requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
		VkDeviceImageMemoryRequirements vk_device_image_memory_requirements = {};
		vk_device_image_memory_requirements.sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS;
		vk_device_image_memory_requirements.planeAspect = VK_IMAGE_ASPECT_PLANE_0_BIT;
		VkImageCreateInfo vk_image_create_info = VKConvertTextureDescription(texture_description);
		vk_device_image_memory_requirements.pCreateInfo = &vk_image_create_info;
		vkGetDeviceImageMemoryRequirements(m_VkDevice, &vk_device_image_memory_requirements, &vk_memory_requirements);
		RHI::AllocationInfo allocation_info = {};
		allocation_info.Alignment = vk_memory_requirements.memoryRequirements.alignment;
		allocation_info.Size = vk_memory_requirements.memoryRequirements.size;
		return allocation_info;
	}

	uint32_t VKDevice::GetTexturePitchAlignment() const {
		return 1u;
	}

	void VKDevice::AllocateDescriptorTable(const RHI::RootSignature root_signature, uint32_t parameter_index, RHI::DescriptorTable& descriptor_table) {
		VkDescriptorSetLayout layout = VKConvertRootDescriptorTableToSetLayout(root_signature->GetDescription().Parameters[parameter_index].RootDescriptorTable, root_signature->GetDescription().StaticSamplers);
		descriptor_table = m_DescriptorHeaps.at(layout)->AllocateTable();
	}

	void VKDevice::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ConstantBufferView* p_cbv) {
		std::vector<VkDescriptorBufferInfo> vk_descriptor_buffer_infos = std::vector<VkDescriptorBufferInfo>(count);
		for (uint32_t i = 0u; i < count; ++i) {
			vk_descriptor_buffer_infos[i] = static_cast<VKConstantBufferView*>(p_cbv[i])->GetNative();
		}
		VkWriteDescriptorSet vk_write_descriptor_set = {};
		vk_write_descriptor_set.dstSet = static_cast<VKDescriptorTable*>(descriptor_table)->GetNative();
		vk_write_descriptor_set.dstBinding = offset;
		vk_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vk_write_descriptor_set.descriptorCount = count;
		vk_write_descriptor_set.pBufferInfo = vk_descriptor_buffer_infos.data();
		vk_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vkUpdateDescriptorSets(m_VkDevice, 1, &vk_write_descriptor_set, 0, VK_NULL_HANDLE);
	}

	void VKDevice::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::Sampler* p_sampler) {
	}

	void VKDevice::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::ShaderResourceView* p_srv) {		
		std::vector<VkDescriptorImageInfo> vk_descriptor_image_infos = std::vector<VkDescriptorImageInfo>(count);
		for (uint32_t i = 0u; i < count; ++i) {
			vk_descriptor_image_infos[i] = static_cast<VKShaderResourceView*>(p_srv[i])->GetNativeImageInfo();
		}
		VkWriteDescriptorSet write = {};
		write.dstSet = static_cast<VKDescriptorTable*>(descriptor_table)->GetNative();
		write.dstBinding = offset;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = count;//1;
		write.pImageInfo = vk_descriptor_image_infos.data();
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		vkUpdateDescriptorSets(m_VkDevice, 1, &write, 0, nullptr);
	}

	void VKDevice::WriteDescriptorTable(RHI::DescriptorTable descriptor_table, uint32_t offset, uint32_t count, const RHI::UnorderedAccessView* p_uav) {
		VkWriteDescriptorSet write = {};
		std::vector<VkDescriptorImageInfo> vk_descriptor_image_infos = std::vector<VkDescriptorImageInfo>(count);
		for (uint32_t i = 0u; i < count; ++i) {
			vk_descriptor_image_infos[i] = static_cast<VKUnorderedAccessView*>(p_uav[i])->GetNativeImageInfo();
		}
		write.dstSet = static_cast<VKDescriptorTable*>(descriptor_table)->GetNative();
		write.dstBinding = offset;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = count;// 1;
		write.pImageInfo = vk_descriptor_image_infos.data();
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		vkUpdateDescriptorSets(m_VkDevice, 1, &write, 0, nullptr);
	}
}