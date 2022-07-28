#include "pch.h"
#include "Atlivu.h"
#include "MainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMainFrame, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_MESSAGE(WM_ATLIVU_INPUT_INITED, OnAtlivuInputInited)
	ON_MESSAGE(WM_ATLIVU_OUTPUT_INITED, OnAtlivuOutputInited)
	ON_COMMAND(ID_OPEN_MEDIA, OnOpenMedia)
	ON_COMMAND(ID_SAVE_MEDIA, OnSaveMedia)
	ON_COMMAND(ID_ABORT_SAVE_MEDIA, OnAbortSaveMedia)
	ON_COMMAND(ID_SELECT_INPUT_PLUGIN, OnSelectInputPlugin)
	ON_COMMAND(ID_SET_INPUT_PLUGIN_CONFIG, OnSetInputPluginConfig)
	ON_COMMAND(ID_SELECT_OUTPUT_PLUGIN, OnSelectOutputPlugin)
	ON_COMMAND(ID_SET_OUTPUT_PLUGIN_CONFIG, OnSetOutputPluginConfig)
	ON_MESSAGE(WM_VIDEO_PROCESSED_SEEK, OnVideoProcessedSeek)
	ON_MESSAGE(WM_VIDEO_PROCESSED_PLAY, OnVideoProcessedPlay)
	ON_MESSAGE(WM_VIDEO_PROCESSED_PLAY_STOP, OnVideoProcessedPlayStop)
	ON_MESSAGE(WM_IS_ABORT, OnIsAbort)
	ON_MESSAGE(WM_REST_TIME_DISP, OnRestTimeDisp)
	ON_MESSAGE(WM_UPDATE_PREVIEW, OnUpdatePreview)
	ON_MESSAGE(WM_GET_VIDEO, OnGetVideo)
	ON_MESSAGE(WM_GET_AUDIO, OnGetAudio)
	ON_MESSAGE(WM_SAVE_FILE_FINISHED, OnSaveFileFinished)
	ON_MESSAGE(WM_CREATE_OUTPUT_VIDEO, OnCreateOutputVideo)
END_MESSAGE_MAP()

CMainFrame::CMainFrame() noexcept
{
	EnableD2DSupport();
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::setupPixelFormat(CDC& dc)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(pfd),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pixelFormat = ::ChoosePixelFormat(dc, &pfd);
	MY_TRACE_INT(pixelFormat);

	if (!pixelFormat)
	{
		MY_TRACE(_T("::ChoosePixelFormat() が失敗しました\n"));

		return FALSE;
	}

	if (!::SetPixelFormat(dc, pixelFormat, &pfd))
	{
		MY_TRACE(_T("::SetPixelFormat() が失敗しました\n"));

		return FALSE;
	}

	return TRUE;
}

BOOL CMainFrame::Create()
{
	if (!CWnd::CreateEx(0, 0, 0, 0, CRect(0, 600, 800, 1000), 0, 0))
		return FALSE;

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = AfxWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = AfxGetInstanceHandle();
	wc.hCursor = ::LoadCursor(0, IDC_CROSS);
	wc.lpszClassName = _T("Atlivu.MainFrame");
	AfxRegisterClass(&wc);
	cs.lpszName = _T("Atlivu.MainFrame");
	cs.lpszClass = _T("Atlivu.MainFrame");
//	cs.style = WS_POPUP | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
//	cs.style = WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = 0;
//	cs.dwExStyle = WS_EX_TOOLWINDOW | WS_EX_NOPARENTNOTIFY;

	CMenu menu; menu.LoadMenu(IDR_MAIN_VIEW);
	cs.hMenu = menu.Detach();

	return CWnd::PreCreateWindow(cs);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	{
		CClientDC dc(this);

		setupPixelFormat(dc);

		m_rc = ::wglCreateContext(dc);
		MY_TRACE_HEX(m_rc);

		if (!m_rc)
		{
			MY_TRACE(_T("::wglCreateContext() が失敗しました\n"));

			return -1;
		}
	}

	if (!theApp.createInputProcess())
		return -1;

	if (!theApp.createOutputProcess())
		return -1;

	m_view.Create(this);
	m_view.SetWindowPos(0, 0, 0, 1000, 600, SWP_NOZORDER | SWP_SHOWWINDOW);
	m_view.UpdateWindow();

	return 0;
}

void CMainFrame::OnDestroy()
{
	theApp.stop();

	if (m_rc)
		::wglDeleteContext(m_rc);

	CWnd::OnDestroy();
}

void CMainFrame::OnPaint()
{
	CPaintDC dc(this);
}

