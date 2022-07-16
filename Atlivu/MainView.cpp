#include "pch.h"
#include "MainView.h"
#include "Atlivu.h"

const int BUTTON_W = 60;
const int BUTTON_H = 30;
const int WAVEFORM_H = 80;

BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_REGISTERED_MESSAGE(AFX_WM_DRAW2D, OnDraw2D)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

CMainView::CMainView()
{
	MY_TRACE(_T("CMainView::CMainView()\n"));

	EnableD2DSupport();

	m_waveformBrush = new CD2DSolidColorBrush(GetRenderTarget(), RGB(0x00, 0xff, 0x00));
}

CMainView::~CMainView()
{
	MY_TRACE(_T("CMainView::~CMainView()\n"));
}

void CMainView::OnOpenMedia()
{
	MY_TRACE(_T("CMainView::OnOpenMedia()\n"));

	// ビットマップを作成する。

	int w = theApp.m_mediaInfo.format.biWidth;
	int h = theApp.m_mediaInfo.format.biHeight;

	m_bitmap = 0;
	GetRenderTarget()->GetRenderTarget()->CreateBitmap(
		D2D1::SizeU(w, h), 
		D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)), 
		&m_bitmap);

	// コントロールの初期値を指定する。

	m_seekBar.SetRange(0, theApp.m_mediaInfo.n - 1);
//	m_seekBar.SetSelection(0, theApp.m_mediaInfo.n - 1);

	m_seekBar.SetPos(0);
	m_currentFrame.SetWindowText(_T("0"));

	theApp.seek(0);
}

void CMainView::OnVideoProcessedSeek(CVideoProcessor* videoProcessor)
{
	MY_TRACE(_T("CMainView::OnVideoProcessedSeek()\n"));

	MediaInfo* mi = &theApp.m_mediaInfo;
	int frame = theApp.m_currentFrame;

	{
		// ビデオプロセッサの映像バッファからビットマップを作成する。
		int w = mi->format.biWidth;
		int pitch = w * 4;
		m_bitmap->CopyFromMemory(0, videoProcessor->m_buffer.data(), pitch);
	}

	{
		// 音声データを取得する。

		int start = frame * mi->audio_format.nSamplesPerSec * mi->scale / mi->rate;
		int length = mi->audio_format.nSamplesPerSec * mi->scale / mi->rate;

		int sampleByteCount = mi->audio_format.wBitsPerSample / 8 * mi->audio_format.nChannels;
		int sampleCount = mi->audio_format.nAvgBytesPerSec / sampleByteCount;

		theApp.raw_readAudio(theApp.m_media, start, sampleCount, m_audioBuffer);
		ReadAudioOutput* output = (ReadAudioOutput*)m_audioBuffer.data();
		short* buffer = (short*)output->buffer;
		m_audioWidth = min(length, output->length);

		_AFX_D2D_STATE* pD2DState = AfxGetD2DState();
		ID2D1Factory* factory = pD2DState->GetDirect2dFactory();

		m_geometry = 0;
		factory->CreatePathGeometry(&m_geometry);
		CComPtr<ID2D1GeometrySink> sink = 0;
		m_geometry->Open(&sink);
		sink->BeginFigure(D2D1::Point2F(0, 0), D2D1_FIGURE_BEGIN_HOLLOW);

		if (mi->audio_format.nChannels == 2)
		{
			int div = 512;

			for (int i = 0; i < m_audioWidth; i++)
			{
				float x = (float)(i + 1);
				float y = (float)((buffer[i * 2] + buffer[i * 2 + 1]) / div);
				sink->AddLine(D2D1::Point2F(x, y));
			}
		}

		sink->EndFigure(D2D1_FIGURE_END_OPEN);
		sink->Close();
	}

	Invalidate(FALSE);
	UpdateWindow();
}

void CMainView::OnVideoProcessedPlay(CVideoProcessor* videoProcessor)
{
	MY_TRACE(_T("CMainView::OnVideoProcessedPlay()\n"));

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

BOOL CMainView::Create(CWnd* parent)
{
	MY_TRACE(_T("CMainView::Create(0x%p)\n"), parent);

	return CWnd::CreateEx(0, 0, 0, 0, CRect(0, 0, 0, 0), parent, 0, 0);
}

BOOL CMainView::PreCreateWindow(CREATESTRUCT& cs)
{
	MY_TRACE(_T("CMainView::PreCreateWindow()\n"));

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE;
	wc.hCursor = ::LoadCursor(0, IDC_CROSS);
	wc.lpfnWndProc = AfxWndProc;
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = _T("Atlivu.MainView");
	AfxRegisterClass(&wc);
	cs.lpszName = _T("Atlivu.MainView");
	cs.lpszClass = _T("Atlivu.MainView");
	cs.style = WS_VISIBLE | WS_CAPTION | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = 0;

	return CWnd::PreCreateWindow(cs);
}

BOOL CMainView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	HWND sender = (HWND)lParam;

	if (sender == m_play.GetSafeHwnd())
	{
		MY_TRACE(_T("CMainView::OnCommand(m_play)\n"));

		if (theApp.m_isPlaying)
			theApp.stop();
		else
			theApp.play();
	}

	return CWnd::OnCommand(wParam, lParam);
}

