#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

inline BYTE clip(int n)
{
	if (n < 0) return 0;
	if (n > 255) return 255;
	return (BYTE)n;
}

inline BYTE* YUV2ToRGB_BT601_32bit_16bit(BYTE* dst, BYTE Y, BYTE U, BYTE V)
{
	int C = Y - 16;
	int D = U - 128;
	int E = V - 128;

	dst[0] = clip((76309 * C              + 104597 * E + 32768) >> 16);
	dst[1] = clip((76309 * C -  25675 * D -  53279 * E + 32768) >> 16);
	dst[2] = clip((76309 * C + 132201 * D              + 32768) >> 16);
	dst[3] = 0xff;

	return dst + 4;
}

inline BYTE* YUV2ToRGB_BT601_32bit_8bit(BYTE* dst, BYTE Y, BYTE U, BYTE V)
{
	int C = Y - 16;
	int D = U - 128;
	int E = V - 128;

	dst[0] = clip((298 * C           + 409 * E + 128) >> 8);
	dst[1] = clip((298 * C - 100 * D - 208 * E + 128) >> 8);
	dst[2] = clip((298 * C + 516 * D           + 128) >> 8);
	dst[3] = 0xff;

	return dst + 4;
}

//--------------------------------------------------------------------

void CAtlivuApp::seek(int frame)
{
	MY_TRACE(_T("CAtlivuApp::seek(%d)\n"), frame);

	if (!m_media)
	{
		AfxMessageBox(_T("メディアファイルが読み込まれていません"));

		return;
	}

	m_isPlaying = FALSE;
	m_isSubThreadProcessing = FALSE;
	m_startTime = 0;
	m_startFrame = frame;
	m_endFrame = frame + 1;
	m_currentFrame = m_startFrame;
	m_totalSkipCount = 0;
	if (m_videoProcessor->m_isSeeking) return;
	m_videoProcessor->m_isSeeking = TRUE;
	m_videoProcessor->postMessage(WM_VIDEO_PROCESS_SEEK, 0, 0);
}

void CAtlivuApp::play()
{
	MY_TRACE(_T("CAtlivuApp::play()\n"));

	if (!m_media)
	{
		AfxMessageBox(_T("メディアファイルが読み込まれていません"));

		return;
	}

	m_mainFrame.m_play.SetWindowText(_T("停止"));

	m_isPlaying = TRUE;
	m_isSubThreadProcessing = FALSE;
	m_startTime = ::timeGetTime();
	m_startFrame = m_mainFrame.m_seekBar.GetPos();
	m_endFrame = m_mediaInfo.n;
	m_currentFrame = m_startFrame;
	m_totalSkipCount = 0;
	m_videoProcessor->postMessage(WM_VIDEO_PROCESS_PLAY, 0, 0);

	theApp.audioInit();
	theApp.audioPlay();

	m_videoTimerId = ::timeSetEvent(4, 4, videoTimerProc, 0, TIME_PERIODIC);
}

void CAtlivuApp::stop()
{
	MY_TRACE(_T("CAtlivuApp::stop()\n"));

	::timeKillEvent(m_videoTimerId), m_videoTimerId = 0;

	theApp.audioStop();
	theApp.audioTerm();

	m_isPlaying = FALSE;
	m_videoProcessor->sendMessage(WM_NULL, 0, 0);

	if (m_mainFrame.m_play.GetSafeHwnd())
		m_mainFrame.m_play.SetWindowText(_T("再生"));
}

LONG CAtlivuApp::frameToTime(LONG frame)
{
	return 1000 * frame * m_mediaInfo.scale / m_mediaInfo.rate;
}

LONG CAtlivuApp::timeToFrame(LONG time)
{
	return time * m_mediaInfo.rate / m_mediaInfo.scale / 1000;
}

int CAtlivuApp::getRGBBufferSize()
{
	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;
	int pitch = w * 4;

	return pitch * h;
}