LRESULT CMainFrame::OnAtlivuInputInited(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnAtlivuInputInited()\n"));

	theApp.m_inputWindow = (HWND)wParam;

	CString iniPath = theApp.getIniPath();

	{
		TCHAR fileName[MAX_PATH] = {};
		::GetPrivateProfileString(_T("config"), _T("inputPlugin"), _T(""), fileName, MAX_PATH, iniPath);

		if (theApp.raw_loadInputPlugin(fileName))
		{
			TCHAR fileName[MAX_PATH] = {};
			::GetPrivateProfileString(_T("config"), _T("media"), _T(""), fileName, MAX_PATH, iniPath);

			theApp.openMedia(fileName);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnAtlivuOutputInited(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnAtlivuOutputInited()\n"));

	// 出力プロセスの初期化が終わったあとここにくる。

	// ウィンドウハンドルを取得する。
	theApp.m_outputWindow = (HWND)wParam;

	{
		// パイプの接続を確認する。

		BOOL result = ::ConnectNamedPipe(theApp.m_outputPipe, 0);
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

	CString iniPath = theApp.getIniPath();

	{
		// 前回使用していた出力プラグインを読み込む。

		TCHAR fileName[MAX_PATH] = {};
		::GetPrivateProfileString(_T("config"), _T("outputPlugin"), _T(""), fileName, MAX_PATH, iniPath);

		theApp.raw_loadOutputPlugin(fileName);
	}
#if 0
	// 起動してすぐ動画出力のテストをするなら
	theApp.save(_T("C:\\Users\\main\\source\\repos\\Atlivu\\_bin\\test.mp4"));
#endif
	return 0;
}

void CMainFrame::OnOpenMedia()
{
	MY_TRACE(_T("CMainFrame::OnOpenMedia()\n"));

	theApp.stop();

	CFileDialog dlg(TRUE, 0, 0, 0, _T("All files|*.*|"));
	if (dlg.DoModal() != IDOK)
		return;

	CString fileName = dlg.GetPathName();

	CString iniPath = theApp.getIniPath();
	::WritePrivateProfileString(_T("config"), _T("media"), fileName, iniPath);

	theApp.openMedia(fileName);
}

void CMainFrame::OnSaveMedia()
{
	MY_TRACE(_T("CMainFrame::OnSaveMedia()\n"));

	CFileDialog dlg(FALSE, 0, 0, OFN_OVERWRITEPROMPT, _T("mp4 files|*.mp4|"));
	if (dlg.DoModal() != IDOK)
		return;

	CString fileName = dlg.GetPathName();

	theApp.save(fileName);
}

void CMainFrame::OnAbortSaveMedia()
{
	MY_TRACE(_T("CMainFrame::OnAbortSaveMedia()\n"));

	theApp.abort();
}

void CMainFrame::OnSelectInputPlugin()
{
	theApp.stop();
	theApp.abort();

	CFileDialog dlg(TRUE, 0, 0, 0, _T("InputPlugin files|*.aui|"));
	if (dlg.DoModal() != IDOK)
		return;

	CString fileName = dlg.GetPathName();

	CString iniPath = theApp.getIniPath();
	::WritePrivateProfileString(_T("config"), _T("inputPlugin"), fileName, iniPath);

	theApp.closeMedia();

	theApp.raw_loadInputPlugin(fileName);
}

void CMainFrame::OnSetInputPluginConfig()
{
	theApp.raw_configInputPlugin();
}

void CMainFrame::OnSelectOutputPlugin()
{
	theApp.stop();
	theApp.abort();

	CFileDialog dlg(TRUE, 0, 0, 0, _T("OputputPlugin files|*.auo|"));
	if (dlg.DoModal() != IDOK)
		return;

	CString fileName = dlg.GetPathName();

	CString iniPath = theApp.getIniPath();
	::WritePrivateProfileString(_T("config"), _T("outputPlugin"), fileName, iniPath);

	theApp.raw_loadOutputPlugin(fileName);
}

void CMainFrame::OnSetOutputPluginConfig()
{
	theApp.raw_configOutputPlugin();
}

LRESULT CMainFrame::OnVideoProcessedSeek(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnVideoProcessedSeek(0x%08X, 0x%08X)\n"), wParam, lParam);

	theApp.OnVideoProcessedSeek((CVideoProcessor*)wParam);

	return 0;
}

LRESULT CMainFrame::OnVideoProcessedPlay(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnVideoProcessedPlay(0x%08X, 0x%08X)\n"), wParam, lParam);

	theApp.OnVideoProcessedPlay((CVideoProcessor*)wParam);

	return 0;
}

LRESULT CMainFrame::OnVideoProcessedPlayStop(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnVideoProcessedPlayStop(0x%08X, 0x%08X)\n"), wParam, lParam);

	theApp.OnVideoProcessedPlayStop();

	return 0;
}

LRESULT CMainFrame::OnIsAbort(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnIsAbort(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnIsAbort();
}

LRESULT CMainFrame::OnRestTimeDisp(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnRestTimeDisp(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnRestTimeDisp((int)wParam, (int)lParam);
}

LRESULT CMainFrame::OnUpdatePreview(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnUpdatePreview(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnUpdatePreview();
}

LRESULT CMainFrame::OnGetVideo(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnGetVideo(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnGetVideo();
}

LRESULT CMainFrame::OnGetAudio(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnGetAudio(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnGetAudio();
}

LRESULT CMainFrame::OnSaveFileFinished(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnSaveFileFinished(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnSaveFileFinished();
}

LRESULT CMainFrame::OnCreateOutputVideo(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnCreateOutputVideo(0x%08X, 0x%08X)\n"), wParam, lParam);

	return theApp.OnCreateOutputVideo((int)wParam);
}

//--------------------------------------------------------------------
