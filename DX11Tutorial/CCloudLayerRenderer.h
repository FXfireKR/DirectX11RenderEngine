#pragma once
#include "CRenderWorld.h"

class CCloudLayerRenderer
{
private:
    struct CloudCell { int x = 0; int z = 0; };

public:
	CCloudLayerRenderer() = default;
	~CCloudLayerRenderer() = default;

	bool Initialize(CRenderWorld& rw, const wchar_t* maskPathW);
	void Update(float fDelta, const XMFLOAT3& cameraPos);
	void Submit(CRenderWorld& rw);

private:
    bool _LoadMaskWIC(const wchar_t* pathW);
    bool _SampleMask(int worldCellX, int worldCellZ) const;
    void _RebuildMesh(CRenderWorld& rw);

    void _AppendCellFaces(
        int cellX, int cellZ,
        bool pxOpen, bool nxOpen,
        bool pzOpen, bool nzOpen,
        vector<VERTEX_POSITION_COLOR>& outVerts,
        vector<uint32_t>& outIndices) const;

    void _AppendQuad(
        const XMFLOAT3& p0,
        const XMFLOAT3& p1,
        const XMFLOAT3& p2,
        const XMFLOAT3& p3,
        const XMFLOAT4& color,
        vector<VERTEX_POSITION_COLOR>& outVerts,
        vector<uint32_t>& outIndices) const;

private:
    CPipeline* m_pPipeline = nullptr;
    CMesh* m_pMesh = nullptr;

    uint64_t m_uMeshKey = fnv1a_64("CloudLayerDynamicMesh");

    vector<uint8_t> m_mask;
    int m_iMaskWidth = 0;
    int m_iMaskHeight = 0;

    int m_iAnchorCellX = 0;
    int m_iAnchorCellZ = 0;

    int m_iCenterCellX = INT_MIN;
    int m_iCenterCellZ = INT_MIN;

    int m_iScrollCellX = 0;
    float m_fScrollAccum = 0.f;

    int m_iAnchorSnapStep = 8;   // 8셀 단위로만 재배치
    bool m_bDirty = true;

    float m_fCloudBaseY = 132.f;
    float m_fCloudThickness = 3.f;
    float m_fCloudCellSize = 12.f;
    float m_fScrollSpeedCellPerSec = 0.20f;

    int m_iHalfWindow = 96; // 48x48 cells
};