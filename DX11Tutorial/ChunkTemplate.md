CChunkWorld
  └─ dirty queue / section lookup / render object lookup 제공
  - 스트리밍 로드/언로드
  - dirty queue
  - section lookup
  - render object create/destroy

CChunkMesherSystem
  └─ dirty queue 소비
  └─ CChunkMeshBuilder 호출
  └─ CMeshManager 업로드
  └─ MeshRenderer.SetMesh
  - dirty queue 소비
  - builder 호출
  - dynamic mesh 생성/갱신
  - MeshRenderer.SetMesh()

CChunkMeshBuilder
  └─ section + world 조회로 ChunkMeshData 생성
  └─ face culling
  └─ baked model quad 전개
  └─ atlas uv remap
  - section 데이터 -> ChunkMeshData 생성
  - face culling
  - baked quad append
  - atlas uv remap

static void AddFaceQuad(int x, int y, int z, FACE_DIR eDir, UVRect uv, vector<ChunkMeshVertex>& v, vector<uint32_t>& i)
{
	const XMFLOAT3 n = FaceNormal(eDir);
	XMFLOAT3 p[4];

	switch (eDir)
	{
		case FACE_DIR::PX: // x+ 면
		{
			p[0] = { (float)(x + 1), (float)y,     (float)z };
			p[1] = { (float)(x + 1), (float)(y + 1), (float)z };
			p[2] = { (float)(x + 1), (float)(y + 1), (float)(z + 1) };
			p[3] = { (float)(x + 1), (float)y,     (float)(z + 1) };
		} break;

		case FACE_DIR::NX: // x- 면
		{
			p[0] = { (float)x, (float)y,     (float)(z + 1) };
			p[1] = { (float)x, (float)(y + 1), (float)(z + 1) };
			p[2] = { (float)x, (float)(y + 1), (float)z };
			p[3] = { (float)x, (float)y,     (float)z };
		} break;

		case FACE_DIR::PY: // y+ 면
		{
			p[0] = { (float)x,     (float)(y + 1), (float)z };
			p[1] = { (float)x,     (float)(y + 1), (float)(z + 1) };
			p[2] = { (float)(x + 1), (float)(y + 1), (float)(z + 1) };
			p[3] = { (float)(x + 1), (float)(y + 1), (float)z };
		} break;

		case FACE_DIR::NY: // y- 면
		{
			p[0] = { (float)x,     (float)y, (float)(z + 1) };
			p[1] = { (float)x,     (float)y, (float)z };
			p[2] = { (float)(x + 1), (float)y, (float)z };
			p[3] = { (float)(x + 1), (float)y, (float)(z + 1) };
		} break;

		case FACE_DIR::PZ: // z+ 면
		{
			p[0] = { (float)(x + 1), (float)y,     (float)(z + 1) };
			p[1] = { (float)(x + 1), (float)(y + 1), (float)(z + 1) };
			p[2] = { (float)x,     (float)(y + 1), (float)(z + 1) };
			p[3] = { (float)x,     (float)y,     (float)(z + 1) };
		} break;

		case FACE_DIR::NZ: // z- 면
		default:
		{
			p[0] = { (float)x,     (float)y,     (float)z };
			p[1] = { (float)x,     (float)(y + 1), (float)z };
			p[2] = { (float)(x + 1), (float)(y + 1), (float)z };
			p[3] = { (float)(x + 1), (float)y,     (float)z };
		} break;
	}

	const uint32_t base = (uint32_t)v.size();

	// TODO: UV좌표는 나중에 atlas 방식으로 하면 변경해야함.
	v.push_back({ p[0], n, {uv.u0, uv.v1}, {1,1,1,1} });
	v.push_back({ p[1], n, {uv.u0, uv.v0}, {1,1,1,1} });
	v.push_back({ p[2], n, {uv.u1, uv.v0}, {1,1,1,1} });
	v.push_back({ p[3], n, {uv.u1, uv.v1}, {1,1,1,1} });

	i.push_back(base + 0);
	i.push_back(base + 1);
	i.push_back(base + 2);
	i.push_back(base + 0);
	i.push_back(base + 2);
	i.push_back(base + 3);
}

static bool IsAirWorld(const CChunkWorld& world, int x, int y, int z)
{
	return world.GetBlock(x, y, z).IsAir();
}

static bool IsAirNeighbor(const CChunkWorld& world, const CChunkSection& section, int cx, int sy, int cz, int x, int y, int z)
{
	if (x >= 0 && x < CHUNK_SIZE_X &&
		y >= 0 && y < CHUNK_SECTION_SIZE &&
		z >= 0 && z < CHUNK_SIZE_Z)
	{
		return section.GetBlock(x, y, z).blockID == 0;
	}

	const int wx = cx * CHUNK_SIZE_X + x;
	const int wy = sy * CHUNK_SECTION_SIZE + y;
	const int wz = cz * CHUNK_SIZE_Z + z;

	return world.GetBlock(wx, wy, wz).blockID == 0;
}

static bool IsFaceOccluded(const CChunkWorld& world, int wx, int wy, int wz, uint8_t cullDir)
{
	switch (cullDir)
	{
		case static_cast<uint8_t>(FACE_DIR::PX): ++wx; break;
		case static_cast<uint8_t>(FACE_DIR::NX): --wx; break;
		case static_cast<uint8_t>(FACE_DIR::PY): ++wy; break;
		case static_cast<uint8_t>(FACE_DIR::NY): --wy; break;
		case static_cast<uint8_t>(FACE_DIR::PZ): ++wz; break;
		case static_cast<uint8_t>(FACE_DIR::NZ): --wz; break;
		default: return false;
	}

	return world.GetBlock(wx, wy, wz).blockID != 0;
}