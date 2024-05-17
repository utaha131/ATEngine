#ifndef _RHI_I_RENDER_BACKEND_H_
#define _RHI_I_RENDER_BACKEND_H_
#include <vector>
#include "RHICore.h"
#include "ISwapChain.h"

namespace RHI {
	class IRenderBackend {
	public:
		virtual ~IRenderBackend() = default;
		virtual RHI::Result GetAdapters(std::vector<RHI::Adapter>& adapters) = 0;
		virtual RHI::Result CreateDevice(const RHI::Adapter adapter, RHI::Device& device) = 0;
		virtual RHI::Result CreateSwapChain(RHI::Device device, const SwapChainDescription& description, SwapChain& swap_chain) = 0;
	protected:
		bool m_Debug;
		IRenderBackend(bool debug) :
			m_Debug(debug) {}
	};
}
#endif