#pragma once
#include "ChunkTypes.h"

namespace ChunkMath
{
	inline int FloorDiv(int x, int divisor)
	{
		int q = x / divisor;
		int r = x % divisor;
		if (r < 0) --q;
		return q;
	}

	inline int PositiveMod(int x, int divisor)
	{
		int m = x % divisor;
		if (m < 0) m += divisor;
		return m;
	}

	inline int FloorDiv16(int x)
	{
		return FloorDiv(x, 16);
	}

	inline int Mod16(int x)
	{
		return PositiveMod(x, 16);
	}

	inline int IndexSection(int x, int y, int z)
	{
		return x | (z << 4) |( y << 8);
	}

	inline bool InChunk(int x, int y, int z)
	{
		return (0 <= x && x < CHUNK_SIZE_X)
			&& (0 <= y && y < CHUNK_SIZE_Y)
			&& (0 <= z && z < CHUNK_SIZE_Z);
	}

	inline bool InSection(int x, int y, int z)
	{
		return (0 <= x && x < CHUNK_SIZE_X)
			&& (0 <= y && y < CHUNK_SECTION_SIZE)
			&& (0 <= z && z < CHUNK_SIZE_Z);
	}

	inline uint64_t MakeColumnKey(int x, int z)
	{
		return (uint64_t)(uint32_t)x | ((uint64_t)(uint32_t)z << 32);
	}

	inline uint16_t MakeColumnLocalIndex(int lx, int wy, int lz)
	{
		return static_cast<uint16_t>(wy * CHUNK_SIZE_Z * CHUNK_SIZE_X + lz * CHUNK_SIZE_X + lx);
	}

	inline int MakeIndex(int lx, int ly, int lz)
	{
		return lx + (lz * CHUNK_SECTION_SIZE) + (ly * CHUNK_SECTION_SIZE * CHUNK_SECTION_SIZE);
	}

	static bool GetWallTorchSupportDirFromFacing(uint64_t facingHash, XMINT3& outDir)
	{
		if (facingHash == fnv1a_64("north"))		{ outDir = { 0, 0, 1 }; return true; }
		else if (facingHash == fnv1a_64("south"))	{ outDir = { 0, 0, -1 }; return true; }
		else if (facingHash == fnv1a_64("east"))	{ outDir = { -1, 0, 0 }; return true; }
		else if (facingHash == fnv1a_64("west"))	{ outDir = { 1, 0, 0 }; return true; }
		return false;
	}

}