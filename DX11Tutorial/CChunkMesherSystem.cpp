#include "pch.h"
#include "CChunkMesherSystem.h"
#include "CScene.h"
#include "CObject.h"
#include "CChunkMeshBuilder.h"
#include "CRenderWorld.h"
#include "CMeshRenderer.h"
#include "CMeshManager.h"
#include "CChunkWorld.h"

static uint64_t MakeChunkMeshKey(int cx, int cy, int cz, EChunkSectionRenderSlot slot)
{
	// packing
	uint64_t x = (uint32_t)(cx & 0xFFFF);
	uint64_t y = (uint32_t)(cy & 0xFFFF);
	uint64_t z = (uint32_t)(cz & 0xFFFF);
	uint64_t s = (uint64_t)(static_cast<uint8_t>(slot) & 0xFF);


	return (x) | (y << 16) | (z << 32) | (s << 48);
}

void CChunkMesherSystem::RebuildDirtyChunks(CScene& scene, CChunkWorld& world)
{
	PROFILE_SCOPE();

	CChunkMeshBuilder builder;
	SectionCoord sectionCoord{};

	constexpr int MIN_BUDGET = 4;
	//int iCurrentBudget = std::max(MIN_BUDGET, static_cast<int>(world.GetDirtyQueueSize() / 100));

	int iCurrentBudget = MIN_BUDGET;

	while (iCurrentBudget > 0 && world.PopDirty(sectionCoord))
	{
		CChunkSection* pSection = world.FindSectionDataMutable(sectionCoord.x, sectionCoord.y, sectionCoord.z);
		if (nullptr == pSection) 
			continue;

#ifdef OPTIMIZATION_2
		const bool bMeshDirty = pSection->IsMeshDirty();
		const bool bLightDirty = pSection->IsLightDirty();
#endif // OPTIMIZATION_2

		pSection->SetBuildQueued(false);

#ifdef OPTIMIZATION_2
		if (!bMeshDirty && !bLightDirty)
			continue;
#else // OPTIMIZATION_2
		if (!pSection->IsDirty())
			continue;
#endif // OPTIMIZATION_2

		ChunkSectionMeshSet meshSet;
		builder.BuildSectionMeshes(world, sectionCoord.x, sectionCoord.y, sectionCoord.z, *pSection, meshSet);

		UploadSectionMesh(scene, world, sectionCoord, EChunkSectionRenderSlot::OPAQUE_SLOT, meshSet.opaque);
		UploadSectionMesh(scene, world, sectionCoord, EChunkSectionRenderSlot::CUTOUT_SLOT, meshSet.cutout);
		UploadSectionMesh(scene, world, sectionCoord, EChunkSectionRenderSlot::TRANSLUCENT_SLOT, meshSet.translucent);

#ifdef OPTIMIZATION_2
		if (bMeshDirty)
			pSection->ClearDirty();

		if (bMeshDirty || bLightDirty)
			pSection->ClearLightDirty();
#else // OPTIMIZATION_2
		pSection->ClearDirty();
#endif // OPTIMIZATION_2

		dbg.AddRebuiltSection();
		--iCurrentBudget;
	}
}

void CChunkMesherSystem::UploadSectionMesh(CScene& scene, CChunkWorld& world, const SectionCoord& sectionCoord
	, EChunkSectionRenderSlot slot, const ChunkMeshData& meshData)
{
	PROFILE_SCOPE();

	CObject* pRenderObject = world.FindRenderObject(sectionCoord.x, sectionCoord.y, sectionCoord.z, slot);
	if (nullptr == pRenderObject)
		return;

	auto* meshRender = pRenderObject->GetComponent<CMeshRenderer>();
	if (nullptr == meshRender)
		return;

	const uint64_t meshKey = MakeChunkMeshKey(sectionCoord.x, sectionCoord.y, sectionCoord.z, slot);
	
	CMesh* mesh = scene.GetRenderWorld().GetMeshManager().CreateOrUpdateDynamicMesh(
		scene.GetRenderWorld().GetContext(),
		meshKey,
		meshData.vertices.data(),
		sizeof(ChunkMeshVertex),
		static_cast<uint32_t>(meshData.vertices.size()),
		meshData.indices.data(),
		static_cast<uint32_t>(meshData.indices.size())
	);

	if (nullptr == mesh)
		return;

	meshRender->SetMesh(mesh);
	pRenderObject->SetEnable(!meshData.Empty());
}
