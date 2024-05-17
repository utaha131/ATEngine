#ifndef _RHI_I_RENDER_PASS_H_
#define _RHI_I_RENDER_PASS_H_
#include <vector>
#include <optional>
#include "RHICore.h"

namespace RHI {
	//typedef struct RenderPassAttachment {
	//	enum class LoadOperation {
	//		LOAD,
	//		CLEAR,
	//		DONT_CARE
	//	};

	//	enum class StoreOperation {
	//		STORE,
	//		DONT_CARE
	//	};

	//	RHI::Format Format = RHI::Format::UNKNOWN;
	//	LoadOperation LoadOp;
	//	StoreOperation StoreOp;
	//	LoadOperation StencilLoadOp;
	//	StoreOperation StencilStoreOp;
	//	RHI::TextureState InitialState;
	//	RHI::TextureState  FinalState;
	//} RenderPassAttachment;


	//typedef struct RenderPassDescription {
	//	std::vector<RenderPassAttachment> Attachments;
	//	std::optional<RenderPassAttachment> DepthAttachment;
	//} RenderPassDescription;

	///*class IRenderPass {
	//public:
	//	virtual ~IRenderPass() = default;
	//	inline const RenderPassDescription& GetDescription() const { return m_Description; };
	//protected:
	//	RenderPassDescription m_Description;
	//	IRenderPass(const RenderPassDescription& description) :
	//		m_Description(description) {}
	//};*/
	//class RenderPass {
	//public:
	//	RenderPass(const RenderPassDescription& description) :
	//		m_Description(description) {}
	//	inline const RenderPassDescription& GetDescription() const { return m_Description; };
	//protected:
	//	RenderPassDescription m_Description;
	//};
}
#endif