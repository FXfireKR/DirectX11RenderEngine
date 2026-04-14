#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <DirectXMath.h>

struct ModelFace
{
	std::string textureRef;	// "#all", "minecraft:block/stone"
	bool bHasUV = false;
	float uv[4] = { 0.f, 0.f, 0.f, 0.f };

	bool bHasCullFace = false;
	uint8_t cullFaceDir = 0;

	uint8_t rotation = 0; // 0, 90, 180, 270
	int tintIndex = -1;
};

struct ModelElement
{
	// Minecraft model unit : 0 ~ 16
	float from[3] = { 0.f, 0.f, 0.f };
	float to[3] = { 16.f, 16.f, 16.f };

	bool bHasRotation = false;
	float rotOrigin[3] = { 8.f, 8.f, 8.f };
	uint8_t rotAxis = 0;
	float rotAngleDeg = 0.f;
	bool bRescale = false;

	bool bHasFace[6] = { false, false, false, false, false, false };
	ModelFace faces[6]{};
};

struct ModelRaw
{
	bool bHasParent = false;
	std::string parentKey; // "minecraft:block/cube_all"

	std::unordered_map<std::string, std::string> textures;
	std::vector<ModelElement> elements;
};

struct ModelResolved
{
	std::unordered_map<std::string, std::string> textures;
	std::vector<ModelElement> elements;
};

struct BakedVertex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	uint32_t color = 0xFFFFFFFF; // tint color
};

struct BakedQuad
{
	uint8_t dir = 0;
	bool bHasCullFace = false;
	uint8_t cullFaceDir = 0;
	int tintIndex = -1;
	uint64_t textureHash = 0;
//#ifdef _DEBUG
	std::string debugTextureKey;
//#endif // _DEBUG
	BakedVertex verts[4];
};

struct BakedModel
{
	std::vector<BakedQuad> quads;
};

struct CPUMeshData
{
	std::vector<BakedVertex> vertices;
	std::vector<uint32_t> indices;
};