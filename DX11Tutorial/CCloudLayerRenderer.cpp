#include "pch.h"
#include "CCloudLayerRenderer.h"

bool CCloudLayerRenderer::Initialize(CRenderWorld& rw, const wchar_t* maskPathW)
{
    if (!_LoadMaskWIC(maskPathW))
        return false;

    auto& shaderManager = rw.GetShaderManager();
    auto& ilManager = rw.GetIALayoutManager();
    auto& pipelineManager = rw.GetPipelineManager();

    const uint64_t shaderID = fnv1a_64("ColorForward");
    const CShader* pShader = shaderManager.CreateShader(shaderID, 0);
    shaderManager.Compile();

    const uint64_t layoutID =
        ilManager.Create(VERTEX_POSITION_COLOR::GetLayout(), { shaderID, 0 }, pShader->GetVertexBlob());

    const uint64_t pipeID = pipelineManager.Create(fnv1a_64("CloudLayerPipeline"));
    m_pPipeline = pipelineManager.Get(pipeID);
    m_pPipeline->SetShader(shaderManager.Get(shaderID, 0));
    m_pPipeline->SetInputLayout(ilManager.Get(layoutID));
    m_pPipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pPipeline->CreateOpaqueState(rw.GetDevice());

    return true;
}

void CCloudLayerRenderer::Update(float fDelta, const XMFLOAT3& cameraPos)
{
    m_fScrollAccum += fDelta * m_fScrollSpeedCellPerSec;

    const int newScrollCellX = static_cast<int>(floorf(m_fScrollAccum));

    const int centerCellX = static_cast<int>(floorf(cameraPos.x / m_fCloudCellSize));
    const int centerCellZ = static_cast<int>(floorf(cameraPos.z / m_fCloudCellSize));

    const int snappedAnchorX = (centerCellX / m_iAnchorSnapStep) * m_iAnchorSnapStep;
    const int snappedAnchorZ = (centerCellZ / m_iAnchorSnapStep) * m_iAnchorSnapStep;

    if (snappedAnchorX != m_iAnchorCellX ||
        snappedAnchorZ != m_iAnchorCellZ ||
        newScrollCellX != m_iScrollCellX)
    {
        m_iAnchorCellX = snappedAnchorX;
        m_iAnchorCellZ = snappedAnchorZ;
        m_iCenterCellX = centerCellX;   // 추가
        m_iCenterCellZ = centerCellZ;   // 추가
        m_iScrollCellX = newScrollCellX;
        m_bDirty = true;
    }
    else
    {
        // anchor 안 바뀌어도 center는 최신값 유지
        m_iCenterCellX = centerCellX;
        m_iCenterCellZ = centerCellZ;
    }
}

void CCloudLayerRenderer::Submit(CRenderWorld& rw)
{
    if (!m_pPipeline)
        return;

    if (m_bDirty)
    {
        _RebuildMesh(rw);
        m_bDirty = false;
    }

    if (!m_pMesh)
        return;

    const float fracScrollX = m_fScrollAccum - floorf(m_fScrollAccum);

    const float worldX = (m_iAnchorCellX * m_fCloudCellSize) - (fracScrollX * m_fCloudCellSize);
    const float worldZ = (m_iAnchorCellZ * m_fCloudCellSize);

    const XMMATRIX matWorld = XMMatrixTranslation(worldX, 0.f, worldZ);

    RenderItem item{};
    item.eRenderPass = ERenderPass::CLOUD_PASS;
    item.pMesh = m_pMesh;
    item.pPipeline = m_pPipeline;
    item.pMaterial = nullptr;
    XMStoreFloat4x4(&item.world, XMMatrixTranspose(matWorld));
    rw.Submit(item);
}

