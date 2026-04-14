#pragma once
#include "AtlasTypes.h"
#include "CRuntimeAtlas.h"

class CRuntimeAtlasBuilder
{
public:
	CRuntimeAtlasBuilder();
	~CRuntimeAtlasBuilder() = default;

	void Clear();

	void SetPadding(uint32_t padding);
	void SetDefaultTileSize(uint32_t tileSize);

	inline const uint32_t GetPadding() const { return m_uPadding; }
	inline const uint32_t GetDefaultTileSize() const { return m_uDefaultTileSize; }

public:
	bool AddInput(const AtlasBuildInput& input);
	bool AddInput(const char* textureKey, const char* filePath);
	bool AddInputs(const vector<AtlasBuildInput>& inputs);

	bool Build(ID3D11Device* pDevice, CRuntimeAtlas& outAtlas);
	
private:
	bool _CollectUniqueInputs(vector<AtlasBuildInput>& outInputs) const;
	bool _LoadSourceImages(const vector<AtlasBuildInput>& inputs, vector<AtlasSourceImage>& outImages) const;
	bool _BuildLayout(const vector<AtlasSourceImage>& images, AtlasLayoutResult& outLayout) const;
	bool _BakeAtlasPixels(const vector<AtlasSourceImage>& images, const AtlasLayoutResult& layout, vector<uint8_t>& outPixels
		, uint32_t& outAtlasWidth, uint32_t& outAtlasHeight, unordered_map<string, AtlasRegion>& outRegions) const;

	bool _CreateAtlasResource(ID3D11Device* pDevice, const vector<uint8_t>& pixels
		, uint32_t atlasWidth, uint32_t atlasHeight, unordered_map<string, AtlasRegion>&& regions, CRuntimeAtlas& outAtlas) const;

	bool _LoadImageRGBA8(const char* filePath, vector<uint8_t>& outPixels, uint32_t& outWidth, uint32_t& outHeight) const;
	
	static void _CopyPixelRGBA(const vector<uint8_t>& src, uint32_t srcWidth, uint32_t sx, uint32_t sy,
		vector<uint8_t>& dst, uint32_t dstWidth, uint32_t dx, uint32_t dy);

private:
	vector<AtlasBuildInput> m_vecBuildItems;

	uint32_t m_uPadding = 1;
	uint32_t m_uDefaultTileSize = 16;
};