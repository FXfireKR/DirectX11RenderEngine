#pragma once

class CWindow
{
public:
	CWindow() = default;
	~CWindow() = default;

	void Initialize(HWND hWnd);
	void CalcWindowSize();

public:
	inline const ULONG& GetClientWidth() const { return m_uWidth; }
	inline const ULONG& GetClientHeight() const { return m_uHeight; }
	inline const POINT& GetClientCenter() const { return m_ptClientCenter; }

private:
	HWND m_hwnd = NULL;
	
	ULONG m_uWidth;
	ULONG m_uHeight;
	POINT m_ptClientCenter;
};