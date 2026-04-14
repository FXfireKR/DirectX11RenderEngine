#include "pch.h"
#include "ModelUtil.h"

/*
	FACE_DIR mapping
	PX = east, NX = west
	PY = up,   NY = down
	PZ = south, NZ = north

	Quad vertex order:
	[0] LB, [1] RB, [2] RT, [3] LT

	Default winding assumes front-face when emitted as:
	0,1,2 / 0,2,3
*/

DirectX::XMFLOAT3 FaceToNormalFloat3(FACE_DIR dir)
{
	switch (dir)
	{
		case FACE_DIR::PX: return { 1.f, 0.f, 0.f };
		case FACE_DIR::NX: return { -1.f, 0.f, 0.f };
		case FACE_DIR::PY: return { 0.f, 1.f, 0.f };
		case FACE_DIR::NY: return { 0.f, -1.f, 0.f };
		case FACE_DIR::PZ: return { 0.f, 0.f, 1.f };
		case FACE_DIR::NZ: return { 0.f, 0.f, -1.f };
		default: return { 0.f, 0.f, 0.f };
	}
}

DirectX::XMINT3 FaceToNormalInt3(FACE_DIR dir)
{
	switch (dir)
	{
		case FACE_DIR::PX: return { 1, 0, 0 };
		case FACE_DIR::NX: return { -1, 0, 0 };
		case FACE_DIR::PY: return { 0, 1, 0 };
		case FACE_DIR::NY: return { 0, -1, 0 };
		case FACE_DIR::PZ: return { 0, 0, 1 };
		case FACE_DIR::NZ: return { 0, 0, -1 };
		default: return { 0, 0, 0 };
	}
}

FACE_DIR NormalToFaceDir(const DirectX::XMINT3& normal)
{
	if (normal.x == 1)	return FACE_DIR::PX;
	if (normal.x == -1) return FACE_DIR::NX;
	if (normal.y == 1)	return FACE_DIR::PY;
	if (normal.y == -1) return FACE_DIR::NY;
	if (normal.z == 1)	return FACE_DIR::PZ;
	if (normal.z == -1)	return FACE_DIR::NZ;
	return FACE_DIR::COUNT;
}

bool TryParseFaceDir(const char* strDir, FACE_DIR& outDir)
{
	if (!strDir) return false;

	if (strcmp(strDir, "east") == 0)	{ outDir = FACE_DIR::PX; return true; }
	if (strcmp(strDir, "west") == 0)	{ outDir = FACE_DIR::NX; return true; }
	if (strcmp(strDir, "up") == 0)		{ outDir = FACE_DIR::PY; return true; }
	if (strcmp(strDir, "down") == 0)	{ outDir = FACE_DIR::NY; return true; }
	if (strcmp(strDir, "south") == 0)	{ outDir = FACE_DIR::PZ; return true; }
	if (strcmp(strDir, "north") == 0)	{ outDir = FACE_DIR::NZ; return true; }

	return false;
}

bool TryParseAxis(const char* strAxis, ROT_AXIS& outAxis)
{
	if (!strAxis) return false;

	if (strcmp(strAxis, "x") == 0) { outAxis = ROT_AXIS::X; return true; }
	if (strcmp(strAxis, "y") == 0) { outAxis = ROT_AXIS::Y; return true; }
	if (strcmp(strAxis, "z") == 0) { outAxis = ROT_AXIS::Z; return true; }

	return false;
}

float MCCoordToUnit(float v)
{
	return v / 16.0f;
}

