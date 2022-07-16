#pragma once

#include "resource.h"
#include "MainFrame.h"

class CAtlivuOutputApp : public CWinApp
{
public:

	HWND m_hostProcessWindow = 0;
	DWORD m_hostProcessId = 0;
	HANDLE m_hostProcess = 0;

	HANDLE m_pipe = 0;

	OutputManagerPtr m_outputManager;
	MediaInfo m_mediaInfo = {};
	std::vector<BYTE> m_videoBuffer;
	std::vector<BYTE> m_audioBuffer;

	CAtlivuOutputApp() noexcept;
	~CAtlivuOutputApp();

	BOOL save(LPCTSTR fileName, MediaInfo* mediaInfo);

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::SendMessage(m_hostProcessWindow, message, wParam, lParam);
	}

	BOOL postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::PostMessage(m_hostProcessWindow, message, wParam, lParam);
	}

public:

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CAtlivuOutputApp theApp;
