#include "pch.h"
#include "CWindow.h"

void CWindow::Initialize(HWND hWnd)
{
	m_hwnd = hWnd;
}

void CWindow::CalcWindowSize()
{
	if (!m_hwnd) return;

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	m_uWidth = rc.right - rc.left;
	m_uHeight = rc.bottom - rc.top;

	m_ptClientCenter.x = m_uWidth / 2;
	m_ptClientCenter.y = m_uHeight / 2;

	ClientToScreen(m_hwnd, &m_ptClientCenter);
}
