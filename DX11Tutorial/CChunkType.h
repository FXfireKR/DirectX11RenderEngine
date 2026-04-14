#pragma once
#include "VoxelTypes.h"

static constexpr int CHUNK_SIZE_X = 16;
static constexpr int CHUNK_SIZE_Y = 256;
static constexpr int CHUNK_SIZE_Z = 16;
static constexpr int CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

static constexpr int CHUNK_SECTION_SIZE = 16;
static constexpr int CHUNK_SECTION_COUNT = CHUNK_SIZE_Y / CHUNK_SECTION_SIZE;
static constexpr int CHUNK_SECTION_VOLUME = CHUNK_SIZE_X * CHUNK_SECTION_SIZE * CHUNK_SIZE_Z;

struct ChunkCoord { int x = 0, y = 0, z = 0; };
struct ChunkColumnCoord { int cx = 0, cz = 0; };

struct BlockCell
{
	BLOCK_ID blockID;
	uint16_t stateIndex = 0;

	BlockCell& operator=(const BlockCell& other) {
		this->blockID = other.blockID;
		this->stateIndex = other.stateIndex;
		return *this;
	}

	bool operator==(const BlockCell& other) {
		return this->blockID == other.blockID && 
			this->stateIndex == other.stateIndex;
	}

	bool operator!=(const BlockCell& other) {
		return this->blockID != other.blockID ||
			this->stateIndex != other.stateIndex;
	}

	const bool IsAir() const {
		return blockID == 0;
	}
};

struct ChunkSection
{
	std::array<BlockCell, CHUNK_SECTION_VOLUME> blocks{};
	bool dirty = true;
	bool dirtyQueued = false; // 중복 큐잉 방지

	ChunkSection() { blocks.fill({ 0,0 }); }
};

struct ChunkColumn
{
	ChunkColumnCoord coord;
	std::unique_ptr<ChunkSection> sections[CHUNK_SECTION_COUNT]{};
	ObjectID renderObj[CHUNK_SECTION_COUNT]{};
};


static int FloorDiv16(int x)
{
	int q = x / 16;
	int r = x % 16;
	if (r < 0) --q;
	return q;
};

static int Mod16(int x)
{
	int m = x % 16;
	if (m < 0) m += 16;
	return m;
};

static int Index16(int x, int y, int z)
{
	return x + (z * 16) + (y * 16 * 16);
};

static int GetChunkIndex(int x, int y, int z)
{
	return x + (y * CHUNK_SIZE_X) + (z * CHUNK_SIZE_X * CHUNK_SIZE_Y);
};

static bool InChunk(int x, int y, int z)
{
	return (0 <= x && x < CHUNK_SIZE_X) && (0 <= y && y < CHUNK_SIZE_Y) && (0 <= z && z < CHUNK_SIZE_Z);
};

