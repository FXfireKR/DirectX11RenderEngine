#include "pch.h"
#include "CFrustumCuller.h"

void CFrustumCuller::Update(CXMMATRIX view, CXMMATRIX proj)
{
	BoundingFrustum localFrustum;
	BoundingFrustum::CreateFromMatrix(localFrustum, proj, false);

	const XMMATRIX invView = XMMatrixInverse(nullptr, view);
	localFrustum.Transform(m_worldFrustum, invView);
}

bool CFrustumCuller::IsVisible(const XMFLOAT3& worldTrans, const XMFLOAT3& boundCenter, const XMFLOAT3& boundExtents) const
{
	BoundingBox box{};
	box.Center =
	{
		worldTrans.x + boundCenter.x,
		worldTrans.y + boundCenter.y,
		worldTrans.z + boundCenter.z
	};
	box.Extents = boundExtents;
	return m_worldFrustum.Intersects(box);
}
