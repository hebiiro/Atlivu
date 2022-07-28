#include "pch.h"
#include "InputManager.h"

//--------------------------------------------------------------------

InputManager::InputManager(LPCTSTR fileName)
{
	MY_TRACE(_T("InputManager::InputManager(%s)\n"), fileName);

	m_aui = 0;
	m_inputPluginTable = 0;

	{
		// プラグイン DLL をロードする。
		m_aui = ::LoadLibrary(fileName);
		if (!m_aui)
			throw _T("プラグインのロードに失敗しました");

		// プラグイン関数を取得する。
		Type_GetInputPluginTable GetInputPluginTable =
			(Type_GetInputPluginTable)::GetProcAddress(m_aui, "GetInputPluginTable");
		if (!GetInputPluginTable)
			throw _T("GetInputPluginTable() が見つかりませんでした");

		// プラグインテーブルを取得する。
		m_inputPluginTable = GetInputPluginTable();
		if (!m_inputPluginTable)
			throw _T("プラグインテーブルが無効です");

		// プラグインを初期化する。
		if (m_inputPluginTable->func_init)
		{
			BOOL result = m_inputPluginTable->func_init();
		}
	}
}

InputManager::~InputManager()
{
	MY_TRACE(_T("InputManager::~InputManager()\n"));

	if (m_aui)
	{
		// プラグインの後始末をする。
		if (m_inputPluginTable && m_inputPluginTable->func_exit)
		{
			BOOL result = m_inputPluginTable->func_exit();
		}

		// プラグイン DLL をアンロードする。
		::FreeLibrary(m_aui), m_aui = 0;
	}
}

INPUT_PLUGIN_TABLE* InputManager::getInputPlugin() const
{
	return m_inputPluginTable;
}

//--------------------------------------------------------------------

Media::Media(LPCTSTR fileName)
{
	MY_TRACE(_T("Media::Media(%s)\n"), fileName);

	// メディアを開く。
	m_fileName = fileName;
	MY_TRACE_TSTR((LPCTSTR)fileName);

	_bstr_t fileNameBSTR = m_fileName;

	::MessageBoxA(0, fileNameBSTR, "AtlivuInput", MB_OK);

	m_inputHandle = getInputManager()->getInputPlugin()->func_open(fileNameBSTR);
	MY_TRACE_HEX(m_inputHandle);

	if (!m_inputHandle)
	{
		m_fileName = _T("");

		throw _T("メディアファイルを開けませんでした");
	}

	// 入力情報を取得する。
	BOOL result = getInputManager()->getInputPlugin()->func_info_get(m_inputHandle, &m_inputInfo);
	m_mediaInfo.flag = m_inputInfo.flag;
	m_mediaInfo.rate = m_inputInfo.rate;
	m_mediaInfo.scale = m_inputInfo.scale;
	m_mediaInfo.n = m_inputInfo.n;
	memcpy(&m_mediaInfo.format, m_inputInfo.format, sizeof(m_mediaInfo.format));
	m_mediaInfo.format.biSize = sizeof(m_mediaInfo.format);
	m_mediaInfo.format_size = sizeof(m_mediaInfo.format);
	m_mediaInfo.audio_n = m_inputInfo.audio_n;
	memcpy(&m_mediaInfo.audio_format, m_inputInfo.audio_format, sizeof(m_mediaInfo.audio_format));
	m_mediaInfo.audio_format.cbSize = sizeof(m_mediaInfo.audio_format);
	m_mediaInfo.audio_format_size = sizeof(m_mediaInfo.audio_format);
	m_mediaInfo.handler = m_inputInfo.handler;

	// ビデオバッファはここで確保しておく。
	int w = m_mediaInfo.format.biWidth;
	int h = m_mediaInfo.format.biHeight;
	int bytePerPixel = m_mediaInfo.format.biBitCount / 8;
	int bufferSize = w * h * bytePerPixel;
	int offset = offsetof(ReadVideoOutput, buffer);

	m_videoBuffer.resize(offset + bufferSize);
	ReadVideoOutput* output = (ReadVideoOutput*)m_videoBuffer.data();
	output->structSize = m_videoBuffer.size();
}

Media::~Media()
{
	MY_TRACE(_T("Media::~Media()\n"));

	// メディアを閉じる。
	if (m_inputHandle)
	{
		BOOL result = getInputManager()->getInputPlugin()->func_close(m_inputHandle);
		m_inputHandle = 0;
	}
}

LPCTSTR Media::getFileName()
{
	return m_fileName;
}

INPUT_HANDLE Media::getInputHandle()
{
	return m_inputHandle;
}

INPUT_INFO* Media::getInputInfo()
{
	return &m_inputInfo;
}

MediaInfo* Media::getMediaInfo()
{
	return &m_mediaInfo;
}

ReadVideoOutput* Media::readVideo(ReadVideoInput* input)
{
	MY_TRACE(_T("Media::readVideo(%d)\n"), input->frame);

	ReadVideoOutput* output = (ReadVideoOutput*)m_videoBuffer.data();

	INPUT_PLUGIN_TABLE* ip = getInputManager()->getInputPlugin();

	// 前回と同じフレームを要求すると 0 が返ってくる。
	int result2 = ip->func_read_video(m_inputHandle, input->frame, output->buffer);
	MY_TRACE_INT(result2);

	return output;
}

ReadAudioOutput* Media::readAudio(ReadAudioInput* input)
{
	MY_TRACE(_T("Media::readAudio(%d, %d)\n"), input->start, input->length);

	{
		// オーディオバッファはここで確保する。
		int bitsPerSample = m_mediaInfo.audio_format.wBitsPerSample;
		int channelCount = m_mediaInfo.audio_format.nChannels;
		int bufferSize = bitsPerSample / 8 * channelCount * input->length;
		int offset = offsetof(ReadAudioOutput, buffer);
		m_audioBuffer.resize(offset + bufferSize);
	}
	ReadAudioOutput* output = (ReadAudioOutput*)m_audioBuffer.data();
	output->structSize = m_audioBuffer.size();

	INPUT_PLUGIN_TABLE* ip = getInputManager()->getInputPlugin();

	output->length = ip->func_read_audio(m_inputHandle, input->start, input->length, output->buffer);
	MY_TRACE_INT(output->length);

	return output;
}

//--------------------------------------------------------------------
