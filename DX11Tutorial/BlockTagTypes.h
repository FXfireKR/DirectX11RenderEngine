#pragma once

#include "VoxelTypes.h"

#include <string>
#include <vector>
#include <unordered_map>

using BLOCK_TAG_HASH = uint32_t;

struct BlockTagRaw
{
	std::string name; // minecraft:mineable/pickaxe
	std::vector<std::string> values; // minecraft:stone, #minecraft:logs
	bool bLoaded = false;
};

struct BlockTagDef
{
	std::string name;
	std::vector<BLOCK_ID> blockIDs;
	bool bResolved = false;
};