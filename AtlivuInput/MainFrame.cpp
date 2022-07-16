#include "pch.h"
#include "AtlivuInput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const UINT TIMER_ID = 2022;

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_COPYDATA()
END_MESSAGE_MAP()

CMainFrame::CMainFrame() noexcept
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	MY_TRACE(_T("CMainFrame::OnCreate()\n"));

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(TIMER_ID, 1000, 0);

	return 0;
}

void CMainFrame::OnDestroy()
{
	MY_TRACE(_T("CMainFrame::OnDestroy()\n"));

	KillTimer(TIMER_ID);

	CFrameWnd::OnDestroy();
}

void CMainFrame::OnTimer(UINT_PTR timerId)
{
	if (timerId == TIMER_ID)
	{
		if (!::IsWindow(theApp.m_hostProcessWindow))
		{
			KillTimer(TIMER_ID);
			PostMessage(WM_CLOSE);
		}
	}

	CFrameWnd::OnTimer(timerId);
}

BOOL CMainFrame::OnCopyData(CWnd* wnd, COPYDATASTRUCT* cds)
{
	MY_TRACE(_T("CMainFrame::OnCopyData()\n"));
	MY_TRACE_INT(cds->dwData);
	MY_TRACE_INT(cds->cbData);

	switch (cds->dwData)
	{
	case WM_LOAD_INPUT_PLUGIN:
		{
			MY_TRACE(_T("WM_LOAD_INPUT_PLUGIN\n"));

			LPCTSTR fileName = (LPCTSTR)cds->lpData;
			MY_TRACE_TSTR(fileName);

			try
			{
				theApp.m_mediaMap.clear();

				theApp.m_inputManager.reset(new InputManager(fileName));
			}
			catch (LPCTSTR e)
			{
				MY_TRACE_TSTR(e);

				return FALSE;
			}

			return TRUE;
		}
	case WM_UNLOAD_INPUT_PLUGIN:
		{
			MY_TRACE(_T("WM_UNLOAD_INPUT_PLUGIN\n"));

			try
			{
				theApp.m_inputManager = 0;
			}
			catch (LPCTSTR e)
			{
				MY_TRACE_TSTR(e);

				return FALSE;
			}

			return TRUE;
		}
	case WM_CONFIG_INPUT_PLUGIN:
		{
			MY_TRACE(_T("WM_CONFIG_INPUT_PLUGIN\n"));

			if (!theApp.m_inputManager)
				return FALSE;

			if (!theApp.m_inputManager->getInputPlugin()->func_config)
				return FALSE;

			return theApp.m_inputManager->getInputPlugin()->func_config(AfxGetMainWnd()->GetSafeHwnd(), theApp.m_inputManager->m_aui);
		}
	case WM_OPEN_MEDIA:
		{
			MY_TRACE(_T("WM_OPEN_MEDIA\n"));

			LPCTSTR fileName = (LPCTSTR)cds->lpData;
			MY_TRACE_TSTR(fileName);

			try
			{
				MediaPtr media(new Media(fileName));

				theApp.m_mediaMap[media.get()] = media;

				return (BOOL)media.get();
			}
			catch (LPCTSTR e)
			{
				MY_TRACE(_T("例外が発生しました : %s\n"), e);
			}

			return 0;
		}
	case WM_CLOSE_MEDIA:
		{
			MY_TRACE(_T("WM_CLOSE_MEDIA\n"));

			Media* media = *(Media**)cds->lpData;

			return (BOOL)theApp.m_mediaMap.erase(media);
		}
	case WM_GET_MEDIA_INFO:
		{
			MY_TRACE(_T("WM_GET_MEDIA_INFO\n"));

			Media* media = *(Media**)cds->lpData;

			if (theApp.m_mediaMap.find(media) == theApp.m_mediaMap.end())
				return FALSE;

			return (BOOL)media->getMediaInfo();
		}
	case WM_READ_VIDEO:
		{
			MY_TRACE(_T("WM_READ_VIDEO\n"));

			ReadVideoInput* input = (ReadVideoInput*)cds->lpData;

			Media* media = (Media*)input->media;

			if (theApp.m_mediaMap.find(media) == theApp.m_mediaMap.end())
				return FALSE;

			return (BOOL)media->readVideo(input);
		}
	case WM_READ_AUDIO:
		{
			MY_TRACE(_T("WM_READ_AUDIO\n"));

			ReadAudioInput* input = (ReadAudioInput*)cds->lpData;

			Media* media = (Media*)input->media;

			if (theApp.m_mediaMap.find(media) == theApp.m_mediaMap.end())
				return FALSE;

			return (BOOL)media->readAudio(input);
		}
	}

	return CFrameWnd::OnCopyData(wnd, cds);
}
