#pragma once

#include "MainView.h"

//--------------------------------------------------------------------

typedef std::shared_ptr<CD2DTextFormat> CD2DTextFormatPtr;
typedef std::shared_ptr<CD2DSolidColorBrush> CD2DSolidColorBrushPtr;
typedef std::shared_ptr<CD2DLinearGradientBrush> CD2DLinearGradientBrushPtr;

//--------------------------------------------------------------------

class CMainFrame : public CWnd
{
public:

	int m_headingWidth;
	int m_headingHeight;
	int m_shortGuageHeight;
	int m_longGuageHeight;
	int m_zoom;
	int m_layerHeight;

	CMainView m_view;

public:

	HGLRC m_rc = 0;

	BOOL setupPixelFormat(CDC& dc);

public:

	CMainFrame() noexcept;
	virtual ~CMainFrame();

	HANDLE getRetValue()
	{
		return ::GetProp(GetSafeHwnd(), PROP_NAME_RET_VALUE);
	}

	virtual BOOL Create();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg LRESULT OnAtlivuInputInited(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAtlivuOutputInited(WPARAM wParam, LPARAM lParam);
	afx_msg void OnOpenMedia();
	afx_msg void OnSaveMedia();
	afx_msg void OnAbortSaveMedia();
	afx_msg void OnSelectInputPlugin();
	afx_msg void OnSetInputPluginConfig();
	afx_msg void OnSelectOutputPlugin();
	afx_msg void OnSetOutputPluginConfig();
	afx_msg LRESULT OnVideoProcessedSeek(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnVideoProcessedPlay(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnVideoProcessedPlayStop(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIsAbort(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRestTimeDisp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdatePreview(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetVideo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGetAudio(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSaveFileFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCreateOutputVideo(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------