void BuildFaceQuadPositions01(const ModelElement& elem, FACE_DIR dir, DirectX::XMFLOAT3 outPos[4])
{
	const DirectX::XMFLOAT3 from =
	{
		MCCoordToUnit(elem.from[0]),
		MCCoordToUnit(elem.from[1]),
		MCCoordToUnit(elem.from[2])
	};

	const DirectX::XMFLOAT3 to =
	{
		MCCoordToUnit(elem.to[0]),
		MCCoordToUnit(elem.to[1]),
		MCCoordToUnit(elem.to[2])
	};

	switch (dir)
	{
		case FACE_DIR::PX: // EAST
		{
			outPos[0] = { to.x, from.y, to.z };
			outPos[1] = { to.x, from.y, from.z };
			outPos[2] = { to.x, to.y, from.z };
			outPos[3] = { to.x, to.y, to.z };
		} break;
		case FACE_DIR::NX: // WEST
		{
			outPos[0] = { from.x, from.y, from.z };
			outPos[1] = { from.x, from.y, to.z };
			outPos[2] = { from.x, to.y, to.z };
			outPos[3] = { from.x, to.y, from.z };
		} break;
		case FACE_DIR::PY: // UP
		{
			outPos[0] = { from.x, to.y, to.z };
			outPos[1] = { to.x, to.y, to.z };
			outPos[2] = { to.x, to.y, from.z };
			outPos[3] = { from.x, to.y, from.z };
		} break;
		case FACE_DIR::NY: // DOWN
		{
			outPos[0] = { from.x, from.y, from.z };
			outPos[1] = { to.x, from.y, from.z };
			outPos[2] = { to.x, from.y, to.z };
			outPos[3] = { from.x, from.y, to.z };
		} break;
		case FACE_DIR::PZ: // SOUTH
		{
			outPos[0] = { from.x, from.y, to.z };
			outPos[1] = { to.x, from.y, to.z };
			outPos[2] = { to.x, to.y, to.z };
			outPos[3] = { from.x, to.y, to.z };
		} break;
		case FACE_DIR::NZ: // NORTH
		{
			outPos[0] = { to.x, from.y, from.z };
			outPos[1] = { from.x, from.y, from.z };
			outPos[2] = { from.x, to.y, from.z };
			outPos[3] = { to.x, to.y, from.z };
		} break;

		default:
		{
			assert(false);
		} break;
	}
}

void ComputeFaceUVDefault(const ModelElement& elem, FACE_DIR dir, float outUV[4])
{
	const DirectX::XMFLOAT3 from =
	{
		MCCoordToUnit(elem.from[0]),
		MCCoordToUnit(elem.from[1]),
		MCCoordToUnit(elem.from[2])
	};

	const DirectX::XMFLOAT3 to =
	{
		MCCoordToUnit(elem.to[0]),
		MCCoordToUnit(elem.to[1]),
		MCCoordToUnit(elem.to[2])
	};

	float u0 = 0.f, v0 = 0.f, u1 = 1.f, v1 = 1.f;

	switch (dir)
	{
		case FACE_DIR::PX: // east: u=z, v=y
		case FACE_DIR::NX: // west: u=z, v=y
		{
			u0 = from.z; 
			v0 = 1.0f - to.y; 
			u1 = to.z;
			v1 = 1.0f - from.y;
		} break;

		// up/down: V축이 z로 간다. top-left 기준으로 “북쪽(-Z)이 위”가 되게 맞춘다.
		case FACE_DIR::PY: // up: u=x, v=z  (from.z이 위쪽(v0))
		{
			u0 = from.x; 
			v0 = from.z; 
			u1 = to.x;
			v1 = to.z;
		} break;

		case FACE_DIR::NY: // down: u=x, v=z  (down은 “위쪽”이 반대로 느껴지기 쉬운데 positions에서 V축을 -Z로 잡았으므로 여기서는 동일 처리)
		{
			u0 = from.x; 
			v0 = from.z; 
			u1 = to.x;
			v1 = to.z;
		} break;

		// side faces: U는 수평축, V는 Y(위아래)인데 텍스처는 top-left 기준이라 Y를 뒤집어야 “위쪽(to.y)”이 “v0(0)”로 감
		case FACE_DIR::PZ: // south: u=x, v=y
		case FACE_DIR::NZ: // north: u=x, v=y (방향성은 positions에서 해결)
		{
			u0 = from.x; 
			v0 = 1.0f - to.y; 
			u1 = to.x;
			v1 = 1.0f - from.y;
		} break;

		default: break;
	}

	outUV[0] = u0;
	outUV[1] = v0;
	outUV[2] = u1;
	outUV[3] = v1;
}

void ApplyUVRotation(DirectX::XMFLOAT2 uv[4], uint8_t rotDeg)
{
	const uint8_t rot = static_cast<uint8_t>(rotDeg % 360);
	if (rot == 0) return;

	DirectX::XMFLOAT2 temp[4];

	// uv order: [0]=LB, [1]=RB, [2]=RT, [3]=LT
	// CW 90: LB<-LT, RB<-LB, RT<-RB, LT<-RT
	switch (rot)
	{
		case 90:
		{
			temp[0] = uv[3]; // LB <- LT
			temp[1] = uv[0]; // RB <- LB
			temp[2] = uv[1]; // RT <- RB
			temp[3] = uv[2]; // LT <- RT
		} break;

		case 180:
		{
			temp[0] = uv[2];
			temp[1] = uv[3];
			temp[2] = uv[0];
			temp[3] = uv[1];
		} break;

		case 270:
		{
			temp[0] = uv[1];
			temp[1] = uv[2];
			temp[2] = uv[3];
			temp[3] = uv[0];
		} break;

		default: return;
	}

	memcpy(uv, temp, sizeof(temp));
}