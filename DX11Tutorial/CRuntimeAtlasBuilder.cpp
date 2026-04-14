#include "pch.h"
#include "CRuntimeAtlasBuilder.h"

namespace
{
    static uint32_t CeilSqrtU32(uint32_t value)
    {
        if (value == 0) return 0;

        const float root = std::sqrt(static_cast<float>(value));
        return static_cast<uint32_t>(std::ceil(root));
    }
}

CRuntimeAtlasBuilder::CRuntimeAtlasBuilder()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

void CRuntimeAtlasBuilder::Clear()
{
    m_vecBuildItems.clear();
}

void CRuntimeAtlasBuilder::SetPadding(uint32_t padding)
{
    m_uPadding = padding;
}

void CRuntimeAtlasBuilder::SetDefaultTileSize(uint32_t tileSize)
{
    if (tileSize == 0)
        return;

    m_uDefaultTileSize = tileSize;
}

bool CRuntimeAtlasBuilder::AddInput(const AtlasBuildInput& input)
{
    if (input.textureKey.empty())
        return false;

    if (input.filePath.empty())
        return false;

    m_vecBuildItems.push_back(input);
    return true;
}

bool CRuntimeAtlasBuilder::AddInput(const char* textureKey, const char* filePath)
{
    if (nullptr == textureKey || nullptr == filePath)
        return false;

    AtlasBuildInput input;
    input.textureKey = textureKey;
    input.filePath = filePath;
    
    return AddInput(input);
}

bool CRuntimeAtlasBuilder::AddInputs(const vector<AtlasBuildInput>& inputs)
{
    bool bAllSuccess = true;

    for (const AtlasBuildInput& input : inputs)
    {
        if (!AddInput(input)) 
        {
            bAllSuccess = false;
        }
    }

    return bAllSuccess;
}

bool CRuntimeAtlasBuilder::Build(ID3D11Device* pDevice, CRuntimeAtlas& outAtlas)
{
    outAtlas.Clear();

    vector<AtlasBuildInput> uniqueInputs;
    if (!_CollectUniqueInputs(uniqueInputs))
        return false;

    if (uniqueInputs.empty())
        return false;

    vector<AtlasSourceImage> images;
    if (!_LoadSourceImages(uniqueInputs, images))
        return false;

    if (images.empty())
        return false;

    AtlasLayoutResult layout;
    if (!_BuildLayout(images, layout))
        return false;

    vector<uint8_t> atlasPixels;
    uint32_t atlasWidth = 0;
    uint32_t atlasHeight = 0;
    unordered_map<string, AtlasRegion> regions;

    if (!_BakeAtlasPixels(images, layout, atlasPixels, atlasWidth, atlasHeight, regions))
        return false;

    if (!_CreateAtlasResource(pDevice, atlasPixels, atlasWidth, atlasHeight, std::move(regions), outAtlas))
        return false;

    return true;
}

bool CRuntimeAtlasBuilder::_CollectUniqueInputs(vector<AtlasBuildInput>& outInputs) const
{
    outInputs.clear();

    if (m_vecBuildItems.empty())
        return false;

    unordered_map<string, string> mapUnique;
    for (const AtlasBuildInput& input : m_vecBuildItems)
    {
        if (input.textureKey.empty() || input.filePath.empty()) continue;

        auto it = mapUnique.find(input.textureKey);
        if (it == mapUnique.end())
        {
            mapUnique.emplace(input.textureKey, input.filePath);
        }
        else
        {
            // TODO: logging
        }
    }

    outInputs.reserve(mapUnique.size());

    for (const auto& pair : mapUnique)
    {
        AtlasBuildInput input;
        input.textureKey = pair.first;
        input.filePath = pair.second;
        outInputs.push_back(std::move(input));
    }

    std::sort(outInputs.begin(), outInputs.end(), [](const AtlasBuildInput& a, const AtlasBuildInput& b) 
        {
            return a.textureKey < b.textureKey;
        });

    return !outInputs.empty();
}

bool CRuntimeAtlasBuilder::_LoadSourceImages(const vector<AtlasBuildInput>& inputs, vector<AtlasSourceImage>& outImages) const
{
    outImages.clear();

    if (inputs.empty())
        return false;

    outImages.reserve(inputs.size());

    for (const AtlasBuildInput& input : inputs)
    {
        AtlasSourceImage image;
        image.textureKey = input.textureKey;
        image.filePath = input.filePath;

        if (!_LoadImageRGBA8(input.filePath.c_str(), image.pixels, image.width, image.height))
            return false;

        if (image.width == 0 || image.height == 0 || image.pixels.empty())
            return false;

        outImages.push_back(std::move(image));
    }

    return !outImages.empty();
}

