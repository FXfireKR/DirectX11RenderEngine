#include "pch.h"
#include "CChunkSection.h"
#include "ChunkMath.h"
#include "CChunkWorld.h"

using namespace ChunkMath;

CChunkSection::CChunkSection()
{
    m_cells.fill(BlockCell{});
    m_nonAirCount = 0;
#ifdef OPTIMIZATION_2
    m_bMeshDirty = true;
    m_bLightDirty = true;
#else // OPTIMIZATION_2
    m_bDirty = true;
#endif // OPTIMIZATION_2
    m_bBuildQueued = false;
    ClearAllRenderObjectIDs();
}

BlockCell CChunkSection::GetBlock(int lx, int ly, int lz) const
{ 
    // TODO : 복사해서 넘길 이유가 없다면 나중에 const-ref로 바꾸자.
    return m_cells[MakeIndex(lx, ly, lz)];
}

void CChunkSection::SetBlock(int lx, int ly, int lz, const BlockCell& cell)
{
    BlockCell& oldCell = m_cells[MakeIndex(lx, ly, lz)];
    if (oldCell == cell)
        return;

    const bool wasAir = oldCell.IsAir();
    const bool isAir = cell.IsAir();

    oldCell = cell;

    if (wasAir != isAir)
        m_nonAirCount += isAir ? -1 : 1;

#ifdef OPTIMIZATION_2
    m_bMeshDirty = true;
    m_bLightDirty = true;
#else // OPTIMIZATION_2
    m_bDirty = true;
#endif // OPTIMIZATION_2
}

uint8_t CChunkLightSection::GetBlockLight(int lx, int ly, int lz) const
{
    return m_blockLight[MakeIndex(lx, ly, lz)];
}

void CChunkLightSection::SetBlockLight(int lx, int ly, int lz, uint8_t level)
{
    if (level > 15)
        level = 15;

    uint8_t& oldLevel = m_blockLight[MakeIndex(lx, ly, lz)];
    if (oldLevel == level)
        return;

    const bool wasZero = (oldLevel == 0);
    const bool isZero = (level == 0);

    oldLevel = level;

    if (wasZero != isZero)
        m_nonZeroCount += isZero ? -1 : 1;

}

void CChunkLightSection::Clear()
{
    m_blockLight.fill(0);
    m_nonZeroCount = 0;
}