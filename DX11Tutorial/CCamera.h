#pragma once
#include "CComponentBase.h"

class CTransform;

enum class CAMERA_PROJECTION_TYPE
{
	PERSPECTIVE,	// 원근 (깊이, 거리에 따른 랜더 차등)
	ORTHOGRAPHIC,	// 직교 (깊이, 거리에 관계 없는 동일 랜더)
};

struct KPerspectiveParams
{
	float fFieldOfView = XM_PI / 2.0f;//XM_PI / 4.0f;
	float fAspectRatio = 1440.0f / 1024.0f;
	float fNearZ = 0.1f;
	float fFarZ = 1000.0f;
};

struct KOrthographicParams
{
	float fWidth = 10.0f;
	float fHeight = 10.0f;
	float fNearZ = 0.1f;
	float fFarZ = 1000.0f;
};

class CCamera : public CComponentBase<CCamera, COMPONENT_TYPE::CAMERA>
{
public:
	CCamera() = default;
	virtual ~CCamera() = default;

	void Init() override;
	void Start() override;
	void Build() override;

	void UpdateCameraMatrix();

	void SetProjToPerspective();
	void SetProjToOrthographic();

	void SetOrthographicSize(float width, float height);
	void SetOrthographicNearFar(float nearZ, float farZ);

public:
	void SetFov(float newFov);
	void SetAspectRatio(float newRatio);

	inline const CAMERA_PROJECTION_TYPE& GetProjectionType() const { return m_eProjectionType; }
	void SetProjectionType(const CAMERA_PROJECTION_TYPE& eProjectionType_);

	inline const XMMATRIX& GetViewMatrix() const { return m_matView; }
	inline const XMMATRIX& GetProjMatrix() const { return m_matProjection; }

	inline const KPerspectiveParams& GetPerspectiveParams() const { return m_kPerspective; }
	inline const KOrthographicParams& GetOrthographicParams() const { return m_kOrthographic; }

	const CTransform* GetTransform() const;

private:
	void _UpdateViewMatrix();
	void _UpdateProjectionMatrix();

private:
	CAMERA_PROJECTION_TYPE m_eProjectionType = CAMERA_PROJECTION_TYPE::PERSPECTIVE;
	
	XMMATRIX m_matView = XMMatrixIdentity();
	XMMATRIX m_matProjection = XMMatrixIdentity();

	CTransform* m_pOwnTransform = nullptr;
	bool m_bDirty = true;

	KPerspectiveParams m_kPerspective{};
	KOrthographicParams m_kOrthographic{};
}; 