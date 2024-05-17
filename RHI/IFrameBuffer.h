#ifndef _RHI_I_FRAME_BUFFER_H_
#define _RHI_I_FRAME_BUFFER_H_
#include <vector>
#include <optional>
#include "RHICore.h"

namespace RHI {
	typedef struct FrameBufferDescription {
		RHI::RenderPass RenderPass;
		uint32_t Width;
		uint32_t Height;
		std::vector<RHI::RenderTargetView> RenderTargetViews;
		std::optional<RHI::DepthStencilView> DepthStencilView;
	} FrameBufferDescription;

	class IFrameBuffer {
	public:
		virtual ~IFrameBuffer() = default;
		inline const FrameBufferDescription& GetDescription() const { return m_Description; };
	protected:
		FrameBufferDescription m_Description;
		IFrameBuffer(const FrameBufferDescription& description) :
			m_Description(description) {}
	};
}
#endif