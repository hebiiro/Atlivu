#pragma once

#include "resource.h"
#include "MainFrame.h"

class CAtlivuInputApp : public CWinApp
{
public:

	HWND m_hostProcessWindow = 0;
	DWORD m_hostProcessId = 0;
	HANDLE m_hostProcess = 0;

	InputManagerPtr m_inputManager;
	MediaMap m_mediaMap;

	CAtlivuInputApp() noexcept;
	~CAtlivuInputApp();

public:

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CAtlivuInputApp theApp;
