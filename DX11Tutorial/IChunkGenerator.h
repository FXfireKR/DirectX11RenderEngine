#pragma once
#include "CChunkColumn.h"

struct BlockCell;

struct WorldGenerateSettings
{
	uint32_t seed = 12345u;
	int baseHeight = 48;
	int heightAmplitude = 12;
	float frequency = 0.01f;
	int seaLevel = 48;
};

class IChunkGenerator
{
public:
	virtual ~IChunkGenerator() = default;

	virtual void Initialize(const WorldGenerateSettings& settings)  PURE;
	virtual BlockCell SampleBlock(int wx, int wy, int wz) const PURE;
	virtual void GenerateColumn(CChunkColumn& column) const PURE;
};