#pragma once

#include <array>

#include "ChunkTypes.h"
#include "ObjectTypes.h"

using RENDER_OBJECT_ID = OBJECT_ID;
constexpr RENDER_OBJECT_ID INVALID_RENDER_OBJECT_ID = INVALID_OBJECT_ID;

struct ChunkColumnRenderBinding
{
	std::array<RENDER_OBJECT_ID, CHUNK_SECTION_COUNT> renderObj{};

	ChunkColumnRenderBinding()
	{
		renderObj.fill(INVALID_RENDER_OBJECT_ID);
	}
};