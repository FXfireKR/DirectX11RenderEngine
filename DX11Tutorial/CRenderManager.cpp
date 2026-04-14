#include "pch.h"
#include "CRenderManager.h"

void CRenderManager::Initialize(size_t uMaxRenderFrame_, ID3D11Buffer& cbBuffer)
{
	m_uMaxRenderFrame = uMaxRenderFrame_;
	m_pCBBuffer = &cbBuffer;

	m_pBuildingFrame = nullptr;
}

void CRenderManager::BeginFrame()
{
	m_pBuildingFrame = make_unique<CRenderFrame>();
	m_pBuildingFrame->Initialize(*m_pCBBuffer);
}

void CRenderManager::Submit(const RenderItem& renderItem)
{
	m_pBuildingFrame->Submit(renderItem);
}

void CRenderManager::EndFrame()
{
	//m_queueReadyFrames.push(move(m_pBuildingFrame));
}

void CRenderManager::Draw(ID3D11DeviceContext* pContext)
{
	//if (m_queueReadyFrames.empty()) return;

	// m_queueReadyFrames.front();

	if (!m_pBuildingFrame) return;

	m_pBuildingFrame->Draw(pContext);

	m_pBuildingFrame.reset();
}

void CRenderManager::DrawPass(ID3D11DeviceContext* pContext, ERenderPass ePass)
{
	if (!m_pBuildingFrame)
		return;
	m_pBuildingFrame->DrawPass(pContext, ePass);
}