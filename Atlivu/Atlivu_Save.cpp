#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

using namespace Output;

//--------------------------------------------------------------------

BOOL CAtlivuApp::save(LPCTSTR fileName)
{
	MY_TRACE(_T("CAtlivuApp::save(%s)\n"), fileName);

	m_isSaving = TRUE;

	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;

	m_tempImage.reset(new CImage());
	m_tempImage->Create(w, -h, 32);

	AfxBeginThread(outputThreadProc, 0);

	return raw_saveFile(fileName);
}

BOOL CAtlivuApp::abort()
{
	MY_TRACE(_T("CAtlivuApp::abort()\n"));

	m_isSaving = FALSE;

	return TRUE;
}

UINT CAtlivuApp::outputPipeLoop(LPVOID param)
{
	MY_TRACE(_T("CAtlivuApp::outputPipeLoop()\n"));

	while (1)
	{
		int32_t commandId = CommandID::None;
		DWORD read = readFile(m_outputPipe, &commandId, sizeof(commandId));
		MY_TRACE_INT(read);
		MY_TRACE_INT(commandId);

		if (!read)
			return 1;

		switch (commandId)
		{
		case CommandID::End:
			{
				MY_TRACE(_T("CommandID::End\n"));

				return 0;
			}
		case CommandID::IsAbort:
			{
				int32_t result = OnIsAbort();
				writeFile(m_outputPipe, &result, sizeof(result));

				break;
			}
		case CommandID::RestTimeDisp:
			{
				int32_t now = 0;
				readFile(m_outputPipe, &now, sizeof(now));

				int32_t total = 0;
				readFile(m_outputPipe, &total, sizeof(total));

				OnRestTimeDisp(now, total);

				break;
			}
		case CommandID::UpdatePreview:
			{
				OnUpdatePreview();

				break;
			}
		case CommandID::GetVideo:
			{
				int32_t frame = 0;
				readFile(m_outputPipe, &frame, sizeof(frame));

				DWORD format = 0;
				readFile(m_outputPipe, &format, sizeof(format));

				OnGetVideo(frame, format);

				break;
			}
		case CommandID::GetAudio:
			{
				int32_t start = 0;
				readFile(m_outputPipe, &start, sizeof(start));

				int32_t length = 0;
				readFile(m_outputPipe, &length, sizeof(length));

				OnGetAudio(start, length);

				break;
			}
		}
	}

	return 0;
}

UINT CAtlivuApp::OnOutputThreadProc(LPVOID param)
{
	MY_TRACE(_T("CAtlivuApp::OnOutputThreadProc()\n"));

	OnCreateOutputVideo(0);

	UINT result = outputPipeLoop(param);

	m_isSaving = FALSE;

	return result;
}

UINT CAtlivuApp::outputThreadProc(LPVOID param)
{
	MY_TRACE(_T("CAtlivuApp::outputThreadProc()\n"));

	return theApp.OnOutputThreadProc(param);
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
	m_mainFrame.SetWindowText(text);

	return TRUE;
}

BOOL CAtlivuApp::OnUpdatePreview()
{
	MY_TRACE(_T("CAtlivuApp::OnUpdatePreview()\n"));

	return TRUE;
}

BOOL CAtlivuApp::OnGetVideo(int32_t frame, DWORD format)
{
	MY_TRACE(_T("CAtlivuApp::OnGetVideo(%d, 0x%08X)\n"), frame, format);

	// バッファを構築する。
	// 直接 = 52.9秒
	// Post = 51.7秒
	// パイプ = 48.3秒

	int index = frame % 2;

	int32_t bufferSize = (int32_t)m_outputVideoBuffer[index].size();

	//　バッファのサイズを書き込む。
	writeFile(m_outputPipe, &bufferSize, sizeof(bufferSize));

	//　バッファを書き込む。
	writeFile(m_outputPipe, m_outputVideoBuffer[index].data(), bufferSize);

	// 次のビデオバッファを作成しておく。
	OnCreateOutputVideo(frame + 1);

	return TRUE;
}

BOOL CAtlivuApp::OnGetAudio(int32_t start, int32_t length)
{
	MY_TRACE(_T("CAtlivuApp::OnGetAudio(%d, %d)\n"), start, length);

	int32_t bufferLength = 0;
	raw_readAudio(m_media, start, length, &bufferLength, m_outputAudioBuffer);

	int32_t bufferSize = (int32_t)m_outputAudioBuffer.size();

	writeFile(m_outputPipe, &bufferSize, sizeof(bufferSize));
	writeFile(m_outputPipe, m_outputAudioBuffer.data(), bufferSize);

	return TRUE;
}

BOOL CAtlivuApp::OnCreateOutputVideo(int frame)
{
	MY_TRACE(_T("CAtlivuApp::OnCreateOutputVideo(%d)\n"), frame);

	int index = frame % 2;

	// ここで YUY2 を取得する。

	// テンポラリバッファに YUY2 画素データを格納する。
	raw_readVideo(m_media, frame, m_outputVideoRawBufferTemp);

	// イメージビッツに RGBA 画素データを格納する。
	BYTE* bits = (BYTE*)m_tempImage->GetBits();
	getRGBBuffer(m_outputVideoRawBufferTemp.data(), bits);

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

	// アウトプットバッファに YUY2 画素データを格納する。
	int32_t bufferSize = getYUY2BufferSize();
	m_outputVideoBuffer[index].resize(bufferSize);
	getYUY2Buffer(bits, m_outputVideoBuffer[index].data());

	return TRUE;
}

//--------------------------------------------------------------------
