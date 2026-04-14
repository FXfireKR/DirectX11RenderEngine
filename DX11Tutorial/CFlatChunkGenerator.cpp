#include "pch.h"
#include "CFlatChunkGenerator.h"
#include "CHeightmapChunkGenerator.h"

void CFlatChunkGenerator::Initialize(const WorldGenerateSettings& settings)
{
	m_settings = settings;

	m_air = { 0, 0 };
	m_bedrock = _MakeBlockCell("minecraft:bedrock");
	m_stone = _MakeBlockCell("minecraft:stone");
	m_dirt = _MakeBlockCell("minecraft:dirt");
}

BlockCell CFlatChunkGenerator::SampleBlock(int wx, int wy, int wz) const
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return m_air;

	// 0 은 bedrock
	if (wy == 0)
		return m_bedrock;

	else if (wy < 12)
		return m_stone;

	else if (wy < 14)
		return m_dirt;

	return m_air;
}

void CFlatChunkGenerator::GenerateColumn(CChunkColumn& column) const
{
	const ChunkCoord coord = column.GetCoord();

	CChunkSection* pSection = column.EnsureSection(0);
	if (!pSection)
		return;

	for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz)
	{
		for (int lx = 0; lx < CHUNK_SIZE_X; ++lx)
		{
			const int wx = coord.x * CHUNK_SIZE_X + lx;
			const int wz = coord.z * CHUNK_SIZE_Z + lz;

			pSection->SetBlock(lx, 0, lz, SampleBlock(wx, 0, wz));
		}
	}
}

BlockCell CFlatChunkGenerator::_MakeBlockCell(const char* blockName) const
{
	const BLOCK_ID blockID = BlockDB.FindBlockID(blockName);

	BlockPropHashMap props;
	STATE_INDEX sidx{};
	const bool ok = BlockDB.EncodeStateIndex(blockID, props, sidx);
	assert(ok);

	return { blockID, sidx };
}