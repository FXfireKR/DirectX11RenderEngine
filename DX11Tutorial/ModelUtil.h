#pragma once
#include "ModelTypes.h"

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

enum class FACE_DIR : uint8_t
{
	PX = 0, // east
	NX,		// west
	PY,		// up
	NY,		// down
	PZ,		// south
	NZ,		// north
	COUNT
};

enum class ROT_AXIS : uint8_t
{
	X = 0,
	Y,
	Z
};

DirectX::XMFLOAT3 FaceToNormalFloat3(FACE_DIR dir);
DirectX::XMINT3 FaceToNormalInt3(FACE_DIR dir);
FACE_DIR NormalToFaceDir(const DirectX::XMINT3& normal);
bool TryParseFaceDir(const char* strDir, FACE_DIR& outDir);
bool TryParseAxis(const char* strAxis, ROT_AXIS& outAxis);
float MCCoordToUnit(float v);
void BuildFaceQuadPositions01(const ModelElement& elem, FACE_DIR dir, DirectX::XMFLOAT3 outPos[4]);
void ComputeFaceUVDefault(const ModelElement& elem, FACE_DIR dir, float outUV[4]);
void ApplyUVRotation(DirectX::XMFLOAT2 uv[4], uint8_t rotDeg);