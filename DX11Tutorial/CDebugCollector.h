#pragma once
#include "DebugTypes.h"
#include "singleton.h"

class CDebugCollector : public singleton<CDebugCollector>
{
public:
    void BeginFrame();
    void EndFrame();
    
    inline const DebugStatsSnapshot& GetSnapshot() const { return m_displayShot; }
    inline const DebugHistory& GetHistory() const { return m_history; }

public:
    // -------------------------------------------------
    // Frame
    // -------------------------------------------------
    inline void SetFPS(float fps) { m_snapshot.frame.fps = fps; }
    inline void SetFrameMs(float ms) { m_snapshot.frame.frameMs = ms; } 
    inline void SetUpdateMs(float ms) { m_snapshot.frame.updateMs = ms; } 
    inline void SetLateUpdateMs(float ms) { m_snapshot.frame.lateUpdateMs = ms; }
    inline void SetRenderBuildMs(float ms) { m_snapshot.frame.renderBuildMs = ms; }
    inline void SetRenderExecuteMs(float ms) { m_snapshot.frame.renderExecuteMs = ms; }
    inline void SetPresentMs(float ms) { m_snapshot.frame.presentMs = ms; }

public:
    // -------------------------------------------------
    // World
    // -------------------------------------------------
    inline void SetLoadedColumnCount(int count) { m_snapshot.world.loadedColumnCount = count; }
    inline void SetLoadedSectionCount(int count) { m_snapshot.world.loadedSectionCount = count; }
    inline void SetVisibleSectionCount(int count) { m_snapshot.world.visibleSectionCount = count; }
    inline void SetDirtySectionCount(int count) { m_snapshot.world.dirtySectionCount = count; }
    inline void SetRebuildQueuedCount(int count) { m_snapshot.world.rebuildQueuedCount = count; }
    inline void SetModifiedColumnCount(int count) { m_snapshot.world.modifiedColumnCount = count; }
    inline void SetModifiedCellCount(int count) { m_snapshot.world.modifiedCellCount = count; }

    void AddChunkLoad(int count = 1);
    void AddChunkUnload(int count = 1);
    void AddBlockEdit(int count = 1);
    void AddRebuiltSection(int count = 1);
    void AddHiddenSection(int count = 1);
    void AddCulledSection(int count = 1);

public:
    // -------------------------------------------------
    // Render
    // -------------------------------------------------
    void AddSubmittedRenderItem(int count = 1);

    void AddDrawCall(int count = 1);
    void AddDrawCallOpaque(int count = 1);
    void AddDrawCallSky(int count = 1);
    void AddDrawCallShadow(int count = 1);
    void AddDrawCallCutout(int count = 1);
    void AddDrawCallTranslucent(int count = 1);
    void AddDrawCallDebug(int count = 1);
    void AddDrawCallUI(int count = 1);

    void AddPipelineBind(int count = 1);
    void AddMaterialBind(int count = 1);
    void AddMeshBind(int count = 1);

public:
    // -------------------------------------------------
    // Player / Debug Interaction
    // -------------------------------------------------
    inline void SetPlayerPosition(const XMFLOAT3& pos) { m_snapshot.player.playerPos = pos; }
    inline void SetCurrentChunkCoord(const XMINT3& coord) { m_snapshot.player.currentChunkCoord = coord; }
    inline void SetCurrentBlockCoord(const XMINT3& coord) { m_snapshot.player.currentBlockCoord = coord; }

    void ClearBlockHit();
    void SetBlockHit(const XMINT3& block, const XMINT3& normal
        , uint16_t blockId, uint16_t stateIndex);

public:
    inline void FlipDebugOverlay() { m_bShowOverlay = !m_bShowOverlay; }
    inline bool IsVisible() const { return m_bShowOverlay; }

private:
    void _ResetPerFrameCounters();
    void _PushHistory();

private:
    DebugStatsSnapshot m_snapshot; // collect snapshot
    DebugStatsSnapshot m_displayShot; // N-1 Frame SnapShot
    DebugHistory m_history;

    bool m_bShowOverlay = false;
};