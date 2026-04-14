#pragma once
#include "AtlasTypes.h"

class CRuntimeAtlas
{
public:
	CRuntimeAtlas() = default;
	~CRuntimeAtlas() = default;

	void Clear();
	bool Initialize(const ComPtr<ID3D11ShaderResourceView>& pShaderResourceView
		, uint32_t atlasWidth, uint32_t atlasHeight, unordered_map<string, AtlasRegion>&& mapRegions);

	const AtlasRegion* FindRegion(const char* textureKey) const;
	const bool TryGetRegion(const char* textureKey, AtlasRegion& outRegion) const;

public:
	inline const bool IsValid() const { return m_pShaderResourceView != nullptr; }

	inline ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView.Get(); }

	inline const uint32_t GetWidth() const { return m_uAtlasWidth; }
	inline const uint32_t GetHeight() const { return m_uAtlasHeight; }


private:
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
	unordered_map<string, AtlasRegion> m_mapRegions;

	uint32_t m_uAtlasWidth = 0;
	uint32_t m_uAtlasHeight = 0;
};


/*
	bool Create(ID3D11Device* pDevice, const RuntimeAtlasDesc& desc);
	bool AddTileFromFile(ID3D11DeviceContext* pContext, uint64_t tileKey, const char* pathUTF8);

	bool HasTile(uint64_t tileKey) const;
	uint16_t GetTileID(uint64_t tileKey) const;
	UVRect GetUV(uint16_t tileID, float padPx = 0.5f) const;

	inline ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pShaderResourceView.Get(); }
	inline const RuntimeAtlasDesc& GetDesc() const { return m_desc; }

private:
	bool _AllocateSlot(uint16_t& outTileID);
	bool _DecodeRGBA32_WIC(const wchar_t* pathW, vector<uint8_t>& outRGBA, uint32_t& outW, uint32_t& outH);
	bool _LoadImageToRGBA32(const char* pathUTF8, vector<uint8_t>& outRGBA, uint32_t& outW, uint32_t& outH);

private:
	RuntimeAtlasDesc m_desc{};
	uint32_t m_tilesX = 0;
	uint32_t m_tilesY = 0;
	
	vector<uint8_t> m_cpu;

	ComPtr<ID3D11Texture2D> m_pTexture;
	ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;

	uint16_t m_nextSlot = 0;
	unordered_map<uint64_t, uint16_t> m_mapTile;
*/