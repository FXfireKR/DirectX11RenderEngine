#pragma once
#include "ChunkTypes.h"

class CChunkWorld;
class CChunkColumn;
class CChunkSection;

namespace // no name
{
    using FastCubeKey = uint64_t;

    struct FastCubeFaceCache
    {
        bool valid = false;
        FACE_DIR face = FACE_DIR::PX;
        AtlasRegion region{};
        int tintIndex = -1;
        BakedQuad quad{};
    };

    struct FastCubeCache
    {
        bool initialized = false;
        bool canUse = false;
        FastCubeFaceCache faces[6];
    };

    static std::unordered_map<FastCubeKey, FastCubeCache> g_fastCubeCache;

    static FastCubeKey MakeFastCubeKey(BLOCK_ID blockID, STATE_INDEX stateIndex)
    {
        return (static_cast<uint64_t>(blockID) << 32) | static_cast<uint64_t>(stateIndex);
    }

    static int FaceDirToIndex(FACE_DIR dir)
    {
        switch (dir)
        {
        case FACE_DIR::PX: return 0;
        case FACE_DIR::NX: return 1;
        case FACE_DIR::PY: return 2;
        case FACE_DIR::NY: return 3;
        case FACE_DIR::PZ: return 4;
        case FACE_DIR::NZ: return 5;
        default: return -1;
        }
    }
}

class CChunkMeshBuilder
{
public:
	bool BuildSectionMeshes(const CChunkWorld& world, int cx, int sy, int cz
		, const CChunkSection& section, ChunkSectionMeshSet& outMeshes) const;

private:
	bool _AppendBlockQuads(const CChunkWorld& world
		, int wx, int wy, int wz, int lx, int ly, int lz
		, const BlockCell& cell, ChunkSectionMeshSet& outMeshes) const;

	bool _ShouldCullFace(const CChunkWorld& world, int wx, int wy, int wz, const BlockCell& cell
		, FACE_DIR dir) const;

	bool _AppendQuad(const CChunkWorld& world, const BakedQuad& quad
		, int wx, int wy, int wz, int lx, int ly, int lz
		, ChunkMeshData& outMesh) const;

	uint8_t _ResolveQuadBlockLight(const CChunkWorld& world, const BakedQuad& quad
		, int wx, int wy, int wz) const;

	XMFLOAT4 _ResolveQuadColor_DebugBlockLight(const CChunkWorld& world, const BakedQuad& quad
		, int wx, int wy, int wz) const;

	XMFLOAT3 _RotatePointByBlockState(const XMFLOAT3& p, int rotXDeg, int rotYDeg) const;
	XMFLOAT3 _RotateNormalByBlockState(const XMFLOAT3& n, int rotXDeg, int rotYDeg) const;
	FACE_DIR _RotateFaceDirY(FACE_DIR dir, int rotYDeg) const;
	void _ApplyModelRotation(BakedQuad& quad, int rotXDeg, int rotYDeg) const;

	XMFLOAT4 ResolveBlockTint(const BakedQuad& quad) const;
	XMFLOAT2 _RemapAtlasUV(const AtlasRegion& region, const XMFLOAT2& localUV) const;

	bool _TryAppendFastOpaqueCube(const CChunkWorld& world,
		int wx, int wy, int wz, int lx, int ly, int lz,
		const BlockCell& cell, ChunkSectionMeshSet& outMeshes) const;

	const FastCubeCache& _GetOrBuildFastCubeCache(BLOCK_ID blockID, STATE_INDEX stateIndex) const;

	void _AppendFastCubeFace(const CChunkWorld& world,
		const FastCubeFaceCache& faceCache,
		int wx, int wy, int wz, int lx, int ly, int lz,
		ChunkMeshData& outMesh) const;
};