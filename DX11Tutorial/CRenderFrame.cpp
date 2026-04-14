#include "pch.h"
#include "CRenderFrame.h"

void CRenderFrame::Initialize(ID3D11Buffer& cbObject)
{
	m_pCBObject = &cbObject;
}

void CRenderFrame::Submit(const RenderItem& renderItem)
{
	const size_t idx = static_cast<size_t>(renderItem.eRenderPass);
	if (idx >= m_arrPassBucket.size())
		return;

	m_arrPassBucket[idx].push_back(renderItem);
}

void CRenderFrame::Draw(ID3D11DeviceContext* pContext)
{
	_DrawPass(pContext, ERenderPass::SHADOW_PASS);
	_DrawPass(pContext, ERenderPass::SKY_PASS);
	_DrawPass(pContext, ERenderPass::CLOUD_PASS);
	_DrawPass(pContext, ERenderPass::OPAQUE_PASS);
	_DrawPass(pContext, ERenderPass::CUTOUT_PASS);
	_DrawPass(pContext, ERenderPass::TRANSPARENT_PASS);
	_DrawPass(pContext, ERenderPass::DEBUG_PASS);
	_DrawPass(pContext, ERenderPass::ORTH_PASS);
}

void CRenderFrame::DrawPass(ID3D11DeviceContext* pContext, ERenderPass ePass)
{
	_DrawPass(pContext, ePass);
}

void CRenderFrame::_DrawPass(ID3D11DeviceContext* pContext, ERenderPass ePass)
{
	auto& vecItems = m_arrPassBucket[static_cast<size_t>(ePass)];
	if (vecItems.empty())
		return;

	if (ERenderPass::TRANSPARENT_PASS == ePass)
	{
		std::stable_sort(vecItems.begin(), vecItems.end(), [](const RenderItem& a, const RenderItem& b){
			return a.fSortDepth > b.fSortDepth;
		});
	}

	_DrawItems(pContext, vecItems);
	vecItems.clear();
}

void CRenderFrame::_DrawItems(ID3D11DeviceContext* pContext, vector<RenderItem>& vecItems)
{
	const CMesh* pLastMesh = nullptr;
	const CPipeline* pLastPipeline = nullptr;
	const CMaterial* pLastMaterial = nullptr;

	for (const RenderItem& renderItem : vecItems)
	{
		if (!_CheckValidToDraw(renderItem))
			continue;

		if (pLastPipeline != renderItem.pPipeline) 
		{
			renderItem.pPipeline->Bind(pContext);
			pLastPipeline = renderItem.pPipeline;
			dbg.AddPipelineBind();
		}

		if (pLastMesh != renderItem.pMesh) 
		{
			renderItem.pMesh->Bind(pContext);
			pLastMesh = renderItem.pMesh;
			dbg.AddMeshBind();
		}

		if (nullptr != renderItem.pMaterial) 
		{
			if (pLastMaterial != renderItem.pMaterial) 
			{
				renderItem.pMaterial->Bind(pContext);
				pLastMaterial = renderItem.pMaterial;
				dbg.AddMaterialBind();
			}
		}

		_UpdateConstantBuffer(pContext, { renderItem.world });
		pContext->VSSetConstantBuffers(1, 1, &m_pCBObject);

		switch (renderItem.eRenderPass)
		{
			case ERenderPass::SHADOW_PASS:		dbg.AddDrawCallShadow(); break;
			case ERenderPass::SKY_PASS:			dbg.AddDrawCallSky(); break;
			case ERenderPass::OPAQUE_PASS:		dbg.AddDrawCallOpaque(); break;
			case ERenderPass::CUTOUT_PASS:		dbg.AddDrawCallCutout(); break;
			case ERenderPass::TRANSPARENT_PASS:	dbg.AddDrawCallTranslucent(); break;
			case ERenderPass::DEBUG_PASS:		dbg.AddDrawCallDebug(); break;
			case ERenderPass::ORTH_PASS:		dbg.AddDrawCallUI(); break;
			default: dbg.AddDrawCall(); break;
		}
		
		// [note] _CheckValidToDraw true면 pMesh는 nullptr이 아님. ensure.
		renderItem.pMesh->Draw(pContext);
	}
}

void CRenderFrame::_UpdateConstantBuffer(ID3D11DeviceContext* pContext, CB_ObjectData&& objData)
{
	static_assert(sizeof(CB_ObjectData) % 16 == 0, "CB_ObjectData must be 16-byte aligned.");

	D3D11_MAPPED_SUBRESOURCE mapped{};
	pContext->Map(m_pCBObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &objData, sizeof(CB_ObjectData));
	pContext->Unmap(m_pCBObject, 0);
}

bool CRenderFrame::_CheckValidToDraw(const RenderItem& renderItem)
{
	if (nullptr == renderItem.pPipeline) 
		return false;
	if (nullptr == renderItem.pMesh) 
		return false;
	//if (nullptr == renderItem.pMaterial) 
	//	return false; // optional

	return true;
}