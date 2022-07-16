#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

BOOL CAtlivuApp::save(LPCTSTR fileName)
{
	MY_TRACE(_T("CAtlivuApp::save(%s)\n"), fileName);

	m_isSaving = TRUE;

	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;

	m_tempImage.reset(new CImage());
	m_tempImage->Create(w, -h, 32);

	m_mainFrame.PostMessageW(WM_CREATE_OUTPUT_VIDEO, 0, 0);

	return raw_saveFile(fileName);
}

BOOL CAtlivuApp::abort()
{
	MY_TRACE(_T("CAtlivuApp::abort()\n"));

	m_isSaving = FALSE;

	return TRUE;
}

BOOL CAtlivuApp::OnIsAbort()
{
	MY_TRACE(_T("CAtlivuApp::OnIsAbort()\n"));

	return m_isSaving == FALSE;
}

BOOL CAtlivuApp::OnRestTimeDisp(int now, int total)
{
	MY_TRACE(_T("CAtlivuApp::OnRestTimeDisp()\n"));

	CString text; text.Format(_T("%d/%d\n"), now, total);
	m_mainFrame.m_view.SetWindowText(text);

	return TRUE;
}

BOOL CAtlivuApp::OnUpdatePreview()
{
	MY_TRACE(_T("CAtlivuApp::OnUpdatePreview()\n"));

	return TRUE;
}

BOOL CAtlivuApp::OnGetVideo()
{
	MY_TRACE(_T("CAtlivuApp::OnGetVideo()\n"));

	// 入力情報を読み込む。
	GetVideoInput input = {};
	DWORD read1 = readFile(m_outputPipe, &input, sizeof(input));
//	MY_TRACE_INT(read1);

	// バッファを構築する。
	// 直接 = 52.9秒
	// Post = 51.7秒
//	OnCreateOutputVideo(input.frame);
	m_mainFrame.PostMessageW(WM_CREATE_OUTPUT_VIDEO, input.frame + 1, 0);

	int index = input.frame % 2;

	// バッファをキャストする。
	DWORD outputSize = (DWORD)m_outputVideoBuffer[index].size();
	GetVideoOutput* output = (GetVideoOutput*)m_outputVideoBuffer[index].data();

	//　バッファのサイズを書き込む。
	DWORD write1 = writeFile(m_outputPipe, &outputSize, sizeof(outputSize));
//	MY_TRACE_INT(write1);

	//　バッファを書き込む。
	DWORD write2 = writeFile(m_outputPipe, output, outputSize);
//	MY_TRACE_INT(write2);

	return TRUE;
}

BOOL CAtlivuApp::OnGetAudio()
{
	MY_TRACE(_T("CAtlivuApp::OnGetAudio()\n"));

	GetAudioInput input = {};
	DWORD read1 = readFile(m_outputPipe, &input, sizeof(input));
	MY_TRACE_INT(read1);

	WAVEFORMATEX* wf = &m_mediaInfo.audio_format;

	std::vector<BYTE> buffer;
	raw_readAudio(m_media, input.start, input.length, buffer);
	ReadAudioOutput* _output = (ReadAudioOutput*)buffer.data();

	MediaInfo* mi = &theApp.m_mediaInfo;

	int bytesPerBlock = mi->audio_format.wBitsPerSample / 8 * mi->audio_format.nChannels;

	int offset = offsetof(GetAudioOutput, buffer);
	int bufferSize = _output->length * bytesPerBlock;
	DWORD outputSize = offset + bufferSize;

	m_outputAudioBuffer.resize(outputSize);
	GetAudioOutput* output = (GetAudioOutput*)m_outputAudioBuffer.data();

	output->readed = _output->length;
	memcpy(output->buffer, _output->buffer, bufferSize);

	DWORD write1 = writeFile(m_outputPipe, &outputSize, sizeof(outputSize));
	MY_TRACE_INT(write1);

	DWORD write2 = writeFile(m_outputPipe, output, outputSize);
	MY_TRACE_INT(write2);

	return TRUE;
}

BOOL CAtlivuApp::OnSaveFileFinished()
{
	MY_TRACE(_T("CAtlivuApp::OnSaveFileFinished()\n"));

	return abort();
}

BOOL CAtlivuApp::OnCreateOutputVideo(int frame)
{
	MY_TRACE(_T("CAtlivuApp::OnCreateOutputVideo(%d)\n"), frame);

	int offset = offsetof(GetVideoOutput, buffer);
	int bufferSize = getYUY2BufferSize();
	DWORD outputSize = offset + bufferSize;

	int index = frame % 2;

	m_outputVideoBuffer[index].resize(outputSize);
	GetVideoOutput* output = (GetVideoOutput*)m_outputVideoBuffer[index].data();

	{
		// ここで YUY2 を取得する。

		raw_readVideo(m_media, frame, m_outputVideoRawBufferTemp);
		ReadVideoOutput* raw = (ReadVideoOutput*)m_outputVideoRawBufferTemp.data();

		BYTE* bits = (BYTE*)m_tempImage->GetBits();
		getRGBBuffer(raw->buffer, bits);

		{
			HDC dc = m_tempImage->GetDC();

			CString text; text.Format(_T("frame = %d"), frame);
			CRect rc(0, 0, 0, 0); ::DrawText(dc, text, -1, &rc, DT_CALCRECT);
			rc.InflateRect(10, 10);
			int tw = rc.Width();
			int th = rc.Height();
			rc.left = (m_tempImage->GetWidth() - tw) / 2;
			rc.top = 10;
			rc.right = rc.left + tw;
			rc.bottom = rc.top + th;

			::SetBkColor(dc, RGB(0x00, 0x20, 0x00));
			::SetTextColor(dc, RGB(0xcc, 0xcc, 0xff));
			::ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
			::DrawText(dc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			m_tempImage->ReleaseDC();
		}

		getYUY2Buffer(bits, output->buffer);
	}

	return TRUE;
}

//--------------------------------------------------------------------
