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
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, &CMainFrame::OnDraw2D)
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

	CWnd::OnDestroy();
}

LRESULT CMainFrame::OnDraw2D(WPARAM wParam, LPARAM lParam)
{
	CHwndRenderTarget* renderTarget = (CHwndRenderTarget*)lParam;

	CRect rc; GetClientRect(&rc);

	D2D1_ANTIALIAS_MODE mode0 = renderTarget->GetAntialiasMode();
	D2D1_TEXT_ANTIALIAS_MODE mode1 = renderTarget->GetTextAntialiasMode();
//	CD2DSizeF dpi = renderTarget->GetDpi();
//	renderTarget->SetDpi(CSize(96, 96));
//	renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
//	renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

	renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
//	renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0.5f, 0.5f));

	CD2DLayer layer(renderTarget);
	renderTarget->PushLayer(D2D1::LayerParameters(CD2DRectF(rc)), layer);

	{
		CD2DSolidColorBrush brush(renderTarget, RGB(0x33, 0x33, 0x33));
		renderTarget->FillRectangle(rc, &brush);
	}

	double zoom = 1.0;
	int guageWidth = 150;
	int guageHeight = 40;
	int shortGuageHeight = 10;
	int longGuageHeight = 20;
	int layerButtonWidth = 100;
	int layerButtonHeight = 40;

	CD2DTextFormat textFormat(renderTarget, _T("Segoe UI"), (FLOAT)(guageHeight - longGuageHeight));
	CD2DSolidColorBrush textBrush(renderTarget, RGB(0xff, 0xff, 0xff));

	{
		CRect rcBase = rc;
		rcBase.left += layerButtonWidth;
		rcBase.bottom = rcBase.top + guageHeight;
		int baseWidth = rcBase.Width();
		int baseHeight = rcBase.Height();

		CD2DSolidColorBrush brush(renderTarget, RGB(0xff, 0xff, 0xff));
		renderTarget->DrawLine(CPoint(rcBase.left, rcBase.bottom), CPoint(rcBase.right, rcBase.bottom), &brush);

		textFormat.Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		textFormat.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		for (int i = 0; i < baseWidth / guageWidth + 1; i++)
		{
			double time = i / zoom;
			int hour = (int)time / 60 / 60;
			int min = (int)time / 60 % 60;
			double sec = fmod(time, 60);

			int mx = rcBase.left + i * guageWidth;
			int my = rcBase.bottom - longGuageHeight;
			int lx = mx;
			int ly = rcBase.bottom;
			renderTarget->DrawLine(CPoint(mx, my), CPoint(lx, ly), &brush);

			CString text; text.Format(_T("%02d:%02d:%06.3f"), hour, min, sec);
			CRect rcText(mx, rc.top, mx + guageWidth, my);
			renderTarget->DrawText(text, rcText, &textBrush, &textFormat);
//			renderTarget->DrawRectangle(rcText, &brush);

			for (int j = 1; j < 10; j++)
			{
				int mx = rcBase.left + i * guageWidth + (guageWidth / 10 * j);
				int my = rcBase.bottom - shortGuageHeight;
				int lx = mx;
				int ly = rcBase.bottom;
				renderTarget->DrawLine(CPoint(mx, my), CPoint(lx, ly), &brush);
			}
		}
	}

	{
		CRect rcBase = rc;
		rcBase.right = rcBase.left + layerButtonWidth;
		rcBase.top = rcBase.top + guageHeight;
		int baseWidth = rcBase.Width();
		int baseHeight = rcBase.Height();

		CD2DSolidColorBrush fillBrush(renderTarget, RGB(0x44, 0x44, 0x44));
		CD2DSolidColorBrush edgeBrush(renderTarget, RGB(0x99, 0x99, 0x99));

		textFormat.Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textFormat.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		for (int i = 0; i < baseHeight / layerButtonHeight + 1; i++)
		{
			CRect rc;
			rc.left = rcBase.left;
			rc.right = rcBase.right;
			rc.top = rcBase.top + i * layerButtonHeight;
			rc.bottom = rc.top + layerButtonHeight;

			renderTarget->FillRectangle(rc, &fillBrush);
			renderTarget->DrawRectangle(rc, &edgeBrush);

			CString text; text.Format(_T("Layer %02d"), i + 1);
			renderTarget->DrawText(text, rc, &textBrush, &textFormat);
		}
	}

	{
		CRect rcBase = rc;
		rcBase.left = rcBase.left + layerButtonWidth;
		rcBase.top = rcBase.top + guageHeight;
		int baseWidth = rcBase.Width();
		int baseHeight = rcBase.Height();

		CD2DSolidColorBrush brush(renderTarget, RGB(0x66, 0x66, 0x66));

		for (int i = 1; i < baseHeight / layerButtonHeight + 1; i++)
		{
			int mx = rcBase.left;
			int my = rcBase.top + i * layerButtonHeight;
			int lx = rcBase.right;
			int ly = my;
			renderTarget->DrawLine(CPoint(mx, my), CPoint(lx, ly), &brush);
		}
	}

	{
		CRect rcBase = rc;
		rcBase.right = rcBase.left + layerButtonWidth;
		rcBase.bottom = rcBase.top + guageHeight;
		int baseWidth = rcBase.Width();
		int baseHeight = rcBase.Height();

		CD2DSolidColorBrush fillBrush(renderTarget, RGB(0x99, 0x00, 0x00));
		CD2DSolidColorBrush edgeBrush(renderTarget, RGB(0x99, 0x99, 0x99));

		textFormat.Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textFormat.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		{
			renderTarget->FillRectangle(rcBase, &fillBrush);
			renderTarget->DrawRectangle(rcBase, &edgeBrush);

			CString text; text.Format(_T("Root"));
			renderTarget->DrawText(text, rcBase, &textBrush, &textFormat);
		}
	}

	{
		CRect rcBase = rc;
		rcBase.left = rcBase.left + layerButtonWidth;
		rcBase.top = rcBase.top + guageHeight;
		rcBase.right = rcBase.left + guageWidth * 2;
		rcBase.bottom = rcBase.top + layerButtonHeight;
		int baseWidth = rcBase.Width();
		int baseHeight = rcBase.Height();

		CD2DSolidColorBrush fillBrush(renderTarget, RGB(0x00, 0x99, 0xff));
		CD2DSolidColorBrush edgeBrush(renderTarget, RGB(0xff, 0xff, 0xff));

		textFormat.Get()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textFormat.Get()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		{
			renderTarget->FillRoundedRectangle(CD2DRoundedRect(rcBase, CSize(10, 10)), &fillBrush);
			renderTarget->DrawRoundedRectangle(CD2DRoundedRect(rcBase, CSize(10, 10)), &edgeBrush);

			CString text; text.Format(_T("描画のテスト"));
			renderTarget->DrawText(text, rcBase, &textBrush, &textFormat);
		}
	}

	renderTarget->PopLayer();

	return TRUE;
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
