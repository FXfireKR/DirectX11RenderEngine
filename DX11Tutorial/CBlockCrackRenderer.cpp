#include "pch.h"
#include "CBlockCrackRenderer.h"

#include "CRenderWorld.h"
#include "CBlockInteractor.h"
#include "CMeshManager.h"
#include "CShaderManager.h"
#include "CInputLayerManager.h"
#include "CPipelineManager.h"
#include "CMaterialManager.h"
#include "CTextureManager.h"
#include "CSamplerManager.h"

bool CBlockCrackRenderer::Initialize(CRenderWorld& rw)
{
    auto& shaderManager = rw.GetShaderManager();
    auto& ilManager = rw.GetIALayoutManager();
    auto& pipelineManager = rw.GetPipelineManager();
    auto& meshManager = rw.GetMeshManager();
    auto& materialManager = rw.GetMaterialManager();
    auto& textureManager = rw.GetTextureManager();
    auto& samplerManager = rw.GetSamplerManager();

    const uint64_t shaderID = fnv1a_64("NormalImageForward");
    const CShader* pShader = shaderManager.CreateShader(shaderID, 0);
    shaderManager.Compile();

    const uint64_t layoutID =
        ilManager.Create(VERTEX_POSITION_NORMAL_UV_COLOR::GetLayout(), { shaderID, 0 }, pShader->GetVertexBlob());

    const uint64_t pipeID = pipelineManager.Create(fnv1a_64("BlockCrackPipeline"));
    m_pPipeline = pipelineManager.Get(pipeID);
    m_pPipeline->SetShader(shaderManager.Get(shaderID, 0));
    m_pPipeline->SetInputLayout(ilManager.Get(layoutID));
    m_pPipeline->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_pPipeline->CreateTransparentAlphaState(rw.GetDevice(), false);

    const uint64_t crackSamplerID = samplerManager.Create(SAMPLER_TYPE::POINT_CLAMP);
    const uint64_t shadowSamplerID = samplerManager.Create(SAMPLER_TYPE::SHADOWCOMPARISON);

    for (int i = 0; i < 10; ++i)
    {
        char matName[64];
        sprintf_s(matName, "BlockCrackMaterial_%d", i);
        const uint64_t materialID = materialManager.Create(fnv1a_64(matName));
        CMaterial* pMaterial = materialManager.Get(materialID);

        char texPath[256];
        sprintf_s(texPath, "../Resource/assets/minecraft/textures/block/destroy_stage_%d.png", i);

        char texName[64];
        sprintf_s(texName, "destroy_stage_%d", i);
        const uint64_t textureID = textureManager.LoadTexture2D(
            fnv1a_64(texName),
            texPath,
            TEXTURE_USAGE::StaticColor);

        pMaterial->SetTexture(0, textureManager.GetTexture(textureID)->GetShaderResourceView());
        pMaterial->SetSampler(0, samplerManager.Get(crackSamplerID)->Get());
        pMaterial->SetTexture(1, rw.GetShadowMapSRV());
        pMaterial->SetSampler(1, samplerManager.Get(shadowSamplerID)->Get());

        m_arrStageMaterials[i] = pMaterial;
    }

    // 전용 mesh는 Submit 전에 lazy build
    m_pMesh = nullptr;

    return (m_pPipeline != nullptr);
}

void CBlockCrackRenderer::Update(const CBlockInteractor& interactor)
{
    m_bVisible = false;
    m_iStage = 0;

    if (!interactor.IsMining())
        return;

    const BlockCell& cell = interactor.GetMiningCell();
    if (cell.IsAir())
        return;

    if (!BlockDB.IsFullCube(cell.blockID))
        return;

    m_block = interactor.GetMiningBlock();

    const float t = interactor.GetBreakProgress01();
    m_iStage = std::clamp(static_cast<int>(t * 10.0f), 0, 9);
    m_bVisible = true;
}