bool CRuntimeAtlasBuilder::_BuildLayout(const vector<AtlasSourceImage>& images, AtlasLayoutResult& outLayout) const
{
    outLayout = {};
    if (images.empty())
        return false;

    uint32_t maxImageWidth = m_uDefaultTileSize;
    uint32_t maxImageHeight = m_uDefaultTileSize;

    for (const AtlasSourceImage& image : images)
    {
        maxImageWidth = std::max(maxImageWidth, image.width);
        maxImageHeight = std::max(maxImageHeight, image.height);
    }

    const uint32_t cellWidth = maxImageWidth + (m_uPadding * 2);
    const uint32_t cellHeight = maxImageHeight + (m_uPadding * 2);

    const uint32_t imageCount = static_cast<uint32_t>(images.size());
    const uint32_t cols = CeilSqrtU32(imageCount);
    const uint32_t rows = (imageCount + cols - 1) / cols;

    outLayout.atlasWidth = cols * cellWidth;
    outLayout.atlasHeight = rows * cellHeight;
    outLayout.placedImages.reserve(images.size());

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        const AtlasSourceImage& image = images[i];

        const uint32_t col = i % cols;
        const uint32_t row = i / cols;

        const uint32_t cellOriginX = col * cellWidth;
        const uint32_t cellOriginY = row * cellHeight;

        AtlasPlacedImage placed;
        placed.textureKey = image.textureKey;
        placed.srcWidth = image.width;
        placed.srcHeight = image.height;

        // 좌상단 기준
        placed.dstX = cellOriginX + m_uPadding;
        placed.dstY = cellOriginY + m_uPadding;

        outLayout.placedImages.push_back(std::move(placed));
    }

    return true;
}

bool CRuntimeAtlasBuilder::_BakeAtlasPixels(const vector<AtlasSourceImage>& images, const AtlasLayoutResult& layout, vector<uint8_t>& outPixels
    , uint32_t& outAtlasWidth, uint32_t& outAtlasHeight, unordered_map<string, AtlasRegion>& outRegions) const
{
    outPixels.clear();
    outRegions.clear();
    outAtlasWidth = 0;
    outAtlasHeight = 0;

    if (images.empty())
        return false;

    if (images.size() != layout.placedImages.size())
        return false;

    if (layout.atlasWidth == 0 || layout.atlasHeight == 0)
        return false;

    outAtlasWidth = layout.atlasWidth;
    outAtlasHeight = layout.atlasHeight;

    outPixels.resize(static_cast<size_t>(outAtlasWidth) * outAtlasHeight * 4, 0);
    outRegions.reserve(images.size());

    unordered_map<string, const AtlasSourceImage*> mapImageByKey;
    mapImageByKey.reserve(images.size());

    for (const AtlasSourceImage& image : images)
    {
        mapImageByKey.emplace(image.textureKey, &image);
    }

    for (const AtlasPlacedImage& placed : layout.placedImages)
    {
        auto it = mapImageByKey.find(placed.textureKey);
        if (it == mapImageByKey.end())
            return false;

        const AtlasSourceImage& image = *it->second;

        if (image.width != placed.srcWidth || image.height != placed.srcHeight)
            return false;

        // 1. 본 픽셀 복사
        for (uint32_t y = 0; y < image.height; ++y)
        {
            for (uint32_t x = 0; x < image.width; ++x)
            {
                _CopyPixelRGBA(
                    image.pixels,
                    image.width,
                    x,
                    y,
                    outPixels,
                    outAtlasWidth,
                    placed.dstX + x,
                    placed.dstY + y);
            }
        }

        // 2. 좌우 padding bleed
        for (uint32_t y = 0; y < image.height; ++y)
        {
            for (uint32_t p = 1; p <= m_uPadding; ++p)
            {
                if (placed.dstX >= p)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX,
                        placed.dstY + y,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX - p,
                        placed.dstY + y);
                }

                if ((placed.dstX + image.width - 1 + p) < outAtlasWidth)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + image.width - 1,
                        placed.dstY + y,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + image.width - 1 + p,
                        placed.dstY + y);
                }
            }
        }

        // 3. 상하 padding bleed
        for (uint32_t x = 0; x < image.width; ++x)
        {
            for (uint32_t p = 1; p <= m_uPadding; ++p)
            {
                if (placed.dstY >= p)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + x,
                        placed.dstY,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + x,
                        placed.dstY - p);
                }

                if ((placed.dstY + image.height - 1 + p) < outAtlasHeight)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + x,
                        placed.dstY + image.height - 1,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + x,
                        placed.dstY + image.height - 1 + p);
                }
            }
        }

        // 4. 코너 padding bleed
        for (uint32_t py = 1; py <= m_uPadding; ++py)
        {
            for (uint32_t px = 1; px <= m_uPadding; ++px)
            {
                // left-top
                if (placed.dstX >= px && placed.dstY >= py)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX,
                        placed.dstY,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX - px,
                        placed.dstY - py);
                }

                // right-top
                if ((placed.dstX + image.width - 1 + px) < outAtlasWidth && placed.dstY >= py)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + image.width - 1,
                        placed.dstY,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + image.width - 1 + px,
                        placed.dstY - py);
                }

                // left-bottom
                if (placed.dstX >= px && (placed.dstY + image.height - 1 + py) < outAtlasHeight)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX,
                        placed.dstY + image.height - 1,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX - px,
                        placed.dstY + image.height - 1 + py);
                }

                // right-bottom
                if ((placed.dstX + image.width - 1 + px) < outAtlasWidth &&
                    (placed.dstY + image.height - 1 + py) < outAtlasHeight)
                {
                    _CopyPixelRGBA(
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + image.width - 1,
                        placed.dstY + image.height - 1,
                        outPixels,
                        outAtlasWidth,
                        placed.dstX + image.width - 1 + px,
                        placed.dstY + image.height - 1 + py);
                }
            }
        }

        AtlasRegion region;
        region.x = placed.dstX;
        region.y = placed.dstY;
        region.width = image.width;
        region.height = image.height;

        region.u0 = static_cast<float>(region.x) / static_cast<float>(outAtlasWidth);
        region.v0 = static_cast<float>(region.y) / static_cast<float>(outAtlasHeight);
        region.u1 = static_cast<float>(region.x + region.width) / static_cast<float>(outAtlasWidth);
        region.v1 = static_cast<float>(region.y + region.height) / static_cast<float>(outAtlasHeight);

        outRegions.emplace(placed.textureKey, region);
    }

    return !outRegions.empty();
}

