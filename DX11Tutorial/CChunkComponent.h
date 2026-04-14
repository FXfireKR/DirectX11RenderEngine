#pragma once
#include "CComponentBase.h"
#include "ChunkTypes.h"

class CMesh;
class CMeshRenderer;

class CChunkComponent : public CComponentBase<CChunkComponent, COMPONENT_TYPE::CUSTOM_0>
{
public:
	CChunkComponent() = default;
	~CChunkComponent() override = default;

	void Init() override;

public:
	BLOCK_ID GetBlock(int x, int y, int z) const;
	void SetBlock(int x, int y, int z, BLOCK_ID id);

	inline const ChunkCoord& GetChunkCoord() const { return m_kCoord; }
	inline void SetChunkCoord(const ChunkCoord& c) { m_kCoord = c; }

	inline void SetMesh(CMesh* pMesh) { m_pMesh = pMesh; }
	inline void SetMeshRenderer(CMeshRenderer* pMeshRenderer) { m_pMeshRender = pMeshRenderer; }

	inline CMesh* GetMesh() const { return m_pMesh; }
	inline CMeshRenderer* GetMeshRenderer() const { return m_pMeshRender; }

	inline const bool& IsDirty() const { return m_bDirty; }
	inline void ClearDirty() { m_bDirty = false; }

private:
	ChunkCoord m_kCoord{};
	array<BLOCK_ID, CHUNK_VOLUME> m_arrayBlocks{};
	bool m_bDirty = false;
	
	CMesh* m_pMesh = nullptr;
	CMeshRenderer* m_pMeshRender = nullptr;
};