void CBlockCrackRenderer::Submit(CRenderWorld& rw)
{
    if (!m_bVisible || !m_pPipeline)
        return;

    CMaterial* pMaterial = m_arrStageMaterials[m_iStage];
    if (!pMaterial)
        return;

    if (!m_pMesh)
    {
        vector<VERTEX_POSITION_NORMAL_UV_COLOR> verts;
        vector<uint32_t> indices;
        _BuildCrackCubeMesh(verts, indices);

        m_pMesh = rw.GetMeshManager().CreateOrUpdateDynamicMesh(
            rw.GetContext(),
            m_uMeshKey,
            verts.data(),
            sizeof(VERTEX_POSITION_NORMAL_UV_COLOR),
            static_cast<uint32_t>(verts.size()),
            indices.data(),
            static_cast<uint32_t>(indices.size()));
    }

    if (!m_pMesh)
        return;

    const float scale = 1.006f;

    const XMMATRIX matS = XMMatrixScaling(scale, scale, scale);
    const XMMATRIX matT = XMMatrixTranslation(
        static_cast<float>(m_block.x) + 0.5f,
        static_cast<float>(m_block.y) + 0.5f,
        static_cast<float>(m_block.z) + 0.5f);

    RenderItem item{};
    item.eRenderPass = ERenderPass::TRANSPARENT_PASS;
    item.pMesh = m_pMesh;
    item.pPipeline = m_pPipeline;
    item.pMaterial = pMaterial;
    XMStoreFloat4x4(&item.world, XMMatrixTranspose(matS * matT));
    rw.Submit(item);
}

void CBlockCrackRenderer::_BuildCrackCubeMesh(vector<VERTEX_POSITION_NORMAL_UV_COLOR>& outVerts, vector<uint32_t>& outIndices)
{
    outVerts.clear();
    outIndices.clear();

    const XMFLOAT4 col = { 1.f, 1.f, 1.f, 1.f };

    auto AppendFace = [&](const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, const XMFLOAT3& n)
        {
            const uint32_t base = static_cast<uint32_t>(outVerts.size());

            outVerts.push_back({ p0, n, {0.f, 0.f}, col });
            outVerts.push_back({ p1, n, {1.f, 0.f}, col });
            outVerts.push_back({ p2, n, {0.f, 1.f}, col });
            outVerts.push_back({ p3, n, {1.f, 1.f}, col });

            outIndices.push_back(base + 0);
            outIndices.push_back(base + 1);
            outIndices.push_back(base + 2);
            outIndices.push_back(base + 2);
            outIndices.push_back(base + 1);
            outIndices.push_back(base + 3);
        };

    const float x0 = -0.5f, x1 = 0.5f;
    const float y0 = -0.5f, y1 = 0.5f;
    const float z0 = -0.5f, z1 = 0.5f;

    // front (-z)
    AppendFace({ x0,y1,z0 }, { x1,y1,z0 }, { x0,y0,z0 }, { x1,y0,z0 }, { 0,0,-1 });
    // right (+x)
    AppendFace({ x1,y1,z0 }, { x1,y1,z1 }, { x1,y0,z0 }, { x1,y0,z1 }, { 1,0,0 });
    // back (+z)
    AppendFace({ x1,y1,z1 }, { x0,y1,z1 }, { x1,y0,z1 }, { x0,y0,z1 }, { 0,0,1 });
    // left (-x)
    AppendFace({ x0,y1,z1 }, { x0,y1,z0 }, { x0,y0,z1 }, { x0,y0,z0 }, { -1,0,0 });
    // top (+y)
    AppendFace({ x0,y1,z1 }, { x1,y1,z1 }, { x0,y1,z0 }, { x1,y1,z0 }, { 0,1,0 });
    // bottom (-y)
    AppendFace({ x0,y0,z0 }, { x1,y0,z0 }, { x0,y0,z1 }, { x1,y0,z1 }, { 0,-1,0 });
}