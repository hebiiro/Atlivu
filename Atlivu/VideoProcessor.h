#pragma once

#include "Resource.h"
#include "VideoProcessorWindow.h"

//--------------------------------------------------------------------

class CVideoProcessor : public CWinThread
{
public:

	CVideoProcessorWindow m_mainFrame;

	BOOL m_isSeeking = FALSE;
	std::vector<BYTE> m_rawBuffer;
	std::vector<BYTE> m_buffer;

public:

	CVideoProcessor() noexcept;
	virtual ~CVideoProcessor();

	void getRawBuffer();
	void getBuffer();

	afx_msg LRESULT OnVideoProcessSeek(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnVideoProcessPlay(WPARAM wParam, LPARAM lParam);

	LRESULT sendMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::SendMessage(m_mainFrame, message, wParam, lParam);
	}

	BOOL postMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ::PostMessage(m_mainFrame, message, wParam, lParam);
	}

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	DECLARE_MESSAGE_MAP()
};

//--------------------------------------------------------------------
