#include "pch.h"
#include "VideoProcessor.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CVideoProcessor, CWinThread)
END_MESSAGE_MAP()

CVideoProcessor::CVideoProcessor() noexcept
{
	m_bAutoDelete = FALSE;
}

CVideoProcessor::~CVideoProcessor()
{
}

void CVideoProcessor::getRawBuffer()
{
	// 生のビデオバッファを取得する。

	theApp.raw_readVideo(theApp.m_media, theApp.m_currentFrame, m_rawBuffer);
}

void CVideoProcessor::getBuffer()
{
	// RGB ビデオバッファを作成する。

	ReadVideoOutput* output = (ReadVideoOutput*)m_rawBuffer.data();
	m_buffer.resize(theApp.getRGBBufferSize());
	theApp.getRGBBuffer(output->buffer, m_buffer.data());
}

LRESULT CVideoProcessor::OnVideoProcessSeek(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CVideoProcessor::OnVideoProcessSeek(0x%08X, 0x%08X)\n"), wParam, lParam);

	m_isLocked = FALSE;

	getRawBuffer();
	getBuffer();

	theApp.sendMessage(WM_VIDEO_PROCESSED_SEEK, (WPARAM)this, 0);

	return 0;
}

LRESULT CVideoProcessor::OnVideoProcessPlay(WPARAM wParam, LPARAM lParam)
{
	MY_TRACE(_T("CVideoProcessor::OnVideoProcessPlay(0x%08X, 0x%08X)\n"), wParam, lParam);

	theApp.m_isProcessing = FALSE;

	if (!theApp.m_isPlaying)
		return 0;

	getRawBuffer();
	getBuffer();

	theApp.sendMessage(WM_VIDEO_PROCESSED_PLAY, (WPARAM)this, 0);

	return 0;
}

BOOL CVideoProcessor::InitInstance()
{
	CWinThread::InitInstance();

	m_pMainWnd = &m_mainFrame;
	m_mainFrame.Create(0);

	return TRUE;
}

int CVideoProcessor::ExitInstance()
{
	return CWinThread::ExitInstance();
}

//--------------------------------------------------------------------
