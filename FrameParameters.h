#ifndef _AT_FRAME_PARAMETERS_H_
#define _AT_FRAME_PARAMETERS_H_
#include <stdint.h>
#include <chrono>
#include <array>
#include "RenderData.h"
//Struct storing frame globals.
namespace AT {
	struct FrameParameters {
		uint64_t FrameNumber;
		float DeltaTime;
		std::chrono::steady_clock::time_point LastTime;		
		RenderData RenderData;
		std::array<AT::RenderData, 6> ReflectionProbeRenderData;
	};
}
#endif