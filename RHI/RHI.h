#ifndef _RHI_H_
#define _RHI_H_
#include "RHICore.h"
#ifdef _WIN32
	#include "./DirectX12/DX12RenderBackend.h"
	#include "./Vulkan/VKRenderBackend.h"
#elif __APPLE__
#else
	#include "./Vulkan/VKRenderBackend.h"
#endif
namespace RHI {
	enum class RenderAPI {
		VULKAN,
#ifdef _WIN32
		DIRECTX,
#endif
	};
	inline void CreateRenderBackend(RHI::RenderAPI api, bool debug, RHI::RenderBackend& render_backend) {
		switch (api) {
		case RHI::RenderAPI::DIRECTX:
			render_backend = new RHI::DX12::DX12RenderBackend(debug);
			break;
		case RHI::RenderAPI::VULKAN:
			render_backend = new RHI::VK::VKRenderBackend(debug);
			break;
		}
	}
};
#endif