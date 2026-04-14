#include "pch.h"
#include "CInputManager.h"

void CInputManager::Initialize()
{
}

void CInputManager::BeginFrame()
{
	if (m_pMouse)
		m_pMouse->BeginFrame();

	if (m_pGamePad)
		m_pKeyboard->BeginFrame();

	if (m_pGamePad)
		m_pGamePad->BeginFrame();
}

void CInputManager::EndFrame()
{
	if (m_pMouse)
		m_pMouse->EndFrame();

	if (m_pKeyboard)
		m_pKeyboard->EndFrame();

	if (m_pGamePad)
		m_pGamePad->EndFrame();
}