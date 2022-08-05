#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

void CAtlivuApp::audioInit()
{
	MY_TRACE(_T("CAtlivuApp::audioInit()\n"));

	MediaInfo* mi = &m_mediaInfo;
	WAVEFORMATEX* wf = &m_mediaInfo.audio_format;

	// WaveOut を開く。
	MMRESULT result1 = ::waveOutOpen(&m_waveOut, WAVE_MAPPER, wf, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	MY_TRACE(_T("result1 = 0x%08X\n"), result1);

	// 音声ボリュームを設定する。
	MMRESULT result2 = ::waveOutSetVolume(m_waveOut, 0x80008000);
	MY_TRACE(_T("result2 = 0x%08X\n"), result2);

	for (int k = 0; k < _countof(m_waveHeader); k++)
	{
		// 音声バッファを確保する。
		m_waveHeader[k].lpData = new char[wf->nAvgBytesPerSec];
		m_waveHeader[k].dwUser = FALSE;

		// 音声バッファの準備を完了する。
		MMRESULT result3 = ::waveOutPrepareHeader(m_waveOut, &m_waveHeader[k], sizeof(WAVEHDR));
		MY_TRACE(_T("result3 = 0x%08X\n"), result3);
	}

	// カレントサンプルを設定する。
	m_waveCurrentSample = m_startFrame * wf->nSamplesPerSec * mi->scale / mi->rate;
	m_waveCurrentBuffer = 0;
}

void CAtlivuApp::audioPlay()
{
	MY_TRACE(_T("CAtlivuApp::audioPlay()\n"));

	WAVEFORMATEX* wf = &m_mediaInfo.audio_format;

	for (int k = 0; k < _countof(m_waveHeader); k++)
	{
		// 音声バッファに書き込む。
		audioWrite(m_waveHeader[k]);
	}
}

void CAtlivuApp::audioStop()
{
	MY_TRACE(_T("CAtlivuApp::audioStop()\n"));

	if (!m_waveOut)
		return;

	for (int k = 0; k < _countof(m_waveHeader); k++)
	{
		// 中止フラグを設定する。
		m_waveHeader[k].dwUser = TRUE; // waveOutReset関数を呼び出した
	}

	// 再生を中止する。
	MMRESULT result1 = ::waveOutReset(m_waveOut);
	MY_TRACE(_T("result1 = 0x%08X\n"), result1);
}

void CAtlivuApp::audioTerm()
{
	MY_TRACE(_T("CAtlivuApp::audioTerm()\n"));

	if (!m_waveOut)
		return;

	for (int k = 0; k < _countof(m_waveHeader); k++)
	{
		// 音声バッファの準備を解除する。
		MMRESULT result2 = ::waveOutUnprepareHeader(m_waveOut, &m_waveHeader[k], sizeof(WAVEHDR));
		MY_TRACE(_T("result2 = 0x%08X\n"), result2);

		// 音声バッファを削除する。
		delete[] m_waveHeader[k].lpData, m_waveHeader[k].lpData = 0;
	}

	// WaveOut を閉じる。
	MMRESULT result3 = ::waveOutClose(m_waveOut); m_waveOut = 0;
	MY_TRACE(_T("result3 = 0x%08X\n"), result3);
}

void CAtlivuApp::audioWrite(WAVEHDR& waveHeader)
{
	if (m_waveCurrentSample < 0) return;

	WAVEFORMATEX* wf = &m_mediaInfo.audio_format;

	int sampleByteCount = wf->wBitsPerSample / 8 * wf->nChannels;
	int sampleCount = wf->nAvgBytesPerSec / sampleByteCount;

	int32_t rawBufferLength = 0;
	std::vector<BYTE> rawBuffer;
	if (!raw_readAudio(m_media, m_waveCurrentSample, sampleCount, &rawBufferLength, rawBuffer))
	{
		m_waveCurrentSample = -1;
		return;
	}
	sampleCount = rawBufferLength;
	waveHeader.dwBufferLength = sampleCount * sampleByteCount;
	memcpy(waveHeader.lpData, rawBuffer.data(), waveHeader.dwBufferLength);
	::waveOutWrite(m_waveOut, &waveHeader, sizeof(WAVEHDR));

	if (waveHeader.dwBufferLength == wf->nAvgBytesPerSec)
	{
		m_waveCurrentSample += wf->nAvgBytesPerSec / sampleByteCount;
	}
	else
	{
		m_waveCurrentSample = -1;
	}
}

void CALLBACK CAtlivuApp::OnWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR wParam, DWORD_PTR lParam)
{
	switch (uMsg)
	{
	case MM_WOM_DONE:
		{
			MY_TRACE(_T("MM_WOM_DONE\n"));

//			HWAVEOUT waveOut = (HWAVEOUT)wParam;
//			LPWAVEHDR waveHeader = (LPWAVEHDR)lParam;
			HWAVEOUT waveOut = (HWAVEOUT)m_waveOut;
			LPWAVEHDR waveHeader = (LPWAVEHDR)wParam;

			if (!m_isPlaying)
			{
				MY_TRACE(_T("再生中ではありません\n"));
			}
			else if (waveHeader->dwUser)
			{
				// waveOutReset関数で停止した
				m_waveCurrentBuffer = 0;
			}
			else if (m_waveCurrentSample < 0)
			{
				MY_TRACE(_T("すべてのサンプルを再生しました\n"));
			}
			else
			{
				// バッファの最後まで再生したので次のバッファを書き込む。
				audioWrite(*waveHeader);
				//::waveOutWrite(waveOut, waveHeader, sizeof(WAVEHDR));

				// 再生中のバッファを切り替える。
				m_waveCurrentBuffer = (m_waveCurrentBuffer + 1) % 2;
			}

			break;
		}
	}
}

void CALLBACK CAtlivuApp::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR wParam, DWORD_PTR lParam)
{
	theApp.OnWaveOutProc(hwo, uMsg, dwInstance, wParam, lParam);
}

//--------------------------------------------------------------------
