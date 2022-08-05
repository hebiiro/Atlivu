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
	ON_WM_SIZE()
	ON_WM_HSCROLL()
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
END_MESSAGE_MAP()

CMainFrame::CMainFrame() noexcept
{
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

void CMainFrame::OnOpenMedia(MediaInfo* mediaInfo)
{
	MY_TRACE(_T("CMainFrame::OnOpenMedia()\n"));

	{
		// ビットマップを作成する。

		m_pixelWidth = mediaInfo->format.biWidth;
		m_pixelHeight = mediaInfo->format.biHeight;

		CClientDC dc(this);
		MakeCurrent makeCurrent(dc, m_rc);

		glBindTexture(GL_TEXTURE_2D, m_videoTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pixelWidth, m_pixelHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		GLenum error = ::glGetError();
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	{
		// コントロールの初期値を指定する。

		m_seekBar.SetRange(0, mediaInfo->n - 1);
//		m_seekBar.SetSelection(0, mediaInfo->n - 1);

		m_seekBar.SetPos(0);
		m_currentFrame.SetWindowText(_T("0"));
	}

	theApp.seek(0);
}

void CMainFrame::OnVideoProcessedSeek(CVideoProcessor* videoProcessor)
{
	MY_TRACE(_T("CMainFrame::OnVideoProcessedSeek()\n"));

	MediaInfo* mi = &theApp.m_mediaInfo;
	int frame = theApp.m_currentFrame;

	{
		CClientDC dc(this);
		MakeCurrent makeCurrent(dc, m_rc);

		{
			// ビデオプロセッサの映像バッファをテクスチャに転送する。

			glBindTexture(GL_TEXTURE_2D, m_videoTextureID);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_pixelWidth, m_pixelHeight, GL_RGBA, GL_UNSIGNED_BYTE, videoProcessor->m_buffer.data());
			GLenum error = ::glGetError();
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		{
			// 音声波形のディスプレイリストを作成する。

			int start = frame * mi->audio_format.nSamplesPerSec * mi->scale / mi->rate;
			int length = mi->audio_format.nSamplesPerSec * mi->scale / mi->rate;

			int BytesPerBlock = mi->audio_format.wBitsPerSample / 8 * mi->audio_format.nChannels;
			int sampleCount = mi->audio_format.nAvgBytesPerSec / BytesPerBlock;

			int32_t bufferLength = 0;
			theApp.raw_readAudio(theApp.m_media, start, sampleCount, &bufferLength, m_audioBuffer);
			short* buffer = (short*)m_audioBuffer.data();
			m_audioWidth = min(length, bufferLength);

			glNewList(m_audioDisplayList, GL_COMPILE);
			glBegin(GL_LINE_STRIP);

			if (mi->audio_format.nChannels == 2)
			{
				int div = 512;

				for (int i = 0; i < m_audioWidth; i++)
				{
					float x = (float)(i);
					float y = (float)((buffer[i * 2] + buffer[i * 2 + 1]) / div);
					glVertex2f(x, y);
				}
			}

			glEnd();
			glEndList();
		}
	}

	Invalidate(FALSE);
	UpdateWindow();
}

void CMainFrame::OnVideoProcessedPlay(CVideoProcessor* videoProcessor)
{
	MY_TRACE(_T("CMainFrame::OnVideoProcessedPlay()\n"));

	MediaInfo* mi = &theApp.m_mediaInfo;
	int frame = theApp.m_currentFrame;

	// コントロールを更新する。
	m_seekBar.SetPos(frame);
	CString text; text.Format(_T("%d"), frame);
	m_currentFrame.SetWindowText(text);

	// キャプションを更新する。
	text.Format(_T("skip=%d"), theApp.m_totalSkipCount);
	SetWindowText(text);

	OnVideoProcessedSeek(videoProcessor);
}

//--------------------------------------------------------------------

BOOL CMainFrame::Create()
{
	MY_TRACE(_T("CMainFrame::Create()\n"));

	if (!CWnd::CreateEx(0, 0, 0, 0, CRect(0, 0, 1000, 800), 0, 0))
		return FALSE;

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	MY_TRACE(_T("CMainFrame::PreCreateWindow()\n"));

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
	cs.style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = 0;

	CMenu menu; menu.LoadMenu(IDR_MAIN_VIEW);
	cs.hMenu = menu.Detach();

	return CWnd::PreCreateWindow(cs);
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HWND sender = (HWND)lParam;

	if (sender == m_play.GetSafeHwnd())
	{
		MY_TRACE(_T("CMainFrame::OnCommand(m_play)\n"));

		if (theApp.m_isPlaying)
			theApp.stop();
		else
			theApp.play();
	}

	return CWnd::OnCommand(wParam, lParam);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!theApp.createInputProcess())
		return -1;

	if (!theApp.createOutputProcess())
		return -1;

	{
		// コントロールを作成する。

		m_play.Create(_T("再生"), WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 0);
		m_currentFrame.Create(WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 0);
		m_currentFrame.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
		m_seekBar.Create(WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS | TBS_ENABLESELRANGE, CRect(0, 0, 0, 0), this, 0);
//		m_seekBar.SetTic(60);
		m_seekBar.SetTicFreq(60);
		m_seekBar.SetPageSize(60);

		CFont font; font.CreateStockObject(DEFAULT_GUI_FONT);
		m_play.SetFont(&font);
		m_currentFrame.SetFont(&font);
		m_seekBar.SetFont(&font);
	}

	{
		// OpenGL を初期化する。

		CClientDC dc(this);

		setupPixelFormat(dc);

		m_rc = ::wglCreateContext(dc);
		MY_TRACE_HEX(m_rc);

		if (!m_rc)
		{
			MY_TRACE(_T("::wglCreateContext() が失敗しました\n"));

			return -1;
		}

		MakeCurrent makeCurrent(dc, m_rc);

		glGenTextures(1, &m_videoTextureID);

		glBindTexture(GL_TEXTURE_2D, m_videoTextureID);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		// テクスチャを拡大縮小する時のフィルタリング方法を指定
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		//ラッピング方法を指定
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		// テクスチャのアンバインド
		glBindTexture(GL_TEXTURE_2D, 0);

		m_audioDisplayList = glGenLists(1);
	}

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
	MakeCurrent makeCurrent(dc, m_rc);

	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CRect rc; GetClientRect(&rc);
	rc.bottom -= BUTTON_H;
	int width = rc.Width();
	int height = rc.Height();

	glViewport(0, BUTTON_H, width, height + 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-0.5, width-0.5, height-0.5, -0.5, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (m_pixelWidth && m_pixelHeight)
	{
		// 映像を描画する。

		CRect dstRect = rc;
		dstRect.bottom -= WAVEFORM_H;
		int x = dstRect.left;
		int y = dstRect.top;
		int w = dstRect.Width();
		int h = dstRect.Height();
		int w2 = h * m_pixelWidth / m_pixelHeight;
		int h2 = w * m_pixelHeight / m_pixelWidth;

		if (w2 > w) // srcSize の横幅が大きすぎるなら
		{
			// w はそのまま。top と bottom を調整する。

			dstRect.top = (dstRect.top + dstRect.bottom - h2) / 2;
			dstRect.bottom = dstRect.top + h2;
		}
		else // srcSize の縦幅が大きすぎるなら
		{
			// h はそのまま。left と right を調整する。

			dstRect.left = (dstRect.left + dstRect.right - w2) / 2;
			dstRect.right = dstRect.left + w2;
		}

		x = dstRect.left;
		y = dstRect.top;
		w = dstRect.Width();
		h = dstRect.Height();

		glBindTexture(GL_TEXTURE_2D, m_videoTextureID);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		glTexCoord2d(0.0, 0.0); glVertex2i(x, y);
		glTexCoord2d(0.0, 1.0); glVertex2i(x, y + h);
		glTexCoord2d(1.0, 1.0); glVertex2i(x + w, y + h);
		glTexCoord2d(1.0, 0.0); glVertex2i(x + w, y);
		glEnd();
  
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (m_audioBuffer.data())
	{
		// 音声を描画する。

		CRect dstRect = rc;
		dstRect.top = rc.bottom - WAVEFORM_H;
		int cy = (dstRect.top + dstRect.bottom) / 2;

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.5f);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

//		if (0)
		{
			glColor4ub(0xff, 0x00, 0x00, 128);
			glBegin(GL_LINES);
			glVertex2i(dstRect.left, cy);
			glVertex2i(dstRect.right, cy);
			glEnd();
		}

//		if (0)
		{
			float sx = (float)dstRect.Width() / (float)m_audioWidth;

			glPushMatrix();

			glScalef(sx, 1.0f, 1.0f);
			glTranslatef(0.0f, (float)cy, 0.0f);

			glColor4ub(0x00, 0xff, 0x00, 255);
			glCallList(m_audioDisplayList);

			glPopMatrix();
		}

		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);
	}

	::SwapBuffers(dc);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// コントロールの位置を更新する。

	CRect rc; GetClientRect(&rc);

	{
		CRect rc2;
		rc2.left = rc.right - BUTTON_W;
		rc2.top = rc.bottom - BUTTON_H;
		rc2.right = rc.right;
		rc2.bottom = rc.bottom;
		m_play.SetWindowPos(0, rc2.left, rc2.top, rc2.Width(), rc2.Height(), SWP_NOZORDER);
	}

	{
		CRect rc2;
		rc2.left = rc.left;
		rc2.top = rc.bottom - BUTTON_H;
		rc2.right = rc.left + BUTTON_W;
		rc2.bottom = rc.bottom;
		m_currentFrame.SetWindowPos(0, rc2.left, rc2.top, rc2.Width(), rc2.Height(), SWP_NOZORDER);
	}

	{
		CRect rc2;
		rc2.left = rc.left + BUTTON_W;
		rc2.top = rc.bottom - BUTTON_H;
		rc2.right = rc.right - BUTTON_W;
		rc2.bottom = rc.bottom;
		m_seekBar.SetWindowPos(0, rc2.left, rc2.top, rc2.Width(), rc2.Height(), SWP_NOZORDER);
	}
}

void CMainFrame::OnHScroll(UINT code, UINT pos, CScrollBar* scrollBar)
{
	switch (code)
	{
	case TB_ENDTRACK:
		{
			return;
		}
	}

	// シークバーが変動したのでシークする。

	int frame = m_seekBar.GetPos();

	CString text; text.Format(_T("%d"), frame);
	m_currentFrame.SetWindowText(text);

	theApp.seek(frame);

//	CWnd::OnHScroll(code, pos, scrollBar);
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

//--------------------------------------------------------------------
