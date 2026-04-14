#include "pch.h"
#include "CChunkMeshBuilder.h"
#include "CChunkWorld.h"



bool CChunkMeshBuilder::BuildSectionMeshes(const CChunkWorld& world, int cx, int sy, int cz
    , const CChunkSection& section, ChunkSectionMeshSet& outMeshes) const
{
    PROFILE_SCOPE();

    outMeshes.Clear();

    const int baseWx = cx * CHUNK_SIZE_X;
    const int baseWy = sy * CHUNK_SECTION_SIZE;
    const int baseWz = cz * CHUNK_SIZE_Z;

    for (int ly = 0; ly < CHUNK_SECTION_SIZE; ++ly)
    {
        for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz)
        {
            for (int lx = 0; lx < CHUNK_SIZE_X; ++lx)
            {
                const BlockCell cell = section.GetBlock(lx, ly, lz);

                if (cell.IsAir())
                    continue;

                if (BlockDB.GetRenderLayer(cell.blockID) == BLOCK_RENDER_LAYER::INVISIBLE_LAYER)
                    continue;
    
                const int wx = baseWx + lx;
                const int wy = baseWy + ly;
                const int wz = baseWz + lz;
    
                _AppendBlockQuads(world, wx, wy, wz, lx, ly, lz, cell, outMeshes);
            }
        }
    }    
    return true;
}

bool CChunkMeshBuilder::_AppendBlockQuads(const CChunkWorld& world, int wx, int wy, int wz, int lx, int ly, int lz
    , const BlockCell& cell, ChunkSectionMeshSet& outMeshes) const
{
    PROFILE_SCOPE();

    // 패스트트랙
    if (_TryAppendFastOpaqueCube(world, wx, wy, wz, lx, ly, lz, cell, outMeshes))
        return true;

    ChunkMeshData* pTargetMesh = nullptr;

    switch (BlockDB.GetRenderLayer(cell.blockID))
    {
        case BLOCK_RENDER_LAYER::CUTOUT_LAYER:
        {
            pTargetMesh = &outMeshes.cutout;
        } break;

        case BLOCK_RENDER_LAYER::TRANSLUCENT_LAYER:
        {
            pTargetMesh = &outMeshes.translucent;
        } break;

        case BLOCK_RENDER_LAYER::OPAQUE_LAYER:
        default:
        {
            pTargetMesh = &outMeshes.opaque;
        } break;
    }

#ifdef OPTIMIZATION_1
    const vector<AppliedModel>* vecModels;
    if (!BlockDB.GetAppliedModels(cell.blockID, cell.stateIndex, vecModels) || !vecModels)
        return false;

    PROFILE_SCOPE("ModelAppend");

    const AppliedModel& applied = *(vecModels->begin());

    const BakedModel* pBakedModel = BlockDB.FindBakedModel(applied.modelHash);
    if (!pBakedModel)
        return false;

    PROFILE_SCOPE("AppendModelQuads");
    if (applied.rotate)
    {
        for (const BakedQuad& srcQuad : pBakedModel->quads)
        {
            BakedQuad quad = srcQuad;
            _ApplyModelRotation(quad, applied.x, applied.y);

            if (quad.bHasCullFace)
                if (_ShouldCullFace(world, wx, wy, wz, cell, static_cast<FACE_DIR>(quad.cullFaceDir)))
                    continue;

            _AppendQuad(world, quad, wx, wy, wz, lx, ly, lz, *pTargetMesh);
        }
    }
    else // (!applied.rotate)
    {
        for (const BakedQuad& srcQuad : pBakedModel->quads)
        {
            if (srcQuad.bHasCullFace)
                if (_ShouldCullFace(world, wx, wy, wz, cell, static_cast<FACE_DIR>(srcQuad.cullFaceDir)))
                    continue;

            _AppendQuad(world, srcQuad, wx, wy, wz, lx, ly, lz, *pTargetMesh);
        }
    }
    
#else // OPTIMIZATION_1
    vector<AppliedModel> vecModels;
    if (!BlockDB.GetAppliedModels(cell.blockID, cell.stateIndex, vecModels))
        return false;

    PROFILE_SCOPE("ModelAppend");
    for (const AppliedModel& applied : vecModels)
    {
        const BakedModel* pBakedModel = BlockDB.FindBakedModel(applied.modelHash);
        if (!pBakedModel)
            continue;

        PROFILE_SCOPE("AppendModelQuads");
        for (const BakedQuad& srcQuad : pBakedModel->quads)
        {
            BakedQuad quad = srcQuad;
            if (applied.rotate)
                _ApplyModelRotation(quad, applied.x, applied.y);

            if (quad.bHasCullFace)
                if (_ShouldCullFace(world, wx, wy, wz, cell, static_cast<FACE_DIR>(quad.cullFaceDir)))
                    continue;

            _AppendQuad(world, quad, wx, wy, wz, lx, ly, lz, *pTargetMesh);
        }
    }
#endif // OPTIMIZATION_1
    
    return true;
}

