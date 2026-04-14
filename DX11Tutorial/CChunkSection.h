#pragma once
#include "ChunkTypes.h"

class CChunkSection
{
public:
	CChunkSection();
	~CChunkSection() = default;

	BlockCell GetBlock(int lx, int ly, int lz) const;
	void SetBlock(int lx, int ly, int lz, const BlockCell& cell);

public:
#ifdef OPTIMIZATION_2
	inline void MarkDirty() { m_bMeshDirty = m_bLightDirty = true; }
	inline void MarkMeshDirty() { m_bMeshDirty = true; }
	inline void MarkLightDirty() { m_bLightDirty = true; }

	inline const bool IsDirty() const { return m_bMeshDirty || m_bLightDirty; }
	inline const bool IsMeshDirty() const { return m_bMeshDirty; }
	inline const bool IsLightDirty() const { return m_bLightDirty; }

	inline void ClearDirty() { m_bMeshDirty = m_bLightDirty = false; }
	inline void ClearMeshDirty() { m_bMeshDirty = false; }
	inline void ClearLightDirty() { m_bLightDirty = false; }
#else // OPTIMIZATION_2
	inline void MarkDirty() { m_bDirty = true; }
	inline const bool IsDirty() const { return m_bDirty; }
	inline void ClearDirty() { m_bDirty = false; }
#endif // OPTIMIZATION_2

	inline const bool IsBuildQueued() const { return m_bBuildQueued; }
	inline void SetBuildQueued(bool bQueued) { m_bBuildQueued = bQueued; }

	inline bool HasRenderObjectID(EChunkSectionRenderSlot eSlot) const { return IsValidObjectID(m_renderObjectIDs[static_cast<int>(eSlot)]); }
	inline const OBJECT_ID GetRenderObjectID(EChunkSectionRenderSlot eSlot) const { return m_renderObjectIDs[static_cast<int>(eSlot)]; }
	inline void SetRenderObjectID(EChunkSectionRenderSlot eSlot, OBJECT_ID id) { m_renderObjectIDs[static_cast<int>(eSlot)] = id; }
	inline void ClearRenderObjectID(EChunkSectionRenderSlot eSlot) { m_renderObjectIDs[static_cast<int>(eSlot)] = INVALID_OBJECT_ID; }
	inline void ClearAllRenderObjectIDs()
	{
		for (OBJECT_ID& id : m_renderObjectIDs)
			id = INVALID_OBJECT_ID;
	}

	inline bool HasAnyRenderObjectID() const
	{
		for (OBJECT_ID id : m_renderObjectIDs)
		{
			if (IsValidObjectID(id))
				return true;
		}
		return false;
	}

	inline bool IsEmpty() const { return m_nonAirCount == 0; }
	
private:
	array<BlockCell, CHUNK_SECTION_VOLUME> m_cells{};
	uint16_t m_nonAirCount = 0;

#ifdef OPTIMIZATION_2
	bool m_bMeshDirty = false;
	bool m_bLightDirty = false;
#else // OPTIMIZATION_2
	bool m_bDirty = false;
#endif // OPTIMIZATION_2
	bool m_bBuildQueued = false;

	// handle
	array<OBJECT_ID, static_cast<size_t>(EChunkSectionRenderSlot::COUNT)> m_renderObjectIDs
	{
		INVALID_OBJECT_ID,
		INVALID_OBJECT_ID,
		INVALID_OBJECT_ID
	};
};


/*
* 분리한 이유는 Section은 비어있을 수 있지만 Light는 공간으로 전파되므로, 둘은 공존할 수 없는 경우가 존재함.
* 따라서 분리.
*/

class CChunkLightSection
{
public:
	uint8_t GetBlockLight(int lx, int ly, int lz) const;
	void SetBlockLight(int lx, int ly, int lz, uint8_t level);
	void Clear();

	inline bool IsAllZero() const { return m_nonZeroCount == 0; }

private:
	array<uint8_t, CHUNK_SECTION_VOLUME> m_blockLight{};
	uint16_t m_nonZeroCount = 0;
};