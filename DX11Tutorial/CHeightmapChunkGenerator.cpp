#include "pch.h"
#include "CHeightmapChunkGenerator.h"

#include "ChunkMath.h"
using namespace ChunkMath;

void CHeightmapChunkGenerator::Initialize(const WorldGenerateSettings& settings)
{
	m_settings = settings;

	m_air = { 0, 0 };
	m_bedrock = _MakeBlockCell("minecraft:bedrock");
	m_stone = _MakeBlockCell("minecraft:stone");
	m_dirt = _MakeBlockCell("minecraft:dirt");
	m_grass = _MakeBlockCell("minecraft:grass_block");
}

BlockCell CHeightmapChunkGenerator::SampleBlock(int wx, int wy, int wz) const
{
	if (wy < 0 || wy >= CHUNK_SIZE_Y)
		return m_air;

	const int h = _SampleTerrainHeight(wx, wz);

	if (wy == 0)
		return m_bedrock;

	if (wy > h)
		return m_air;

	if (wy == h)
		return m_grass;

	if (wy >= h - 2)
		return m_dirt;

	return m_stone;
}

void CHeightmapChunkGenerator::GenerateColumn(CChunkColumn& column) const
{
	const ChunkCoord coord = column.GetCoord();

	for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz)
	{
		for (int lx = 0; lx < CHUNK_SIZE_X; ++lx)
		{
			const int wx = coord.x * CHUNK_SIZE_X + lx;
			const int wz = coord.z * CHUNK_SIZE_Z + lz;

			const int h = _SampleTerrainHeight(wx, wz);
			const int clampedTop = std::min(h, CHUNK_SIZE_Y - 1);

			for (int wy = 0; wy <= clampedTop; ++wy)
			{
				const BlockCell cell = SampleBlock(wx, wy, wz);
				if (cell.IsAir())
					continue;

				const int sy = wy / CHUNK_SECTION_SIZE;
				const int ly = wy % CHUNK_SECTION_SIZE;

				CChunkSection* pSection = column.EnsureSection(sy);
				if (!pSection)
					continue;

				pSection->SetBlock(lx, ly, lz, cell);
			}
		}
	}
}

BlockCell CHeightmapChunkGenerator::_MakeBlockCell(const char* blockName) const
{
	const BLOCK_ID blockID = BlockDB.FindBlockID(blockName);

	BlockPropHashMap props;
	props.insert({ fnv1a_64("snowy"), fnv1a_64("false") });

	STATE_INDEX sidx{};
	const bool ok = BlockDB.EncodeStateIndex(blockID, props, sidx);
	assert(ok);

	return { blockID, sidx };
}

float CHeightmapChunkGenerator::_Hash01(int x, int z) const
{
	uint32_t h = m_settings.seed;
	h ^= (uint32_t)x * 374761393u;
	h ^= (uint32_t)z * 668265263u;
	h = (h ^ (h >> 13)) * 1274126177u;
	h ^= (h >> 16);

	return (h & 0x00FFFFFF) / 16777215.0f;
}

int CHeightmapChunkGenerator::_SampleTerrainHeight(int wx, int wz) const
{
	const int cellSize = 16;

	const int gx0 = FloorDiv16(wx);
	const int gz0 = FloorDiv16(wz);
	const int gx1 = gx0 + 1;
	const int gz1 = gz0 + 1;

	const float tx = (float)Mod16(wx) / (float)cellSize;
	const float tz = (float)Mod16(wz) / (float)cellSize;

	const float n00 = _Hash01(gx0, gz0);
	const float n10 = _Hash01(gx1, gz0);
	const float n01 = _Hash01(gx0, gz1);
	const float n11 = _Hash01(gx1, gz1);

	const float nx0 = lerp(n00, n10, tx);
	const float nx1 = lerp(n01, n11, tx);
	const float n = lerp(nx0, nx1, tz);

	return m_settings.baseHeight + (int)std::round((n * 2.0f - 1.0f) * (float)m_settings.heightAmplitude);
}