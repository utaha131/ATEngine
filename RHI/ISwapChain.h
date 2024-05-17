#ifndef _RHI_I_SWAP_CHAIN_H_
#define _RHI_I_SWAP_CHAIN_H_
#include <vector>
#include <atomic>
#include "RHICore.h"

namespace RHI {
	typedef struct SwapChainDescription {
		RHI::Format BufferFormat;
		uint32_t Width;
		uint32_t Height;
		uint32_t BufferCount;
		void* Window;
	} SwapChainDescription;

	class ISwapChain {
	public:
		virtual ~ISwapChain() = default;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Present() = 0;
		inline const SwapChainDescription& GetDescription() const { return m_Description; }
		inline RHI::Texture GetBackBuffer(uint32_t index) const { return m_BackBuffers[index]; }
		virtual inline uint32_t GetCurrentBackBufferIndex() = 0;// { return m_current_buffer_index; } //TODO maybe remove?
	protected:
		SwapChainDescription m_Description;
		//std::vector<uint64_t> m_back_buffer_fences;
		std::vector<RHI::Texture> m_BackBuffers;
		uint32_t m_CurrentBufferIndex;
		ISwapChain(const SwapChainDescription& description) :
			m_Description(description),
			m_BackBuffers(std::vector<RHI::Texture>(description.BufferCount)),
			m_CurrentBufferIndex(0u)/*,
			m_back_buffer_fences(std::vector<uint64_t>(description.Buffer_Count, 0))*/ {};
	};
}
#endif