bool CRuntimeAtlasBuilder::_CreateAtlasResource(ID3D11Device* pDevice, const vector<uint8_t>& pixels, uint32_t atlasWidth, uint32_t atlasHeight
    , unordered_map<string, AtlasRegion>&& regions, CRuntimeAtlas& outAtlas) const
{
    if (pixels.empty())
        return false;

    if (atlasWidth == 0 || atlasHeight == 0)
        return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = atlasWidth;
    desc.Height = atlasHeight;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.MipLevels = 1;
    desc.MiscFlags = 0;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = atlasWidth * 4;

    ComPtr<ID3D11Texture2D> pTexture;
    HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, pTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

    ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
    hr = pDevice->CreateShaderResourceView(pTexture.Get(), &srvDesc, pShaderResourceView.GetAddressOf());
    if (FAILED(hr))
        return false;

    return outAtlas.Initialize(pShaderResourceView, atlasWidth, atlasHeight, std::move(regions));
}

bool CRuntimeAtlasBuilder::_LoadImageRGBA8(const char* filePath, vector<uint8_t>& outPixels, uint32_t& outWidth, uint32_t& outHeight) const
{
    outPixels.clear();
    outWidth = 0;
    outHeight = 0;

    if (filePath == nullptr || filePath[0] == '\0')
        return false;

    const wstring wPath = UTF8ToWstring(filePath);

    ComPtr<IWICImagingFactory> factory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
	if (FAILED(hr)) 
        return false;

	ComPtr<IWICBitmapDecoder> decoder;
	hr = factory->CreateDecoderFromFilename(wPath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.GetAddressOf());
	if (FAILED(hr)) 
        return false;

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, frame.GetAddressOf());
	if (FAILED(hr)) 
        return false;

	UINT w = 0, h = 0;
	hr = frame->GetSize(&w, &h);
	if (FAILED(hr) || w == 0 || h == 0) 
        return false;

	ComPtr<IWICFormatConverter> conv;
	hr = factory->CreateFormatConverter(conv.GetAddressOf());
	if (FAILED(hr)) 
        return false;

	hr = conv->Initialize(
		frame.Get(),
		GUID_WICPixelFormat32bppRGBA,   // RGBA32 강제
		WICBitmapDitherTypeNone,
		nullptr,
		0.0,
		WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) 
        return false;

	const uint32_t stride = (uint32_t)w * 4u;
	const uint32_t size = stride * (uint32_t)h;

    outPixels.resize(size);
	hr = conv->CopyPixels(nullptr, stride, size, outPixels.data());
	if (FAILED(hr)) 
        return false;

    outWidth = (uint32_t)w;
    outHeight = (uint32_t)h;
	return true;
}

void CRuntimeAtlasBuilder::_CopyPixelRGBA(const vector<uint8_t>& src, uint32_t srcWidth, uint32_t sx, uint32_t sy, vector<uint8_t>& dst, uint32_t dstWidth, uint32_t dx, uint32_t dy)
{
    const size_t srcIndex = (static_cast<size_t>(sy) * srcWidth + sx) * 4;
    const size_t dstIndex = (static_cast<size_t>(dy) * dstWidth + dx) * 4;

    dst[dstIndex + 0] = src[srcIndex + 0];
    dst[dstIndex + 1] = src[srcIndex + 1];
    dst[dstIndex + 2] = src[srcIndex + 2];
    dst[dstIndex + 3] = src[srcIndex + 3];
}
