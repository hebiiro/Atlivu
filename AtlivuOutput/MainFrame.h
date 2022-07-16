#pragma once

#include "OutputManager.h"

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
	afx_msg LRESULT OnLoadOutputPlugin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUnloadOutputPlugin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnConfigOutputPlugin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSaveFile(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------
