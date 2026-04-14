#pragma once
#include "ChunkTypes.h"

class CScene;
class CChunkWorld;

class CChunkMesherSystem
{
public:
	static void RebuildDirtyChunks(CScene& scene, CChunkWorld& world);
	static void UploadSectionMesh(CScene& scene, CChunkWorld& world, const SectionCoord& sectionCoord
		, EChunkSectionRenderSlot slot, const ChunkMeshData& meshData);
};