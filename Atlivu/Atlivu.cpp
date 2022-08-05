#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

void ___outputLog(LPCTSTR text, LPCTSTR output)
{
	::OutputDebugString(output);
}

//--------------------------------------------------------------------

CAtlivuApp theApp;

//--------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAtlivuApp, CWinApp)
END_MESSAGE_MAP()

CAtlivuApp::CAtlivuApp() noexcept
{
	_tsetlocale(LC_CTYPE, _T(""));
}

CAtlivuApp::~CAtlivuApp()
{
}

BOOL CAtlivuApp::createInputProcess()
{
	MY_TRACE(_T("CAtlivuApp::createInputProcess()\n"));

	DWORD tid = ::GetCurrentThreadId();
	MY_TRACE_INT(tid);

	TCHAR pipeName[MAX_PATH] = {};
	::StringCbPrintf(pipeName, sizeof(pipeName), _T("\\\\.\\pipe\\aui%d"), tid);
	MY_TRACE_TSTR(pipeName);

	m_inputPipe = ::CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 1000, 0);
	MY_TRACE_HEX(m_inputPipe);

	DWORD dwMode = PIPE_READMODE_BYTE;
	BOOL fSuccess = ::SetNamedPipeHandleState(m_inputPipe, &dwMode, NULL, NULL);
	MY_TRACE_HEX(dwMode);
	MY_TRACE_INT(fSuccess);

	TCHAR path[MAX_PATH] = {};
	::GetModuleFileName(0, path, MAX_PATH);
	::PathRemoveFileSpec(path);
	::PathAppend(path, _T("aui.exe"));
	MY_TRACE_TSTR(path);

	TCHAR args[MAX_PATH] = {};
	::StringCbPrintf(args, sizeof(args), _T("0x%08p"), m_mainFrame.GetSafeHwnd());
	MY_TRACE_TSTR(args);

	STARTUPINFO si = { sizeof(si) };

	if (!::CreateProcess(
		path,           // No module name (use command line)
		args,           // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&m_inputPi))    // Pointer to PROCESS_INFORMATION structur
	{
		MY_TRACE(_T("::CreateProcess() failed.\n"));

		return FALSE;
	}

	return TRUE;
}

BOOL CAtlivuApp::createOutputProcess()
{
	MY_TRACE(_T("CAtlivuApp::createOutputProcess()\n"));

	DWORD tid = ::GetCurrentThreadId();
	MY_TRACE_INT(tid);

	TCHAR pipeName[MAX_PATH] = {};
	::StringCbPrintf(pipeName, sizeof(pipeName), _T("\\\\.\\pipe\\auo%d"), tid);
	MY_TRACE_TSTR(pipeName);

	m_outputPipe = ::CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, 0, 1000, 0);
	MY_TRACE_HEX(m_outputPipe);

	DWORD dwMode = PIPE_READMODE_BYTE;
	BOOL fSuccess = ::SetNamedPipeHandleState(m_outputPipe, &dwMode, NULL, NULL);
	MY_TRACE_HEX(dwMode);
	MY_TRACE_INT(fSuccess);

	TCHAR path[MAX_PATH] = {};
	::GetModuleFileName(0, path, MAX_PATH);
	::PathRemoveFileSpec(path);
	::PathAppend(path, _T("auo.exe"));
	MY_TRACE_TSTR(path);

	TCHAR args[MAX_PATH] = {};
	::StringCbPrintf(args, sizeof(args), _T("0x%08p"), m_mainFrame.GetSafeHwnd());
	MY_TRACE_TSTR(args);

	STARTUPINFO si = { sizeof(si) };

	if (!::CreateProcess(
		path,           // No module name (use command line)
		args,           // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&m_outputPi))   // Pointer to PROCESS_INFORMATION structur
	{
		MY_TRACE(_T("::CreateProcess() failed.\n"));

		return FALSE;
	}

	return TRUE;
}

BOOL CAtlivuApp::initInputPlugin()
{
	MY_TRACE(_T("CAtlivuApp::initInputPlugin()\n"));

	{
		// パイプの接続を確認する。

		BOOL result = ::ConnectNamedPipe(m_inputPipe, 0);
		MY_TRACE_INT(result);
		if (!result)
		{
			DWORD error = ::GetLastError();
			MY_TRACE_HEX(error);
			if (error == ERROR_PIPE_CONNECTED)
			{
				MY_TRACE(_T("パイプの接続は成功しています\n"));
			}
		}
	}

	CString iniPath = getIniPath();

	{
		TCHAR fileName[MAX_PATH] = {};
		::GetPrivateProfileString(_T("config"), _T("inputPlugin"), _T(""), fileName, MAX_PATH, iniPath);

		if (raw_loadInputPlugin(fileName))
		{
			TCHAR fileName[MAX_PATH] = {};
			::GetPrivateProfileString(_T("config"), _T("media"), _T(""), fileName, MAX_PATH, iniPath);

			openMedia(fileName);
		}
	}

	return TRUE;
}

BOOL CAtlivuApp::termInputPlugin()
{
	MY_TRACE(_T("CAtlivuApp::termInputPlugin()\n"));

	raw_unloadInputPlugin();

	return TRUE;
}

BOOL CAtlivuApp::initOutputPlugin()
{
	MY_TRACE(_T("CAtlivuApp::initOutputPlugin()\n"));

	{
		// パイプの接続を確認する。

		BOOL result = ::ConnectNamedPipe(m_outputPipe, 0);
		MY_TRACE_INT(result);
		if (!result)
		{
			DWORD error = ::GetLastError();
			MY_TRACE_HEX(error);
			if (error == ERROR_PIPE_CONNECTED)
			{
				MY_TRACE(_T("パイプの接続は成功しています\n"));
			}
		}
	}

	CString iniPath = getIniPath();

	{
		// 前回使用していた出力プラグインを読み込む。

		TCHAR fileName[MAX_PATH] = {};
		::GetPrivateProfileString(_T("config"), _T("outputPlugin"), _T(""), fileName, MAX_PATH, iniPath);

		raw_loadOutputPlugin(fileName);
	}
#if 0
	// 起動してすぐ動画出力のテストをするなら
	save(_T("C:\\Users\\main\\source\\repos\\Atlivu\\_bin\\test.mp4"));
#endif
	return 0;
}

BOOL CAtlivuApp::termOutputPlugin()
{
	MY_TRACE(_T("CAtlivuApp::termOutputPlugin()\n"));

	raw_unloadOutputPlugin();

	return TRUE;
}

BOOL CAtlivuApp::openMedia(LPCTSTR fileName)
{
	MY_TRACE(_T("CAtlivuApp::openMedia(%s)\n"), fileName);

	if (m_media)
		raw_closeMedia(m_media), m_media = 0;

	m_media = raw_openMedia(fileName);
	if (!m_media) return FALSE;

	raw_getMediaInfo(m_media, &m_mediaInfo);

	m_mainFrame.OnOpenMedia(&m_mediaInfo);

	return TRUE;
}

BOOL CAtlivuApp::closeMedia()
{
	MY_TRACE(_T("CAtlivuApp::CloseMedia()\n"));

	if (!m_media)
		return FALSE;

	raw_closeMedia(m_media), m_media = 0;

	return TRUE;
}

BOOL CAtlivuApp::InitInstance()
{
	CWinApp::InitInstance();

	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
//	AfxInitRichEdit2();

	m_gdiSI.SuppressBackgroundThread = TRUE;
	GdiplusStartup(&m_gdiToken, &m_gdiSI, &m_gdiSO);
	m_gdiSO.NotificationHook(&m_gdiHookToken);

	m_videoProcessor.reset(new CVideoProcessor());
	m_videoProcessor->CreateThread();

	m_pMainWnd = &m_mainFrame;
	m_mainFrame.Create();
	m_mainFrame.ShowWindow(SW_SHOW);
	m_mainFrame.UpdateWindow();

	initInputPlugin();
	initOutputPlugin();

	return TRUE;
}

int CAtlivuApp::ExitInstance()
{
	termOutputPlugin();
	termInputPlugin();

	m_videoProcessor->postMessage(WM_CLOSE, 0, 0);
	::WaitForSingleObject(m_videoProcessor->m_hThread, INFINITE);

	m_gdiSO.NotificationUnhook(m_gdiHookToken);
	GdiplusShutdown(m_gdiToken);

	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}

//--------------------------------------------------------------------
