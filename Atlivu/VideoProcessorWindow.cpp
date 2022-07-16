#include "pch.h"
#include "VideoProcessorWindow.h"
#include "VideoProcessor.h"
#include "Atlivu.h"

#define GET_OUTER(theClass, localClass) ((theClass*)((BYTE*)this - offsetof(theClass, localClass)))

BEGIN_MESSAGE_MAP(CVideoProcessorWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_VIDEO_PROCESS_SEEK, OnVideoProcessSeek)
	ON_MESSAGE(WM_VIDEO_PROCESS_PLAY, OnVideoProcessPlay)
END_MESSAGE_MAP()

CVideoProcessorWindow::CVideoProcessorWindow()
{
	MY_TRACE(_T("CVideoProcessorWindow::CVideoProcessorWindow()\n"));
}

CVideoProcessorWindow::~CVideoProcessorWindow()
{
	MY_TRACE(_T("CVideoProcessorWindow::~CVideoProcessorWindow()\n"));
}

BOOL CVideoProcessorWindow::Create(CWnd* parent)
{
	MY_TRACE(_T("CVideoProcessorWindow::Create(0x%p)\n"), parent);

	return CWnd::CreateEx(0, 0, 0, 0, CRect(0, 0, 0, 0), parent, 0, 0);
}

BOOL CVideoProcessorWindow::PreCreateWindow(CREATESTRUCT& cs)
{
	MY_TRACE(_T("CVideoProcessorWindow::PreCreateWindow()\n"));

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_NOCLOSE;
	wc.hCursor = ::LoadCursor(0, IDC_CROSS);
	wc.lpfnWndProc = AfxWndProc;
	wc.hInstance = AfxGetInstanceHandle();
	wc.lpszClassName = _T("Atlivu.VideoProcessorWindow");
	AfxRegisterClass(&wc);
	cs.lpszName = _T("Atlivu.VideoProcessorWindow");
	cs.lpszClass = _T("Atlivu.VideoProcessorWindow");
	cs.style = WS_CAPTION | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	cs.dwExStyle = 0;

	return CWnd::PreCreateWindow(cs);
}

int CVideoProcessorWindow::OnCreate(LPCREATESTRUCT cs)
{
	MY_TRACE(_T("CVideoProcessorWindow::OnCreate()\n"));

	if (CWnd::OnCreate(cs) == -1)
		return -1;

	return 0;
}

void CVideoProcessorWindow::OnDestroy()
{
	MY_TRACE(_T("CVideoProcessorWindow::OnDestroy()\n"));

	CWnd::OnDestroy();
}

LRESULT CVideoProcessorWindow::OnVideoProcessSeek(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CVideoProcessorWindow::OnVideoProcessSeek(0x%08X, 0x%08X)\n"), wParam, lParam);

	CVideoProcessor* videoProcessor = GET_OUTER(CVideoProcessor, m_mainFrame);

	return videoProcessor->OnVideoProcessSeek(wParam, lParam);
}

LRESULT CVideoProcessorWindow::OnVideoProcessPlay(WPARAM wParam, LPARAM lParam)
{
//	MY_TRACE(_T("CVideoProcessorWindow::OnVideoProcessPlay(0x%08X, 0x%08X)\n"), wParam, lParam);

	CVideoProcessor* videoProcessor = GET_OUTER(CVideoProcessor, m_mainFrame);

	return videoProcessor->OnVideoProcessPlay(wParam, lParam);
}
