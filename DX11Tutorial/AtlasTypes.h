#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct AtlasRegion
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint32_t width = 0;
	uint32_t height = 0;

	float u0 = 0.f;
	float v0 = 0.f;
	float u1 = 0.f;
	float v1 = 0.f;
};

struct AtlasBuildInput
{
	std::string textureKey;
	std::string filePath;
};

struct AtlasSourceImage
{
	std::string textureKey;
	std::string filePath;

	uint32_t width = 0;
	uint32_t height = 0;

	std::vector<uint8_t> pixels; // RGBA8
};

struct AtlasPlacedImage
{
	std::string textureKey;

	uint32_t srcWidth = 0;
	uint32_t srcHeight = 0;

	uint32_t dstX = 0;
	uint32_t dstY = 0;
};

struct AtlasLayoutResult
{
	uint32_t atlasWidth = 0;
	uint32_t atlasHeight = 0;

	std::vector<AtlasPlacedImage> placedImages;
};

/*
struct UVRect
{
	float u0, v0, u1, v1;
};


struct RuntimeAtlasDesc
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t tilePx = 0;

	DXGI_FORMAT eFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	bool bMipmap = false;
	bool bSRGB = false;
};
*/