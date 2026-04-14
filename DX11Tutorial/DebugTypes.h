#pragma once
#include <cstdint>
#include <DirectXMath.h>

using namespace DirectX;

struct FrameDebugStats
{
    float fps = 0.f;
    float frameMs = 0.f;

    float updateMs = 0.f;
    float lateUpdateMs = 0.f;
    float renderBuildMs = 0.f;
    float renderExecuteMs = 0.f;
    float presentMs = 0.f;
};

struct DebugHistory
{
    static constexpr int kMaxSamples = 180;
    
    float frameMs[kMaxSamples] = {};
    float drawCalls[kMaxSamples] = {};
    float visibleSections[kMaxSamples] = {};
    float rebuildQueue[kMaxSamples] = {};

    int head = 0;
};

struct WorldDebugStats
{
    int loadedColumnCount = 0;
    int loadedSectionCount = 0;
    int visibleSectionCount = 0;

    int dirtySectionCount = 0;
    int rebuildQueuedCount = 0;
    int rebuiltThisFrameCount = 0;

    int hiddenSectionCount = 0;
    int culledSectionCount = 0;

    int modifiedColumnCount = 0;
    int modifiedCellCount = 0;

    int chunkLoadCountThisFrame = 0;
    int chunkUnloadCountThisFrame = 0;
    int blockEditCountThisFrame = 0;
};

struct RenderDebugStats
{
    int submittedRenderItemCount = 0;

    int drawCallCount = 0;

    int drawCallCountOpaque = 0;
    int drawCallCountSky = 0;
    int drawCallCountShadow = 0;
    int drawCallCountCutout = 0;
    int drawCallCountTransparent = 0;
    int drawCallCountDebug = 0;
    int drawCallCountUI = 0;

    int pipelineBindCount = 0;
    int materialBindCount = 0;
    int meshBindCount = 0;
};

struct PlayerDebugStats
{
    bool hasBlockHit = false;

    XMFLOAT3 playerPos = {};
    XMINT3 currentChunkCoord = {};
    XMINT3 currentBlockCoord = {};

    XMINT3 hitBlock = {};
    XMINT3 hitNormal = {};

    uint16_t targetBlockId = 0;
    uint16_t targetStateIndex = 0;
};

struct DebugStatsSnapshot
{
    FrameDebugStats frame;
    WorldDebugStats world;
    RenderDebugStats render;
    PlayerDebugStats player;
};