#pragma once
#include <array>
#include <memory>
#include <DirectXMath.h>

#include "ObjectTypes.h"
#include "VoxelTypes.h"
#include "VertexTypes.h"

static constexpr int CHUNK_SIZE_X = 16;
static constexpr int CHUNK_SIZE_Y = 256;
static constexpr int CHUNK_SIZE_Z = 16;
static constexpr int CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

static constexpr int CHUNK_SECTION_SIZE = 16;
static constexpr int CHUNK_SECTION_COUNT = CHUNK_SIZE_Y / CHUNK_SECTION_SIZE;
static constexpr int CHUNK_SECTION_VOLUME = CHUNK_SIZE_X * CHUNK_SECTION_SIZE * CHUNK_SIZE_Z;

struct ChunkCoord
{
	int x = 0;
	int z = 0;
	bool operator==(const ChunkCoord& rhs) const
	{
		return x == rhs.x && z == rhs.z;
	}
};

struct ChunkCoordHasher
{
	size_t operator()(const ChunkCoord& key) const
	{
		const uint64_t a = static_cast<uint32_t>(key.x);
		const uint64_t b = static_cast<uint32_t>(key.z);
		return static_cast<size_t>((a << 32) ^ b);
	}
};

using SectionCoord = DirectX::XMINT3;

struct SectionCoordHasher
{
	size_t operator()(const SectionCoord& key) const
	{
		const uint64_t a = static_cast<uint32_t>(key.x);
		const uint64_t b = static_cast<uint32_t>(key.y);
		const uint64_t c = static_cast<uint32_t>(key.z);

		uint64_t h = a;
		h = (h * 1315423911ull) ^ b;
		h = (h * 1315423911ull) ^ c;
		return static_cast<size_t>(h);
	}
};

struct ChunkRenderEntry
{
	OBJECT_ID renderObj = INVALID_OBJECT_ID;

	bool bUploaded = false;
	bool bIsVisible = false;
};

using ChunkMeshVertex = VERTEX_POSITION_NORMAL_UV_COLOR;

struct ChunkMeshData
{
	std::vector<ChunkMeshVertex> vertices;
	std::vector<uint32_t> indices;

	void Clear()
	{
		vertices.clear();
		indices.clear();
	}

	bool Empty() const
	{
		return vertices.empty() || indices.empty();
	}
};

enum class EChunkSectionRenderSlot : uint8_t
{
	OPAQUE_SLOT = 0,
	CUTOUT_SLOT,
	TRANSLUCENT_SLOT,

	COUNT,
};

struct ChunkSectionMeshSet
{
	ChunkMeshData opaque; // OPAQUE
	ChunkMeshData cutout; // CUTOUT
	ChunkMeshData translucent; // TRANSLUCENT

	void Clear()
	{
		opaque.Clear();
		cutout.Clear();
		translucent.Clear();
	}

	bool EmptyAll() const
	{
		return opaque.Empty() && cutout.Empty() && translucent.Empty();
	}
};

struct BlockCell
{
	BLOCK_ID blockID = 0;
	STATE_INDEX stateIndex = 0;

	BlockCell& operator=(const BlockCell& other)
	{
		this->blockID = other.blockID;
		this->stateIndex = other.stateIndex;
		return *this;
	}

	bool operator==(const BlockCell& rhs) const
	{
		return this->blockID == rhs.blockID 
			&& this->stateIndex == rhs.stateIndex;
	}

	bool operator!=(const BlockCell& rhs) const
	{
		return !(*this == rhs);
	}

	const bool IsAir() const
	{
		return blockID == 0;
	}
};
static_assert(sizeof(BlockCell) == 4, "BlockCell must be 4 bytes");