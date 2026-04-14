#include "pch.h"
#include "CChunkComponent.h"
#include "CChunkWorld.h"
#include "ChunkMath.h"

void CChunkComponent::Init()
{
	m_arrayBlocks.fill(0);
	m_bDirty = true;
}

BLOCK_ID CChunkComponent::GetBlock(int x, int y, int z) const
{
	if (!ChunkMath::InChunk(x, y, z)) return 0;
	return m_arrayBlocks[ChunkMath::IndexSection(x, y, z)];
}

void CChunkComponent::SetBlock(int x, int y, int z, BLOCK_ID id)
{
	if (!ChunkMath::InChunk(x, y, z)) return;
	m_arrayBlocks[ChunkMath::IndexSection(x, y, z)] = id;
	m_bDirty = true;
}