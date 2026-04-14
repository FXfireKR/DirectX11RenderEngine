#pragma once
#include <cstddef>
#include <cstdint>

enum class ERenderPass : uint8_t
{
	SHADOW_PASS = 0,
	SKY_PASS,
	CLOUD_PASS,
	OPAQUE_PASS,
	CUTOUT_PASS,
	TRANSPARENT_PASS,
	DEBUG_PASS,
	ORTH_PASS,
	// Other pass add here...

	COUNT
};
constexpr size_t RENDER_PASS_COUNT = static_cast<size_t>(ERenderPass::COUNT);