int CMainView::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("CMainView::OnCreate()\n"));

	if (CWnd::OnCreate(cs) == -1)
		return -1;

	m_play.Create(_T("再生"), WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 0);
	m_currentFrame.Create(WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 0);
	m_currentFrame.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_FRAMECHANGED);
	m_seekBar.Create(WS_VISIBLE | WS_CHILD | TBS_AUTOTICKS | TBS_ENABLESELRANGE, CRect(0, 0, 0, 0), this, 0);
//	m_seekBar.SetTic(60);
	m_seekBar.SetTicFreq(60);
	m_seekBar.SetPageSize(60);

	CFont font; font.CreateStockObject(DEFAULT_GUI_FONT);
	m_play.SetFont(&font);
	m_currentFrame.SetFont(&font);
	m_seekBar.SetFont(&font);

//	OnSize(0, 0, 0);

	return 0;
}

void CMainView::OnDestroy()
{
	MY_TRACE(_T("CMainView::OnDestroy()\n"));

	m_bitmap = 0;
	m_geometry = 0;

	CWnd::OnDestroy();
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

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

void CMainView::OnTimer(UINT_PTR timerId)
{
	switch (timerId)
	{
	case 0:
		break;
	}

	CWnd::OnTimer(timerId);
}

void CMainView::OnHScroll(UINT code, UINT pos, CScrollBar* scrollBar)
{
	switch (code)
	{
	case TB_ENDTRACK:
		{
			return;
		}
	}

	int frame = m_seekBar.GetPos();

	CString text; text.Format(_T("%d"), frame);
	m_currentFrame.SetWindowText(text);

	theApp.seek(frame);

//	CWnd::OnHScroll(code, pos, scrollBar);
}

LRESULT CMainView::OnDraw2D(WPARAM wParam, LPARAM lParam)
{
	CHwndRenderTarget* renderTarget = (CHwndRenderTarget*)lParam;

	renderTarget->Clear(D2D1::ColorF(RGB(0x33, 0x33, 0x33)));

	CRect rc; GetClientRect(&rc);
	rc.bottom -= BUTTON_H;

	D2D1_ANTIALIAS_MODE mode0 = renderTarget->GetAntialiasMode();
	D2D1_TEXT_ANTIALIAS_MODE mode1 = renderTarget->GetTextAntialiasMode();
//	CD2DSizeF dpi = renderTarget->GetDpi();
	renderTarget->SetDpi(CSize(96, 96));
//	renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
//	renderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

	renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
//	renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0.5f, 0.5f));

	CMainFrame& mainFrame = theApp.m_mainFrame;
	MediaInfo* mi = &theApp.m_mediaInfo;

	if (m_bitmap)
	{
		// 映像を描画する。

		CD2DSizeU srcSize = m_bitmap->GetPixelSize();
		CD2DRectF srcRect(0.0f, 0.0f, (float)srcSize.width, (float)srcSize.height);

		CRect dstRect = rc;
		dstRect.bottom -= WAVEFORM_H;
		int w = dstRect.Width();
		int h = dstRect.Height();
		int w2 = h * srcSize.width / srcSize.height;
		int h2 = w * srcSize.height / srcSize.width;

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
		
		renderTarget->GetRenderTarget()->DrawBitmap(m_bitmap, CD2DRectF(dstRect), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, &srcRect);
	}

	if (m_audioBuffer.data())
	{
		// 音声を描画する。

		CRect dstRect = rc;
		dstRect.top = rc.bottom - WAVEFORM_H;
		int cy = (dstRect.top + dstRect.bottom) / 2;

//		if (0)
		{
			CD2DSolidColorBrush* brush = new CD2DSolidColorBrush(renderTarget, RGB(0xff, 0x00, 0x00), 128);

			CPoint from(dstRect.left, cy);
			CPoint to(dstRect.right, cy);

			renderTarget->DrawLine(from, to, brush);
		}

		{
//			ReadAudioOutput* output = (ReadAudioOutput*)m_audioBuffer.data();
			float sx = (float)dstRect.Width() / (float)m_audioWidth;

//			renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			renderTarget->SetTransform(
				D2D1::Matrix3x2F::Scale(D2D1::Size(sx, 1.0f), D2D1::Point2F(0.0f, 0.0f)) *
				D2D1::Matrix3x2F::Translation(0.0f, (float)cy));

			renderTarget->GetRenderTarget()->DrawGeometry(m_geometry, m_waveformBrush->Get());
		}
	}

	return 0;
}
