#pragma once
#include "ChunkTypes.h"

class IBlockAccessor
{
public:
	IBlockAccessor() = default;
	virtual ~IBlockAccessor() = default;

	virtual bool CanRaycastHit(const BlockCell& cell) const PURE;
	virtual BlockCell GetBlock(int wx, int wy, int wz) const PURE;
	virtual bool IsSolid(const BlockCell& cell) const PURE;
};

struct BlockRaycastOptions
{
	bool bCheckStartCell = true;
	bool bTreatFluidAsSolid = false;
	bool bTreatReplaceableAsSolid = false;
};