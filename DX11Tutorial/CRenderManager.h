#pragma once
#include "CRenderFrame.h"

//	class RenderManager
//	
//	* RenderFrame를 전반적으로 관리하고 연계할 수 있게 하는 매니지먼트 입니다.
//	* Max-render frame은 최대 몇 프레임을 세이브할 지 정합니다.
//	* Max 값은 capacity에 영향을 주고, vector size에 직접적으로 관여하지 않습니다. (제한)
//	

class CRenderManager
{
public:
	CRenderManager() = default;
	~CRenderManager() = default;

	void Initialize(size_t uMaxRenderFrame_, ID3D11Buffer& cbBuffer);
	void BeginFrame();
	void Submit(const RenderItem& renderItem);
	void EndFrame();

	void Draw(ID3D11DeviceContext* pContext);
	void DrawPass(ID3D11DeviceContext* pContext, ERenderPass ePass);

public:
	inline const bool CheckGPUOverload() { return m_queueReadyFrames.size() > m_uMaxRenderFrame; }
	inline const size_t& GetMaxWaitRenderFrame() { return m_uMaxRenderFrame; }

private:
	queue<unique_ptr<CRenderFrame>> m_queueReadyFrames;
	unique_ptr<CRenderFrame> m_pBuildingFrame = nullptr;

	size_t m_uMaxRenderFrame = 3; // default
	ID3D11Buffer* m_pCBBuffer = nullptr;
}; 