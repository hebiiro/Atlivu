#pragma once

#include "VideoProcessor.h"

//--------------------------------------------------------------------

class MakeCurrent
{
public:

	MakeCurrent(HDC dc, HGLRC rc)
	{
		::wglMakeCurrent(dc, rc);
	}

	~MakeCurrent()
	{
		::wglMakeCurrent(0, 0);
	}
};

//--------------------------------------------------------------------

class CMainFrame : public CWnd
{
public:

	static const int BUTTON_W = 60;
	static const int BUTTON_H = 30;
	static const int WAVEFORM_H = 80;

	CButton m_play;
	CEdit m_currentFrame;
	CSliderCtrl m_seekBar;

	std::vector<BYTE> m_audioBuffer;
	int m_audioWidth = 0;

	HGLRC m_rc = 0;
	GLuint m_videoTextureID = 0;
	GLuint m_audioDisplayList = 0;
	int m_pixelWidth = 0;
	int m_pixelHeight = 0;

public:

	CMainFrame() noexcept;
	virtual ~CMainFrame();

	BOOL setupPixelFormat(CDC& dc);

	void OnOpenMedia(MediaInfo* mediaInfo);
	void OnVideoProcessedSeek(CVideoProcessor* videoProcessor);
	void OnVideoProcessedPlay(CVideoProcessor* videoProcessor);

	virtual BOOL Create();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

protected:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
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
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------