bool CChunkMeshBuilder::_ShouldCullFace(const CChunkWorld& world, int wx, int wy, int wz, const BlockCell& cell, FACE_DIR dir) const
{
    PROFILE_SCOPE();

    XMINT3 n{};
    switch (dir)
    {
        case FACE_DIR::PX: n = { 1, 0, 0 }; break;
        case FACE_DIR::NX: n = { -1, 0, 0 }; break;
        case FACE_DIR::PY: n = { 0, 1, 0 }; break;
        case FACE_DIR::NY: n = { 0, -1, 0 }; break;
        case FACE_DIR::PZ: n = { 0, 0, 1 }; break;
        case FACE_DIR::NZ: n = { 0, 0, -1 }; break;
        default: break;
    }

    const BlockCell neighbor = world.GetBlock(wx + n.x, wy + n.y, wz + n.z);
    if (neighbor.IsAir())
        return false;

    const BLOCK_RENDER_LAYER curLayer = BlockDB.GetRenderLayer(cell.blockID);
    const BLOCK_RENDER_LAYER neighborLayer = BlockDB.GetRenderLayer(neighbor.blockID);

    // 단, 같은 translucent 블록끼리는 내부면 제거
    if (cell.blockID == neighbor.blockID &&
        cell.stateIndex == neighbor.stateIndex &&
        BlockDB.CanCullSameBlockFace(cell.blockID))
    {
        return true;
    }

    // translucent가 한쪽이라도 끼면 기본은 컬링하지 않음
    if (curLayer == BLOCK_RENDER_LAYER::TRANSLUCENT_LAYER ||
        neighborLayer == BLOCK_RENDER_LAYER::TRANSLUCENT_LAYER)
    {
        return false;
    }

    return BlockDB.IsFaceOccluder(neighbor.blockID);    
}

bool CChunkMeshBuilder::_AppendQuad(const CChunkWorld& world, const BakedQuad& quad
    , int wx, int wy, int wz, int lx, int ly, int lz, ChunkMeshData& outMesh) const
{
    PROFILE_SCOPE();
    AtlasRegion region{};

    // TODO: #1 Change String key to Hash key
    if (!BlockResDB.TryGetRegion(quad.debugTextureKey.c_str(), region))
        return false;

    const uint32_t baseIndex = static_cast<uint32_t>(outMesh.vertices.size());
    const XMFLOAT4 color = _ResolveQuadColor_DebugBlockLight(world, quad, wx, wy, wz);

    for (int i = 0; i < 4; ++i)
    {
        ChunkMeshVertex v{};
        v.position.x = quad.verts[i].pos.x + static_cast<float>(lx);
        v.position.y = quad.verts[i].pos.y + static_cast<float>(ly);
        v.position.z = quad.verts[i].pos.z + static_cast<float>(lz);
        v.normal = quad.verts[i].normal;
        v.color = color;
        v.uv = _RemapAtlasUV(region, quad.verts[i].uv);
        
        outMesh.vertices.push_back(v);
    }

    outMesh.indices.push_back(baseIndex + 0);
    outMesh.indices.push_back(baseIndex + 1);
    outMesh.indices.push_back(baseIndex + 2);
    outMesh.indices.push_back(baseIndex + 0);
    outMesh.indices.push_back(baseIndex + 2);
    outMesh.indices.push_back(baseIndex + 3);
    return true;
}

uint8_t CChunkMeshBuilder::_ResolveQuadBlockLight(const CChunkWorld& world, const BakedQuad& quad, int wx, int wy, int wz) const
{
    if (quad.bHasCullFace)
    {
        const XMINT3 n = FaceToNormalInt3(static_cast<FACE_DIR>(quad.cullFaceDir));
        return world.GetBlockLight(wx + n.x, wy + n.y, wz + n.z);
    }
    return world.GetBlockLight(wx, wy, wz);
}

XMFLOAT4 CChunkMeshBuilder::_ResolveQuadColor_DebugBlockLight(const CChunkWorld& world, const BakedQuad& quad, int wx, int wy, int wz) const
{
    const uint8_t light = _ResolveQuadBlockLight(world, quad, wx, wy, wz);
    const float blockLight01 = static_cast<float>(light) / 15.0f;

    XMFLOAT4 tint = ResolveBlockTint(quad);

    return { tint.x, tint.y, tint.z, blockLight01 };
}