bool CCloudLayerRenderer::_LoadMaskWIC(const wchar_t* pathW)
{
    m_mask.clear();
    m_iMaskWidth = 0;
    m_iMaskHeight = 0;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool bNeedUninit = SUCCEEDED(hr);

    IWICImagingFactory* pFactory = nullptr;
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory));
    if (FAILED(hr) || !pFactory)
    {
        if (bNeedUninit) CoUninitialize();
        return false;
    }

    IWICBitmapDecoder* pDecoder = nullptr;
    hr = pFactory->CreateDecoderFromFilename(
        pathW,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder);
    if (FAILED(hr) || !pDecoder)
    {
        pFactory->Release();
        if (bNeedUninit) CoUninitialize();
        return false;
    }

    IWICBitmapFrameDecode* pFrame = nullptr;
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr) || !pFrame)
    {
        pDecoder->Release();
        pFactory->Release();
        if (bNeedUninit) CoUninitialize();
        return false;
    }

    IWICFormatConverter* pConverter = nullptr;
    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr) || !pConverter)
    {
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        if (bNeedUninit) CoUninitialize();
        return false;
    }

    hr = pConverter->Initialize(
        pFrame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
    {
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        if (bNeedUninit) CoUninitialize();
        return false;
    }

    UINT w = 0, h = 0;
    pConverter->GetSize(&w, &h);
    if (w == 0 || h == 0)
    {
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        if (bNeedUninit) CoUninitialize();
        return false;
    }

    vector<uint8_t> rgba;
    rgba.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);

    hr = pConverter->CopyPixels(
        nullptr,
        w * 4,
        static_cast<UINT>(rgba.size()),
        rgba.data());

    pConverter->Release();
    pFrame->Release();
    pDecoder->Release();
    pFactory->Release();
    if (bNeedUninit) CoUninitialize();

    if (FAILED(hr))
        return false;

    m_iMaskWidth = static_cast<int>(w);
    m_iMaskHeight = static_cast<int>(h);
    m_mask.resize(static_cast<size_t>(w) * static_cast<size_t>(h), 0);

    for (int y = 0; y < m_iMaskHeight; ++y)
    {
        for (int x = 0; x < m_iMaskWidth; ++x)
        {
            const size_t src = (static_cast<size_t>(y) * static_cast<size_t>(m_iMaskWidth) + static_cast<size_t>(x)) * 4;

            const uint8_t r = rgba[src + 0];
            const uint8_t g = rgba[src + 1];
            const uint8_t b = rgba[src + 2];
            const uint8_t a = rgba[src + 3];

            const uint8_t luma = static_cast<uint8_t>((static_cast<int>(r) * 30 + static_cast<int>(g) * 59 + static_cast<int>(b) * 11) / 100);
            const bool bFilled = (a > 24) || (luma > 128);

            m_mask[static_cast<size_t>(y) * static_cast<size_t>(m_iMaskWidth) + static_cast<size_t>(x)] = bFilled ? 1 : 0;
        }
    }

    return true;
}

bool CCloudLayerRenderer::_SampleMask(int worldCellX, int worldCellZ) const
{
    if (m_mask.empty() || m_iMaskWidth <= 0 || m_iMaskHeight <= 0)
        return false;

    int sampleX = worldCellX + m_iScrollCellX;
    int sampleZ = worldCellZ;

    sampleX %= m_iMaskWidth;
    if (sampleX < 0) sampleX += m_iMaskWidth;

    sampleZ %= m_iMaskHeight;
    if (sampleZ < 0) sampleZ += m_iMaskHeight;

    const int idx = sampleZ * m_iMaskWidth + sampleX;
    return m_mask[idx] != 0;
}

void CCloudLayerRenderer::_RebuildMesh(CRenderWorld& rw)
{
    vector<VERTEX_POSITION_COLOR> verts;
    vector<uint32_t> indices;

    verts.reserve(4096);
    indices.reserve(6144);

    for (int dz = -m_iHalfWindow; dz < m_iHalfWindow; ++dz)
    {
        for (int dx = -m_iHalfWindow; dx < m_iHalfWindow; ++dx)
        {
            const int cellX = m_iCenterCellX + dx;
            const int cellZ = m_iCenterCellZ + dz;

            if (!_SampleMask(cellX, cellZ))
                continue;

            const bool pxOpen = !_SampleMask(cellX + 1, cellZ);
            const bool nxOpen = !_SampleMask(cellX - 1, cellZ);
            const bool pzOpen = !_SampleMask(cellX, cellZ + 1);
            const bool nzOpen = !_SampleMask(cellX, cellZ - 1);

            _AppendCellFaces(cellX, cellZ, pxOpen, nxOpen, pzOpen, nzOpen, verts, indices);
        }
    }

    m_pMesh = rw.GetMeshManager().CreateOrUpdateDynamicMesh(
        rw.GetContext(),
        m_uMeshKey,
        verts.empty() ? nullptr : verts.data(),
        sizeof(VERTEX_POSITION_COLOR),
        static_cast<uint32_t>(verts.size()),
        indices.empty() ? nullptr : indices.data(),
        static_cast<uint32_t>(indices.size()));
}

