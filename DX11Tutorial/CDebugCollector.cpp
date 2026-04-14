#include "pch.h"
#include "CDebugCollector.h"

void CDebugCollector::BeginFrame()
{
    _ResetPerFrameCounters();
}

void CDebugCollector::EndFrame()
{
    // 필요하면 나중에 history push, 평균 계산 등을 여기서 수행
    m_displayShot = m_snapshot;
    _PushHistory();
}

void CDebugCollector::AddChunkLoad(int count) 
{ 
    m_snapshot.world.chunkLoadCountThisFrame += count; 
}

void CDebugCollector::AddChunkUnload(int count) 
{
    m_snapshot.world.chunkUnloadCountThisFrame += count; 
}

void CDebugCollector::AddBlockEdit(int count) 
{ 
    m_snapshot.world.blockEditCountThisFrame += count; 
}

void CDebugCollector::AddRebuiltSection(int count) 
{ 
    m_snapshot.world.rebuiltThisFrameCount += count; 
}

void CDebugCollector::AddHiddenSection(int count)
{
    m_snapshot.world.hiddenSectionCount += count;
}

void CDebugCollector::AddCulledSection(int count)
{
    m_snapshot.world.culledSectionCount += count;
}

void CDebugCollector::AddSubmittedRenderItem(int count) 
{ 
    m_snapshot.render.submittedRenderItemCount += count; 
}

void CDebugCollector::AddDrawCall(int count) 
{ 
    m_snapshot.render.drawCallCount += count; 
}

void CDebugCollector::AddDrawCallOpaque(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountOpaque += count;
}
void CDebugCollector::AddDrawCallSky(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountSky += count;
}
void CDebugCollector::AddDrawCallShadow(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountShadow += count;
}
void CDebugCollector::AddDrawCallCutout(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountCutout += count;
}
void CDebugCollector::AddDrawCallTranslucent(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountTransparent += count;
}
void CDebugCollector::AddDrawCallDebug(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountDebug += count;
}
void CDebugCollector::AddDrawCallUI(int count)
{
    m_snapshot.render.drawCallCount += count;
    m_snapshot.render.drawCallCountUI += count;
}

void CDebugCollector::AddPipelineBind(int count) 
{ 
    m_snapshot.render.pipelineBindCount += count;
}

void CDebugCollector::AddMaterialBind(int count) 
{ 
    m_snapshot.render.materialBindCount += count;
}

void CDebugCollector::AddMeshBind(int count) 
{
    m_snapshot.render.meshBindCount += count;
}

void CDebugCollector::ClearBlockHit()
{
    m_snapshot.player.hasBlockHit = false;
    m_snapshot.player.hitBlock = {};
    m_snapshot.player.hitNormal = {};
    m_snapshot.player.targetBlockId = 0;
    m_snapshot.player.targetStateIndex = 0;
}

void CDebugCollector::SetBlockHit(const XMINT3& block, const XMINT3& normal, uint16_t blockId, uint16_t stateIndex)
{
    m_snapshot.player.hasBlockHit = true;
    m_snapshot.player.hitBlock = block;
    m_snapshot.player.hitNormal = normal;
    m_snapshot.player.targetBlockId = blockId;
    m_snapshot.player.targetStateIndex = stateIndex;
}

void CDebugCollector::_ResetPerFrameCounters()
{
    // Frame 시간은 매 프레임 overwrite될 값이라 초기화해도 되고, 바깥에서 Set 하게 둬도 됨.
    m_snapshot.frame.fps = 0.f;
    m_snapshot.frame.frameMs = 0.f;
    m_snapshot.frame.updateMs = 0.f;
    m_snapshot.frame.lateUpdateMs = 0.f;
    m_snapshot.frame.renderBuildMs = 0.f;
    m_snapshot.frame.renderExecuteMs = 0.f;
    m_snapshot.frame.presentMs = 0.f;

    // 상태값(state)은 유지할 수 있는건 유지함
    // loaded/visible/dirty 같은 값은 보통 매 프레임 재설정하는 편이 안전
    //m_snapshot.world.visibleSectionCount = 0;
    //m_snapshot.world.dirtySectionCount = 0;
    //m_snapshot.world.rebuildQueuedCount = 0;
    m_snapshot.world.rebuiltThisFrameCount = 0;
    m_snapshot.world.chunkLoadCountThisFrame = 0;
    m_snapshot.world.chunkUnloadCountThisFrame = 0;
    m_snapshot.world.blockEditCountThisFrame = 0;
    //m_snapshot.world.hiddenSectionCount = 0;
    //m_snapshot.world.culledSectionCount = 0;

    m_snapshot.render.submittedRenderItemCount = 0;
    m_snapshot.render.drawCallCount = 0;
    m_snapshot.render.drawCallCountOpaque = 0;
    m_snapshot.render.drawCallCountSky = 0;
    m_snapshot.render.drawCallCountShadow = 0;
    m_snapshot.render.drawCallCountCutout = 0;
    m_snapshot.render.drawCallCountTransparent = 0;
    m_snapshot.render.drawCallCountDebug = 0;
    m_snapshot.render.drawCallCountUI = 0;
    m_snapshot.render.pipelineBindCount = 0;
    m_snapshot.render.materialBindCount = 0;
    m_snapshot.render.meshBindCount = 0;

    m_snapshot.player.hasBlockHit = false;
    m_snapshot.player.hitBlock = {};
    m_snapshot.player.hitNormal = {};
    m_snapshot.player.targetBlockId = 0;
    m_snapshot.player.targetStateIndex = 0;
}

void CDebugCollector::_PushHistory()
{
    const int idx = m_history.head;

    m_history.frameMs[idx] = m_displayShot.frame.frameMs;
    m_history.drawCalls[idx] = static_cast<float>(m_displayShot.render.drawCallCount);
    m_history.visibleSections[idx] = static_cast<float>(m_displayShot.world.visibleSectionCount);
    m_history.rebuildQueue[idx] = static_cast<float>(m_displayShot.world.rebuildQueuedCount);

    m_history.head = (m_history.head + 1) % DebugHistory::kMaxSamples;
}