XMFLOAT3 CChunkMeshBuilder::_RotatePointByBlockState(const XMFLOAT3& p, int rotXDeg, int rotYDeg) const
{
    XMVECTOR v = XMVectorSet(p.x - 0.5f, p.y - 0.5f, p.z - 0.5f, 0.0f);

    if (rotXDeg != 0)
    {
        const float rx = XMConvertToRadians(static_cast<float>(rotXDeg));
        v = XMVector3Transform(v, XMMatrixRotationX(rx));
    }

    if (rotYDeg != 0)
    {
        const float ry = XMConvertToRadians(static_cast<float>(rotYDeg));
        v = XMVector3Transform(v, XMMatrixRotationY(ry));
    }

    XMFLOAT3 out{};
    XMStoreFloat3(&out, v);

    out.x += 0.5f;
    out.y += 0.5f;
    out.z += 0.5f;
    return out;
}

XMFLOAT3 CChunkMeshBuilder::_RotateNormalByBlockState(const XMFLOAT3& n, int rotXDeg, int rotYDeg) const
{
    XMVECTOR v = XMVectorSet(n.x, n.y, n.z, 0.0f);

    if (rotXDeg != 0)
    {
        const float rx = XMConvertToRadians(static_cast<float>(rotXDeg));
        v = XMVector3TransformNormal(v, XMMatrixRotationX(rx));
    }

    if (rotYDeg != 0)
    {
        const float ry = XMConvertToRadians(static_cast<float>(rotYDeg));
        v = XMVector3TransformNormal(v, XMMatrixRotationY(ry));
    }

    v = XMVector3Normalize(v);

    XMFLOAT3 out{};
    XMStoreFloat3(&out, v);
    return out;
}

FACE_DIR CChunkMeshBuilder::_RotateFaceDirY(FACE_DIR dir, int rotYDeg) const
{
    const int step = (rotYDeg / 90) & 3;
    FACE_DIR cur = dir;

    for (int i = 0; i < step; ++i)
    {
        switch (cur)
        {
        case FACE_DIR::PZ: cur = FACE_DIR::PX; break;
        case FACE_DIR::PX: cur = FACE_DIR::NZ; break;
        case FACE_DIR::NZ: cur = FACE_DIR::NX; break;
        case FACE_DIR::NX: cur = FACE_DIR::PZ; break;
        default: break; // PY / NY unchanged
        }
    }

    return cur;
}

void CChunkMeshBuilder::_ApplyModelRotation(BakedQuad& quad, int rotXDeg, int rotYDeg) const
{
    PROFILE_SCOPE();
    const int fixedY = static_cast<int>((360 - rotYDeg) % 360);

    for (int i = 0; i < 4; ++i)
    {
        quad.verts[i].pos = _RotatePointByBlockState(quad.verts[i].pos, rotXDeg, fixedY);
        quad.verts[i].normal = _RotateNormalByBlockState(quad.verts[i].normal, rotXDeg, fixedY);
    }

    // 지금 wall_torch는 y 회전만 쓰므로 dir/cullFace도 y만 반영
    quad.dir = static_cast<uint8_t>(_RotateFaceDirY(static_cast<FACE_DIR>(quad.dir), fixedY));

    if (quad.bHasCullFace)
    {
        quad.cullFaceDir = static_cast<uint8_t>(
            _RotateFaceDirY(static_cast<FACE_DIR>(quad.cullFaceDir), fixedY));
    }
}

XMFLOAT4 CChunkMeshBuilder::ResolveBlockTint(const BakedQuad& quad) const
{
    if (quad.tintIndex < 0)
        return { 1.f, 1.f, 1.f, 1.f };

        // 나중에 biom grass color sampler
    return {0.55f, 0.74f, 0.32f, 1.f};
}

XMFLOAT2 CChunkMeshBuilder::_RemapAtlasUV(const AtlasRegion& region, const XMFLOAT2& localUV) const
{
    XMFLOAT2 outUV;
    outUV.x = region.u0 + (region.u1 - region.u0) * localUV.x;
    outUV.y = region.v0 + (region.v1 - region.v0) * localUV.y;
    return outUV;
}

bool CChunkMeshBuilder::_TryAppendFastOpaqueCube(const CChunkWorld& world,
    int wx, int wy, int wz, int lx, int ly, int lz,
    const BlockCell& cell, ChunkSectionMeshSet& outMeshes) const
{
    PROFILE_SCOPE();
    if (BlockDB.GetRenderLayer(cell.blockID) != BLOCK_RENDER_LAYER::OPAQUE_LAYER)
        return false;

    if (!BlockDB.IsFullCube(cell.blockID))
        return false;

    const FastCubeCache& cache = _GetOrBuildFastCubeCache(cell.blockID, cell.stateIndex);
    if (!cache.canUse)
        return false;

    ChunkMeshData& mesh = outMeshes.opaque;

    for (const FastCubeFaceCache& faceCache : cache.faces)
    {
        if (!faceCache.valid)
            return false;

        if (_ShouldCullFace(world, wx, wy, wz, cell, faceCache.face))
            continue;

        _AppendFastCubeFace(world, faceCache, wx, wy, wz, lx, ly, lz, mesh);
    }
    return true;
}

