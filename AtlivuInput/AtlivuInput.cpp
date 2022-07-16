#include "pch.h"
#include "AtlivuInput.h"
#include "MainFrame.h"
#include "Common/Tracer2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

InputManager* getInputManager()
{
	return theApp.m_inputManager.get();
}

CAtlivuInputApp theApp;

BEGIN_MESSAGE_MAP(CAtlivuInputApp, CWinApp)
END_MESSAGE_MAP()

CAtlivuInputApp::CAtlivuInputApp() noexcept
{
	trace_init(0, 0, TRUE);
}

CAtlivuInputApp::~CAtlivuInputApp()
{
	m_mediaMap.clear();

	trace_term();
}

BOOL CAtlivuInputApp::InitInstance()
{
	MY_TRACE(_T("CAtlivuInputApp::InitInstance()\n"));

	{
		m_hostProcessWindow = (HWND)_tcstoul(::GetCommandLine(), 0, 0);
		MY_TRACE_HWND(m_hostProcessWindow);

		if (!::IsWindow(m_hostProcessWindow))
			return FALSE;

		::GetWindowThreadProcessId(m_hostProcessWindow, &m_hostProcessId);
		MY_TRACE_INT(m_hostProcessId);

		m_hostProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_hostProcessId);
		MY_TRACE_HEX(m_hostProcess);
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction(FALSE);

	CFrameWnd* pFrame = new CMainFrame;
	if (!pFrame) return FALSE;
	m_pMainWnd = pFrame;
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		nullptr, nullptr);

//	pFrame->ShowWindow(SW_SHOW);
//	pFrame->UpdateWindow();

	::PostMessage(m_hostProcessWindow,
		WM_ATLIVU_INPUT_INITED, (WPARAM)pFrame->GetSafeHwnd(), 0);

	return TRUE;
}

int CAtlivuInputApp::ExitInstance()
{
	MY_TRACE(_T("CAtlivuInputApp::ExitInstance()\n"));

	AfxOleTerm(FALSE);

	return CWinApp::ExitInstance();
}