void CAtlivuApp::getRGBBuffer(BYTE* src, BYTE* dst)
{
	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;
	int srcPitch = (w + 1) / 2 * 4;
	int dstPitch = w * 4;

	BYTE* srcBuffer = src;
	BYTE* dstBuffer = dst;

	for (int y = 0; y < h; y++)
	{
		BYTE* src = srcBuffer + y * srcPitch;
		BYTE* dst = dstBuffer + y * dstPitch;

		for (int i = 0; i < w; i+= 2)
		{
			dst = YUV2ToRGB_BT601_32bit_8bit(dst, src[0], src[1], src[3]);
			dst = YUV2ToRGB_BT601_32bit_8bit(dst, src[2], src[1], src[3]);
			src += 4;
		}
	}
}

int CAtlivuApp::getYUY2BufferSize()
{
	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;
	int pitch = (w + 1) / 2 * 4;

	return pitch * h;
}

void CAtlivuApp::getYUY2Buffer(BYTE* src, BYTE* dst)
{
	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;
	int srcPitch = w * 4;
	int dstPitch = (w + 1) / 2 * 4;

	BYTE* srcBuffer = src;
	BYTE* dstBuffer = dst;

	for (int y = 0; y < h; y++)
	{
		BYTE* src = srcBuffer + y * srcPitch;
		BYTE* dst = dstBuffer + y * dstPitch;

		for (int x = 0; x < w; x+= 2)
		{
			dst[0] = clip((16843 * src[0] + 33030 * src[1] +  6423 * src[2] + 1048576 + 32768) >> 16);
			dst[1] = clip((-9699 * src[0] - 19071 * src[1] + 28770 * src[2] + 8388608 + 32768) >> 16);
			dst[3] = clip((28770 * src[0] - 24117 * src[1] -  4653 * src[2] + 8388608 + 32768) >> 16);
			src += 4;
			dst[2] = clip((16843 * src[0] + 33030 * src[1] +  6423 * src[2] + 1048576 + 32768) >> 16);
			src += 4;
			dst += 4;
		}
	}
}

void CAtlivuApp::OnVideoProcessedSeek(CVideoProcessor* videoProcessor)
{
	MY_TRACE(_T("CAtlivuApp::OnVideoProcessedSeek()\n"));

	m_mainFrame.OnVideoProcessedSeek(videoProcessor);
}

void CAtlivuApp::OnVideoProcessedPlay(CVideoProcessor* videoProcessor)
{
	MY_TRACE(_T("CAtlivuApp::OnVideoProcessedPlay()\n"));

	if (!m_isPlaying)
		return;

	MY_TRACE(_T("%d をレンダリングします\n"), m_currentFrame);

	m_mainFrame.OnVideoProcessedPlay(videoProcessor);
}

void CAtlivuApp::OnVideoProcessedPlayStop()
{
	MY_TRACE(_T("CAtlivuApp::OnVideoProcessedPlayStop()\n"));

	stop();
}

void CAtlivuApp::OnVideoTimerProc(UINT timerId)
{
	if (m_videoTimerId != timerId)
		return;

	LONG currentTime = ::timeGetTime() - m_startTime;
	LONG currentFrame = timeToFrame(currentTime) + m_startFrame;
	LONG incFrame = currentFrame - m_currentFrame;

	// まだカレントフレームが増加していないなら
	if (currentFrame <= m_currentFrame)
		return;

	// カレントフレームを更新する。
	m_currentFrame = currentFrame;

	// カレントフレームが範囲外なら
	if (m_currentFrame >= m_endFrame)
	{
		// 再生を中止する。
		postMessage(WM_VIDEO_PROCESSED_PLAY_STOP, 0, 0);
		return;
	}

	// まだサブスレッドが処理中なら
	if (m_isSubThreadProcessing)
	{
		m_totalSkipCount += incFrame;
		return;
	}

	// 処理中フラグを立てる。
	m_isSubThreadProcessing = TRUE;

	// 処理を開始する。
	m_videoProcessor->postMessage(WM_VIDEO_PROCESS_PLAY, 0, 0);
}

void CALLBACK CAtlivuApp::videoTimerProc(UINT timerId, UINT message, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
{
	theApp.OnVideoTimerProc(timerId);
}

//--------------------------------------------------------------------
