#pragma once
#include <cstdint>

enum class BLOCK_TOOL_KIND : uint8_t
{
	NONE = 0,
	PICKAXE,
	SHOVEL,
	AXE,
	HOE,
};

enum class BLOCK_TOOL_TIER : uint8_t
{
	NONE = 0,
	WOOD,
	STONE,
	IRON,
	DIAMOND,
	NETHERITE,
};