#pragma once

class CVideoProcessorWindow : public CWnd
{
public:

public:

	CVideoProcessorWindow();
	virtual ~CVideoProcessorWindow();

public:

	virtual BOOL Create(CWnd* parent);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnVideoProcessSeek(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnVideoProcessPlay(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
