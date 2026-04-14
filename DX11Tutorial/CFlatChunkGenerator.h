#pragma once
#include "IChunkGenerator.h"

class CFlatChunkGenerator final : public IChunkGenerator
{
public:
	void Initialize(const WorldGenerateSettings& settings) override;

	BlockCell SampleBlock(int wx, int wy, int wz) const override;
	void GenerateColumn(CChunkColumn& column) const override;

private:
	BlockCell _MakeBlockCell(const char* blockName) const;

private:
	WorldGenerateSettings m_settings{};

	BlockCell m_air{};
	BlockCell m_bedrock{};
	BlockCell m_stone{};
	BlockCell m_dirt{};
};