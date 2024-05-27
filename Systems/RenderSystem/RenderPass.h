#ifndef _AT_ENGINE_RENDER_SYSTEM_RENDER_PASS_H_
#define _AT_ENGINE_RENDER_SYSTEM_RENDER_PASS_H_
#include "../RHI/RHI.h"
#include <functional>
#include <vector>
#include <assert.h>
#include <optional>
#include <unordered_set>
#include <string>
#include "FrameGraphResources.h"

namespace AT {

	struct FrameGraphRenderPassIO {
	public:
		void AddRenderTarget(FrameGraphRTVRef rtv, RHI::RenderPassAttachment::LoadOperation Load_Op) {
			assert(rtv != nullptr);
			m_RenderTargets.push_back(rtv);
			m_WriteTextures.insert(rtv->TextureRef);
			m_LoadOpMap[rtv] = Load_Op;
		}
		
		void AddShaderResource(FrameGraphSRVRef srv) {
			assert(srv != nullptr);
			m_ReadTextures.insert(srv->TextureRef);
			m_ShaderResources.push_back(srv);
		}

		void AddUnorderedAccessRead(FrameGraphUAVRef uav) {
			assert(uav != nullptr);
			m_ReadTextures.insert(uav->TextureRef);
			m_UnorderedAccessViewRead.push_back(uav);
		}

		void AddUnorderedAccessWrite(FrameGraphUAVRef uav) {
			assert(uav != nullptr);
			m_WriteTextures.insert(uav->TextureRef);
			m_UnorderedAccessViewWrite.push_back(uav);
		}

		void SetDepthStencilBuffer(FrameGraphDSVRef dsv, RHI::RenderPassAttachment::LoadOperation Load_Op) {
			m_DepthBuffer.emplace(dsv);
			m_WriteTextures.insert(dsv->TextureRef);
			m_DepthBufferIsDependency = false;
			m_LoadOpMap[dsv] = Load_Op;
		}

		void SetDepthStencilBufferAsDependency(FrameGraphDSVRef dsv, RHI::RenderPassAttachment::LoadOperation Load_Op) {
			m_DepthBuffer.emplace(dsv);
			m_WriteTextures.insert(dsv->TextureRef);
			m_DepthBufferIsDependency = true;
			m_LoadOpMap[dsv] = Load_Op;
		}

		bool DepthBufferIsDependency() const {
			return m_DepthBufferIsDependency;
		}

		RHI::RenderPassAttachment::LoadOperation GetLoadOp(void* ref) const {
			RHI::RenderPassAttachment::LoadOperation load_op = m_LoadOpMap.at(ref);
			return load_op;
		}

		std::vector<FrameGraphRTVRef> m_RenderTargets = std::vector<FrameGraphRTVRef>();
		std::vector<FrameGraphSRVRef> m_ShaderResources = std::vector<FrameGraphSRVRef>();
		std::vector<FrameGraphUAVRef> m_UnorderedAccessViewRead = std::vector<FrameGraphUAVRef>();
		std::vector<FrameGraphUAVRef> m_UnorderedAccessViewWrite = std::vector<FrameGraphUAVRef>();
		std::unordered_set<FrameGraphTextureRef> m_WriteTextures = std::unordered_set<FrameGraphTextureRef>();
		std::unordered_set<FrameGraphTextureRef> m_ReadTextures = std::unordered_set<FrameGraphTextureRef>();
		std::optional<FrameGraphDSVRef> m_DepthBuffer;
	private:
		std::unordered_map<void*, RHI::RenderPassAttachment::LoadOperation> m_LoadOpMap;
		bool m_DepthBufferIsDependency = false;
	};

	class FrameGraphRenderPass {
	public:
		enum PIPELINE_TYPE {
			GRAPHICS,
			COMPUTE
		};

		FrameGraphRenderPass(const std::string& name, const FrameGraphRenderPassIO& render_pass_io, PIPELINE_TYPE pipeline_type) :
			m_Name(name),
			m_RenderPassIO(render_pass_io),
			m_PipelineType(pipeline_type)
		{
		}

		virtual ~FrameGraphRenderPass() {}

		inline const FrameGraphRenderPassIO& GetIO() const {
			return m_RenderPassIO;
		}

		inline const std::string& GetName() const {
			return m_Name;
		}

		inline PIPELINE_TYPE GetPipelineType() const {
			return m_PipelineType;
		}

		virtual void Execute(RHI::CommandList command_list) const = 0;
	protected:
		FrameGraphRenderPassIO m_RenderPassIO;
		std::string m_Name;
		PIPELINE_TYPE m_PipelineType;
	};

	template<typename F> class FrameGraphLambdaRenderPass : public FrameGraphRenderPass {
	public:
		FrameGraphLambdaRenderPass(const std::string& name, const FrameGraphRenderPassIO& render_pass_io, PIPELINE_TYPE pipeline_type, F&& execute_function) :
			FrameGraphRenderPass(name, render_pass_io, pipeline_type),
			m_ExecuteFunction(static_cast<F&&>(execute_function))
		{
		}
		
		~FrameGraphLambdaRenderPass() override {}

		void Execute(RHI::CommandList command_list) const override {
				m_ExecuteFunction(command_list);
		}
	private:
		F m_ExecuteFunction;
	};
}
#endif