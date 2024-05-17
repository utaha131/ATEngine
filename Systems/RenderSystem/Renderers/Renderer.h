#ifndef _AT_RENDER_SYSTEM_RENDERER_H_
#define _AT_RENDER_SYSTEM_RENDERER_H_
#include "../FrameGraphBuilder.h"
#include "../GPUShaderManager.h"
#include "../GPUPipelineStateManager.h"
#include "../GPURootSignatureManager.h"
#include "../FrameParameters.h"
#include "../Scene/Scene.h"

namespace AT {
	class Renderer {
	public:
		virtual void RenderLogic(FrameParameters& frame_parameters, Scene* scene) = 0;
		virtual void GPUExecution(FrameParameters& frame_parameters, FrameGraphBuilder& graph_builder, AT::GPUShaderManager& shader_manager, AT::GPUPipelineStateManager& pipeline_state_manager, AT::GPURootSignatureManager& root_signature_manager, RHI::Texture swap_chain_image) = 0;
	};
}
#endif