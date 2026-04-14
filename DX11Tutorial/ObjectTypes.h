#pragma once

#include <cstdint>
#include <limits>

using OBJECT_ID = uint32_t;
constexpr OBJECT_ID INVALID_OBJECT_ID = std::numeric_limits<OBJECT_ID>::max();

inline bool IsValidObjectID(OBJECT_ID id)
{
	return id != INVALID_OBJECT_ID;
}