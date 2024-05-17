#ifndef _VK_FRAME_BUFFER_H_
#define _VK_FRAME_BUFFER_H_
#include "../IFrameBuffer.h"
#include <vulkan/vulkan.h>

namespace RHI::VK {
	class VKFrameBuffer : public RHI::IFrameBuffer {
	public:
		VKFrameBuffer(const FrameBufferDescription& description) : RHI::IFrameBuffer(description) {

		}
		~VKFrameBuffer() override {

		}
	private:
	};
}
#endif