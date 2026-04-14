#pragma once

class CDebugOverlay
{
public:
	void Render();

private:
	void _RenderMenuBar();

private:
	bool m_bOpen = true;
};