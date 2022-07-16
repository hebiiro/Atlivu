#include "pch.h"
#include "AtlivuOutput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const UINT TIMER_ID = 2022;

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE(WM_LOAD_OUTPUT_PLUGIN, OnLoadOutputPlugin)
	ON_MESSAGE(WM_UNLOAD_OUTPUT_PLUGIN, OnUnloadOutputPlugin)
	ON_MESSAGE(WM_CONFIG_OUTPUT_PLUGIN, OnConfigOutputPlugin)
	ON_MESSAGE(WM_SAVE_FILE, OnSaveFile)
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

LRESULT CMainFrame::OnLoadOutputPlugin(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnLoadOutputPlugin()\n"));

	LoadOutputPluginInput input = {};
	DWORD read1 = readFile(theApp.m_pipe, &input, sizeof(input));
	MY_TRACE_INT(read1);

	LPCTSTR fileName = input.fileName;
	MY_TRACE_TSTR(fileName);

	try
	{
		theApp.m_outputManager.reset(new OutputManager(fileName));
	}
	catch (LPCTSTR e)
	{
		MY_TRACE_TSTR(e);

		return FALSE;
	}

	return TRUE;
}

LRESULT CMainFrame::OnUnloadOutputPlugin(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnUnloadOutputPlugin()\n"));

	try
	{
		theApp.m_outputManager = 0;
	}
	catch (LPCTSTR e)
	{
		MY_TRACE_TSTR(e);

		return FALSE;
	}

	return TRUE;
}

LRESULT CMainFrame::OnConfigOutputPlugin(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnConfigOutputPlugin()\n"));

	if (!theApp.m_outputManager)
		return FALSE;

	return theApp.m_outputManager->config(theApp.m_hostProcessWindow);
}

LRESULT CMainFrame::OnSaveFile(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CMainFrame::OnSaveFile()\n"));

	SaveFileInput input = {};
	DWORD read1 = readFile(theApp.m_pipe, &input, sizeof(input));
	MY_TRACE_INT(read1);

	return theApp.save(input.fileName, &input.mediaInfo);
}