void CCloudLayerRenderer::_AppendCellFaces(
    int cellX, int cellZ,
    bool pxOpen, bool nxOpen,
    bool pzOpen, bool nzOpen,
    vector<VERTEX_POSITION_COLOR>& outVerts,
    vector<uint32_t>& outIndices) const
{
    const float x0 = (cellX - m_iAnchorCellX) * m_fCloudCellSize;
    const float x1 = x0 + m_fCloudCellSize;
    const float z0 = (cellZ - m_iAnchorCellZ) * m_fCloudCellSize;
    const float z1 = z0 + m_fCloudCellSize;

    const float y0 = m_fCloudBaseY;
    const float y1 = y0 + m_fCloudThickness;

    /*
    const XMFLOAT4 topCol = { 1.00f, 1.00f, 1.00f, 1.0f };
    const XMFLOAT4 sideCol = { 0.92f, 0.92f, 0.94f, 1.0f };
    const XMFLOAT4 bottomCol = { 0.80f, 0.80f, 0.84f, 1.0f };
    */

    const XMFLOAT4 topCol = { 1.00f, 1.00f, 1.00f, 1.0f };
    const XMFLOAT4 sideCol = { 0.88f, 0.89f, 0.92f, 1.0f };
    const XMFLOAT4 bottomCol = { 0.72f, 0.74f, 0.78f, 1.0f };

    _AppendQuad(
        { x0, y1, z0 }, { x1, y1, z0 }, { x1, y1, z1 }, { x0, y1, z1 },
        topCol, outVerts, outIndices);

    _AppendQuad(
        { x0, y0, z1 }, { x1, y0, z1 }, { x1, y0, z0 }, { x0, y0, z0 },
        bottomCol, outVerts, outIndices);

    if (pxOpen)
    {
        _AppendQuad(
            { x1, y0, z0 }, { x1, y0, z1 }, { x1, y1, z1 }, { x1, y1, z0 },
            sideCol, outVerts, outIndices);
    }

    if (nxOpen)
    {
        _AppendQuad(
            { x0, y0, z1 }, { x0, y0, z0 }, { x0, y1, z0 }, { x0, y1, z1 },
            sideCol, outVerts, outIndices);
    }

    if (pzOpen)
    {
        _AppendQuad(
            { x1, y0, z1 }, { x0, y0, z1 }, { x0, y1, z1 }, { x1, y1, z1 },
            sideCol, outVerts, outIndices);
    }

    if (nzOpen)
    {
        _AppendQuad(
            { x0, y0, z0 }, { x1, y0, z0 }, { x1, y1, z0 }, { x0, y1, z0 },
            sideCol, outVerts, outIndices);
    }
}

void CCloudLayerRenderer::_AppendQuad(
    const XMFLOAT3& p0,
    const XMFLOAT3& p1,
    const XMFLOAT3& p2,
    const XMFLOAT3& p3,
    const XMFLOAT4& color,
    vector<VERTEX_POSITION_COLOR>& outVerts,
    vector<uint32_t>& outIndices) const
{
    const uint32_t base = static_cast<uint32_t>(outVerts.size());

    outVerts.push_back({ { p0.x, p0.y, p0.z }, { color.x, color.y, color.z, color.w } });
    outVerts.push_back({ { p1.x, p1.y, p1.z }, { color.x, color.y, color.z, color.w } });
    outVerts.push_back({ { p2.x, p2.y, p2.z }, { color.x, color.y, color.z, color.w } });
    outVerts.push_back({ { p3.x, p3.y, p3.z }, { color.x, color.y, color.z, color.w } });

    outIndices.push_back(base + 0);
    outIndices.push_back(base + 1);
    outIndices.push_back(base + 2);

    outIndices.push_back(base + 0);
    outIndices.push_back(base + 2);
    outIndices.push_back(base + 3);
}