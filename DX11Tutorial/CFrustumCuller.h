#pragma once
#include <DirectXCollision.h>

class CFrustumCuller
{
public:
	void Update(CXMMATRIX view, CXMMATRIX proj);
	bool IsVisible(const XMFLOAT3& worldTrans, const XMFLOAT3& boundCenter, const XMFLOAT3& boundExtents) const;

private:
	BoundingFrustum m_worldFrustum{};
};