const FastCubeCache& CChunkMeshBuilder::_GetOrBuildFastCubeCache(BLOCK_ID blockID, STATE_INDEX stateIndex) const
{
    const FastCubeKey key = MakeFastCubeKey(blockID, stateIndex);

    auto it = g_fastCubeCache.find(key);
    if (it != g_fastCubeCache.end())
        return it->second;

    FastCubeCache cache{};
    cache.initialized = true;
    cache.canUse = false;

    if (BlockDB.GetRenderLayer(blockID) != BLOCK_RENDER_LAYER::OPAQUE_LAYER)
    {
        auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
        return newIt->second;
    }

    if (!BlockDB.IsFullCube(blockID))
    {
        auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
        return newIt->second;
    }

    const vector<AppliedModel>* pModels = nullptr;
    if (!BlockDB.GetAppliedModels(blockID, stateIndex, pModels) || !pModels || pModels->empty())
    {
        auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
        return newIt->second;
    }

    const AppliedModel& applied = (*pModels)[0];
    if (applied.rotate)
    {
        auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
        return newIt->second;
    }

    const BakedModel* pModel = BlockDB.FindBakedModel(applied.modelHash);
    if (!pModel || pModel->quads.size() != 6)
    {
        auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
        return newIt->second;
    }

    bool used[6] = {};

    for (const BakedQuad& quad : pModel->quads)
    {
        if (!quad.bHasCullFace)
        {
            auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
            return newIt->second;
        }

        const int faceIdx = FaceDirToIndex(static_cast<FACE_DIR>(quad.cullFaceDir));
        if (faceIdx < 0 || used[faceIdx])
        {
            auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
            return newIt->second;
        }

        AtlasRegion region{};
        if (!BlockResDB.TryGetRegion(quad.debugTextureKey.c_str(), region))
        {
            auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
            return newIt->second;
        }

        used[faceIdx] = true;
        cache.faces[faceIdx].valid = true;
        cache.faces[faceIdx].face = static_cast<FACE_DIR>(quad.cullFaceDir);
        cache.faces[faceIdx].region = region;
        cache.faces[faceIdx].tintIndex = quad.tintIndex;
        cache.faces[faceIdx].quad = quad;
    }

    for (bool b : used)
    {
        if (!b)
        {
            auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
            return newIt->second;
        }
    }

    cache.canUse = true;
    auto [newIt, _] = g_fastCubeCache.emplace(key, cache);
    return newIt->second;
}

void CChunkMeshBuilder::_AppendFastCubeFace(const CChunkWorld& world,
    const FastCubeFaceCache& faceCache,
    int wx, int wy, int wz, int lx, int ly, int lz,
    ChunkMeshData& outMesh) const
{
    const uint32_t baseIndex = static_cast<uint32_t>(outMesh.vertices.size());

    const uint8_t light = _ResolveQuadBlockLight(world, faceCache.quad, wx, wy, wz);
    const float blockLight01 = static_cast<float>(light) / 15.0f;

    XMFLOAT4 tint = { 1.f, 1.f, 1.f, 1.f };
    if (faceCache.tintIndex >= 0)
        tint = { 0.55f, 0.74f, 0.32f, 1.f };

    const XMFLOAT4 color =
    {
        tint.x,
        tint.y,
        tint.z,
        blockLight01
    };

    for (int i = 0; i < 4; ++i)
    {
        ChunkMeshVertex v{};
        v.position.x = faceCache.quad.verts[i].pos.x + static_cast<float>(lx);
        v.position.y = faceCache.quad.verts[i].pos.y + static_cast<float>(ly);
        v.position.z = faceCache.quad.verts[i].pos.z + static_cast<float>(lz);
        v.normal = faceCache.quad.verts[i].normal;
        v.color = color;
        v.uv = _RemapAtlasUV(faceCache.region, faceCache.quad.verts[i].uv);
        outMesh.vertices.push_back(v);
    }

    outMesh.indices.push_back(baseIndex + 0);
    outMesh.indices.push_back(baseIndex + 1);
    outMesh.indices.push_back(baseIndex + 2);
    outMesh.indices.push_back(baseIndex + 0);
    outMesh.indices.push_back(baseIndex + 2);
    outMesh.indices.push_back(baseIndex + 3);
}