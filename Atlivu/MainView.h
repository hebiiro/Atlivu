#pragma once

#include "VideoProcessor.h"

class CMainView : public CWnd
{
public:

	CButton m_play;
	CEdit m_currentFrame;
	CSliderCtrl m_seekBar;

	std::vector<BYTE> m_videoBuffer;
	CComPtr<ID2D1Bitmap> m_bitmap;

	std::vector<BYTE> m_audioBuffer;
	CD2DSolidColorBrush* m_waveformBrush = 0;
	CComPtr<ID2D1PathGeometry> m_geometry;
	int m_audioWidth = 0;

public:

	CMainView();
	virtual ~CMainView();

	void OnOpenMedia();
	void OnVideoProcessedSeek(CVideoProcessor* videoProcessor);
	void OnVideoProcessedPlay(CVideoProcessor* videoProcessor);

public:

	virtual BOOL Create(CWnd* parent);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT cs);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnDraw2D(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
