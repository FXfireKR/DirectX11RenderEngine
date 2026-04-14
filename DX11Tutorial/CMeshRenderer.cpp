#include "pch.h"
#include "CMeshRenderer.h"

CMeshRenderer::CMeshRenderer()
{
}

CMeshRenderer::~CMeshRenderer()
{
}

void CMeshRenderer::Init()
{
}

void CMeshRenderer::SetLocalBounds(const XMFLOAT3& center, const XMFLOAT3& extents)
{
	m_vLocalBoundsCenter = center;
	m_vLocalBoundsExtents = extents;
}