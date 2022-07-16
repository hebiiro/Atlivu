#pragma once

#include "InputManager.h"

//--------------------------------------------------------------------

class CMainFrame : public CFrameWnd
{
protected: 

	DECLARE_DYNAMIC(CMainFrame)

public:

	CMainFrame() noexcept;
	virtual ~CMainFrame();

protected:

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------
