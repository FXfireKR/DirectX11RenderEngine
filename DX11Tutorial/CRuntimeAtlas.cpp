#include "pch.h"
#include "CRuntimeAtlas.h"

void CRuntimeAtlas::Clear()
{
	m_pShaderResourceView.Reset();
}

bool CRuntimeAtlas::Initialize(const ComPtr<ID3D11ShaderResourceView>& pShaderResourceView, uint32_t atlasWidth, uint32_t atlasHeight
	, unordered_map<string, AtlasRegion>&& mapRegions)
{
	Clear();

	if (nullptr == pShaderResourceView)
		return false;

	if (atlasWidth == 0 || atlasHeight == 0)
		return false;

	m_pShaderResourceView = pShaderResourceView;

	m_uAtlasWidth = atlasWidth;
	m_uAtlasHeight = atlasHeight;
	
	m_mapRegions = std::move(mapRegions);

	return true;
}

const AtlasRegion* CRuntimeAtlas::FindRegion(const char* textureKey) const
{
	if (nullptr == textureKey || textureKey[0] == '\0')
		return nullptr;

	auto it = m_mapRegions.find(textureKey);
	if (it == m_mapRegions.end())
		return nullptr;

	return &it->second;
}

const bool CRuntimeAtlas::TryGetRegion(const char* textureKey, AtlasRegion& outRegion) const
{
	const AtlasRegion* pRegion = FindRegion(textureKey);
	if (nullptr == pRegion)
		return false;

	outRegion = *pRegion;
	return true;
}

