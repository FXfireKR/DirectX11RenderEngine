#pragma once
#include "CComponentBase.h"
#include "CMesh.h"
#include "CPipeline.h"
#include "CMaterial.h"
#include "RenderPassTypes.h"

class CMeshRenderer : public CComponentBase<CMeshRenderer, COMPONENT_TYPE::MESHRENDER>
{
public:
	CMeshRenderer();
	~CMeshRenderer();

	void Init() override;

public:
	void SetLocalBounds(const XMFLOAT3& center, const XMFLOAT3& extents);

public:
	inline void SetMesh(CMesh* pMesh_) { m_pMesh = pMesh_; }
	inline const CMesh* const GetMesh() { return m_pMesh; }

	inline void SetPipeline(CPipeline* pPipeline_) { m_pPipeline = pPipeline_; }
	inline const CPipeline* const GetPipeline() { return m_pPipeline; }

	inline void SetMaterial(CMaterial* pMaterial_) { m_pMaterial = pMaterial_; }
	inline const CMaterial* const GetMaterial() { return m_pMaterial; }

	inline void SetRenderPass(ERenderPass ePass_) { m_eRenderPass = ePass_; }
	inline const ERenderPass GetRenderPass() const { return m_eRenderPass; }

	inline void SetFrustumCullEnabled(bool bEnable) { m_bFrustumCullEnabled = bEnable; }
	inline const bool IsFrustumCullEnabled() const { return m_bFrustumCullEnabled; }

	inline const XMFLOAT3& GetLocalBoundsCenter() const { return m_vLocalBoundsCenter; }
	inline const XMFLOAT3& GetLocalBoundsExtents() const { return m_vLocalBoundsExtents; }

private:
	CMesh* m_pMesh = nullptr;
	CPipeline* m_pPipeline = nullptr;
	CMaterial* m_pMaterial = nullptr;
	ERenderPass m_eRenderPass = ERenderPass::OPAQUE_PASS;

	bool m_bFrustumCullEnabled = false;
	XMFLOAT3 m_vLocalBoundsCenter = { 0.f, 0.f, 0.f };
	XMFLOAT3 m_vLocalBoundsExtents = { 0.f, 0.f, 0.f };
};