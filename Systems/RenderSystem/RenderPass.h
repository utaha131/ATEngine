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
			m_render_targets.push_back(rtv);
			m_write_textures.insert(rtv->TextureRef);
			m_load_op_map[rtv] = Load_Op;
		}
		
		void AddShaderResource(FrameGraphSRVRef srv) {
			//m_shader_resources.insert(srv);
			assert(srv != nullptr);
			m_read_textures.insert(srv->TextureRef);
			m_shader_resources.push_back(srv);
		}

		void AddUnorderedAccessRead(FrameGraphUAVRef uav) {
			assert(uav != nullptr);
			m_read_textures.insert(uav->TextureRef);
			m_unordered_access_view_read.push_back(uav);
		}

		void AddUnorderedAccessWrite(FrameGraphUAVRef uav) {
			assert(uav != nullptr);
			m_write_textures.insert(uav->TextureRef);
			m_unordered_access_view_write.push_back(uav);
		}

		void SetDepthStencilBuffer(FrameGraphDSVRef dsv, RHI::RenderPassAttachment::LoadOperation Load_Op) {
			m_depth_buffer.emplace(dsv);
			m_write_textures.insert(dsv->TextureRef);
			m_depth_buffer_is_dependency = false;
			m_load_op_map[dsv] = Load_Op;
		}

		void SetDepthStencilBufferAsDependency(FrameGraphDSVRef dsv, RHI::RenderPassAttachment::LoadOperation Load_Op) {
			m_depth_buffer.emplace(dsv);
			m_write_textures.insert(dsv->TextureRef);
			m_depth_buffer_is_dependency = true;
			m_load_op_map[dsv] = Load_Op;
		}

		RHI::RenderPassAttachment::LoadOperation GetLoadOp(void* ref) const {
			RHI::RenderPassAttachment::LoadOperation load_op = m_load_op_map.at(ref);
			return load_op;
		}

		std::vector<FrameGraphRTVRef> m_render_targets = std::vector<FrameGraphRTVRef>();
		std::vector<FrameGraphSRVRef> m_shader_resources = std::vector<FrameGraphSRVRef>();
		std::vector<FrameGraphUAVRef> m_unordered_access_view_read = std::vector<FrameGraphUAVRef>();
		std::vector<FrameGraphUAVRef> m_unordered_access_view_write = std::vector<FrameGraphUAVRef>();
		std::unordered_set<FrameGraphTextureRef> m_write_textures = std::unordered_set<FrameGraphTextureRef>();
		std::unordered_set<FrameGraphTextureRef> m_read_textures = std::unordered_set<FrameGraphTextureRef>();
		std::optional<FrameGraphDSVRef> m_depth_buffer;
		std::unordered_map<void*, RHI::RenderPassAttachment::LoadOperation> m_load_op_map;
		bool m_depth_buffer_is_dependency = false;
	};

	class FrameGraphRenderPass {
	public:
		enum PIPELINE_TYPE {
			GRAPHICS,
			COMPUTE
		};

		FrameGraphRenderPass(const std::string& name, const FrameGraphRenderPassIO& render_pass_io, PIPELINE_TYPE pipeline_type) :
			m_name(name),
			m_render_pass_io(render_pass_io),
			m_pipeline_type(pipeline_type)
		{
		}

		virtual ~FrameGraphRenderPass() {}

		const FrameGraphRenderPassIO& GetIO() const {
			return m_render_pass_io;
		}

		const std::string& GetName() const {
			return m_name;
		}

		PIPELINE_TYPE GetPipelineType() const {
			return m_pipeline_type;
		}

		virtual void Execute(RHI::CommandList command_list) const = 0;
	protected:
		FrameGraphRenderPassIO m_render_pass_io;
		std::string m_name;
		PIPELINE_TYPE m_pipeline_type;
	};

	template<typename F> class FrameGraphLambdaRenderPass : public FrameGraphRenderPass {
	public:
		FrameGraphLambdaRenderPass(const std::string& name, const FrameGraphRenderPassIO& render_pass_io, PIPELINE_TYPE pipeline_type, F&& execute_function) :
			FrameGraphRenderPass(name, render_pass_io, pipeline_type),
			m_ExecuteFunction(static_cast<F&&>(execute_function))
		{
		}
		
		~FrameGraphLambdaRenderPass() override {
			//m_execute_function.~F();
		}

		void Execute(RHI::CommandList command_list) const override {
			m_ExecuteFunction(command_list);
		}
	private:
		F m_ExecuteFunction;
	};
}
#endif