//static DXGI_FORMAT MakeSRGBFormatIfNeeded(DXGI_FORMAT eFormat, bool bSRGB)
//{
//	if (bSRGB) {
//		if (DXGI_FORMAT_R8G8B8A8_UNORM == eFormat) return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//		else if (DXGI_FORMAT_B8G8R8A8_UNORM == eFormat) return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
//	}
//	return eFormat;
//}
//bool CRuntimeAtlas::Create(ID3D11Device* pDevice, const RuntimeAtlasDesc& desc)
//{
//	if (desc.width == 0 || desc.height == 0 || desc.tilePx == 0) return false;
//	if (desc.width % desc.tilePx != 0) return false;
//	if (desc.height % desc.tilePx != 0) return false;
//
//	m_desc = desc;
//	m_tilesX = desc.width / desc.tilePx;
//	m_tilesY = desc.height / desc.tilePx;
//
//	m_mapTile.clear();
//	m_nextSlot = 0;
//
//	m_cpu.assign((size_t)desc.width * (size_t)desc.height * 4ull, 0);
//
//	const DXGI_FORMAT eFormat = MakeSRGBFormatIfNeeded(desc.eFormat, desc.bSRGB);
//
//	D3D11_TEXTURE2D_DESC textureDesc{};
//	textureDesc.Width = desc.width;
//	textureDesc.Height = desc.height;
//	textureDesc.MipLevels = desc.bMipmap ? 0 : 1;
//	textureDesc.ArraySize = 1;
//	textureDesc.Format = eFormat;
//	textureDesc.SampleDesc.Count = 1;
//	textureDesc.Usage = D3D11_USAGE_DEFAULT;
//
//	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//	if (desc.bMipmap) textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
//
//	textureDesc.MiscFlags = 0;
//	if (desc.bMipmap) textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
//
//	HRESULT hr = pDevice->CreateTexture2D(&textureDesc, nullptr, m_pTexture.GetAddressOf());
//	if (FAILED(hr)) return false;
//
//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
//	srvDesc.Format = textureDesc.Format;
//	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	srvDesc.Texture2D.MostDetailedMip = 0;
//	srvDesc.Texture2D.MipLevels = desc.bMipmap ? -1 : 1;
//
//	hr = pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, m_pShaderResourceView.GetAddressOf());
//	if (FAILED(hr)) return false;
//
//	return true;
//}
//
//bool CRuntimeAtlas::AddTileFromFile(ID3D11DeviceContext* pContext, uint64_t tileKey, const char* pathUTF8)
//{
//	if (!m_pTexture || !m_pShaderResourceView) return false;
//	if (HasTile(tileKey))
//		return true;
//
//	uint16_t tileId = 0;
//	if (!_AllocateSlot(tileId))
//		return false;
//
//	// 파일 -> RGBA32 (tilePx x tilePx)
//	std::vector<uint8_t> rgba;
//	uint32_t w = 0, h = 0;
//	if (!_LoadImageToRGBA32(pathUTF8, rgba, w, h))
//		return false;
//
//	if (w != m_desc.tilePx || h != m_desc.tilePx)
//		return false;
//
//	const uint32_t tx = tileId % m_tilesX;
//	const uint32_t ty = tileId / m_tilesX;
//
//	const uint32_t dstX = tx * m_desc.tilePx;
//	const uint32_t dstY = ty * m_desc.tilePx;
//
//	// CPU atlas buffer에도 복사(선택이지만 추천)
//	const uint32_t dstPitch = m_desc.width * 4;
//	const uint32_t srcPitch = m_desc.tilePx * 4;
//
//	for (uint32_t row = 0; row < m_desc.tilePx; ++row)
//	{
//		uint8_t* dst = m_cpu.data() + (size_t)(dstY + row) * dstPitch + (size_t)dstX * 4;
//		const uint8_t* src = rgba.data() + (size_t)row * srcPitch;
//		std::memcpy(dst, src, srcPitch);
//	}
//
//	// GPU 부분 업데이트
//	D3D11_BOX box{};
//	box.left = dstX;
//	box.top = dstY;
//	box.front = 0;
//	box.right = dstX + m_desc.tilePx;
//	box.bottom = dstY + m_desc.tilePx;
//	box.back = 1;
//
//	pContext->UpdateSubresource(m_pTexture.Get(), 0, &box, rgba.data(), srcPitch, 0);
//
//	// mipmap 사용 시: 변경 후 mip 갱신
//	if (m_desc.bMipmap) {
//		pContext->GenerateMips(m_pShaderResourceView.Get());
//	}
//
//	m_mapTile.emplace(tileKey, tileId);
//	return true;
//}
//
//bool CRuntimeAtlas::HasTile(uint64_t tileKey) const
//{
//	return (m_mapTile.find(tileKey) != m_mapTile.end());
//}
//
//uint16_t CRuntimeAtlas::GetTileID(uint64_t tileKey) const
//{
//	auto it = m_mapTile.find(tileKey);
//	if (it == m_mapTile.end()) return UINT16_MAX;
//	return it->second;
//}
//
//UVRect CRuntimeAtlas::GetUV(uint16_t tileID, float padPx) const
//{
//	const uint32_t tx = tileID % m_tilesX;
//	const uint32_t ty = tileID / m_tilesX;
//
//	const float invW = 1.0f / float(m_desc.width);
//	const float invH = 1.0f / float(m_desc.height);
//
//	const float x0 = float(tx * m_desc.tilePx) + padPx;
//	const float y0 = float(ty * m_desc.tilePx) + padPx;
//	const float x1 = float((tx + 1) * m_desc.tilePx) - padPx;
//	const float y1 = float((ty + 1) * m_desc.tilePx) - padPx;
//
//	return { x0 * invW, y0 * invH, x1 * invW, y1 * invH };
//}
//
//static bool HasExtensionLower(const std::string& path, const char* extLower)
//{
//	std::filesystem::path p(path);
//	if (!p.has_extension()) return false;
//	std::string e = p.extension().string();
//	std::transform(e.begin(), e.end(), e.begin(), ::tolower);
//	return (e == extLower);
//}
//
//bool CRuntimeAtlas::_AllocateSlot(uint16_t& outTileID)
//{
//	const uint32_t maxSlots = m_tilesX * m_tilesY;
//	if (m_nextSlot >= maxSlots) return false;
//	outTileID = m_nextSlot++;
//	return true;
//}
//
//bool CRuntimeAtlas::_DecodeRGBA32_WIC(const wchar_t* pathW, vector<uint8_t>& outRGBA, uint32_t& outW, uint32_t& outH)
//{
//	outRGBA.clear();
//	outW = outH = 0;
//
//	// COM init은 엔진에서 이미 했을 수도 있음.
//	// 안전하게 한번 호출: 이미 초기화되어 있으면 S_FALSE 반환 가능.
//	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
//
//	ComPtr<IWICImagingFactory> factory;
//	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
//		IID_PPV_ARGS(factory.GetAddressOf()));
//	if (FAILED(hr)) return false;
//
//	ComPtr<IWICBitmapDecoder> decoder;
//	hr = factory->CreateDecoderFromFilename(pathW, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
//	if (FAILED(hr)) return false;
//
//	ComPtr<IWICBitmapFrameDecode> frame;
//	hr = decoder->GetFrame(0, frame.GetAddressOf());
//	if (FAILED(hr)) return false;
//
//	UINT w = 0, h = 0;
//	hr = frame->GetSize(&w, &h);
//	if (FAILED(hr) || w == 0 || h == 0) return false;
//
//	ComPtr<IWICFormatConverter> conv;
//	hr = factory->CreateFormatConverter(conv.GetAddressOf());
//	if (FAILED(hr)) return false;
//
//	hr = conv->Initialize(
//		frame.Get(),
//		GUID_WICPixelFormat32bppRGBA,   // RGBA32 강제
//		WICBitmapDitherTypeNone,
//		nullptr,
//		0.0,
//		WICBitmapPaletteTypeCustom);
//	if (FAILED(hr)) return false;
//
//	const uint32_t stride = (uint32_t)w * 4u;
//	const uint32_t size = stride * (uint32_t)h;
//
//	outRGBA.resize(size);
//	hr = conv->CopyPixels(nullptr, stride, size, outRGBA.data());
//	if (FAILED(hr)) return false;
//
//	outW = (uint32_t)w;
//	outH = (uint32_t)h;
//	return true;
//}
//
//bool CRuntimeAtlas::_LoadImageToRGBA32(const char* pathUTF8, vector<uint8_t>& outRGBA, uint32_t& outW, uint32_t& outH)
//{
//	std::wstring wpath = UTF8ToWstring(pathUTF8);
//	return _DecodeRGBA32_WIC(wpath.c_str(), outRGBA, outW, outH);
//}