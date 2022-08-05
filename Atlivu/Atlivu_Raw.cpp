#include "pch.h"
#include "Atlivu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//--------------------------------------------------------------------

BOOL CAtlivuApp::raw_loadInputPlugin(LPCTSTR fileName)
{
	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::LoadPlugin, 0, 0);

	TCHAR fileNameBuffer[TextMaxSize] = {};
	::StringCbCopy(fileNameBuffer, sizeof(fileNameBuffer), fileName);
	writeFile(m_inputPipe, fileNameBuffer, sizeof(fileNameBuffer));

	int32_t result = FALSE;
	readFile(m_inputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_unloadInputPlugin()
{
	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::UnloadPlugin, 0, 0);

	int32_t result = FALSE;
	readFile(m_inputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_configInputPlugin()
{
	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::ConfigPlugin, 0, 0);

	int32_t result = FALSE;
	readFile(m_inputPipe, &result, sizeof(result));
	return result;
}

int32_t CAtlivuApp::raw_openMedia(LPCTSTR fileName)
{
	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::OpenMedia, 0, 0);

	TCHAR fileNameBuffer[TextMaxSize] = {};
	::StringCbCopy(fileNameBuffer, sizeof(fileNameBuffer), fileName);
	writeFile(m_inputPipe, fileNameBuffer, sizeof(fileNameBuffer));

	int32_t result = FALSE;
	readFile(m_inputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_closeMedia(int32_t media)
{
	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::CloseMedia, 0, 0);

	writeFile(m_inputPipe, &media, sizeof(media));

	int32_t result = FALSE;
	readFile(m_inputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_getMediaInfo(int32_t media, MediaInfo* mediaInfo)
{
	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::GetMediaInfo, 0, 0);

	writeFile(m_inputPipe, &media, sizeof(media));

	readFile(m_inputPipe, mediaInfo, sizeof(*mediaInfo));

	return TRUE;
}

BOOL CAtlivuApp::raw_readVideo(int32_t media, int32_t frame, std::vector<BYTE>& output)
{
	CSingleLock lock(&m_csMedia, TRUE);

	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::ReadVideo, 0, 0);

	writeFile(m_inputPipe, &media, sizeof(media));
	writeFile(m_inputPipe, &frame, sizeof(frame));

	int32_t bufferSize = 0;
	readFile(m_inputPipe, &bufferSize, sizeof(bufferSize));

	if (bufferSize > 0)
	{
		output.resize(bufferSize);
		readFile(m_inputPipe, output.data(), bufferSize);
	}

	return TRUE;
}

BOOL CAtlivuApp::raw_readAudio(int32_t media, int32_t start, int32_t length, int32_t* bufferLength, std::vector<BYTE>& output)
{
	CSingleLock lock(&m_csMedia, TRUE);

	::PostThreadMessage(m_inputPi.dwThreadId, Input::CommandID::ReadAudio, 0, 0);

	writeFile(m_inputPipe, &media, sizeof(media));
	writeFile(m_inputPipe, &start, sizeof(start));
	writeFile(m_inputPipe, &length, sizeof(length));

	readFile(m_inputPipe, bufferLength, sizeof(*bufferLength));

	if (*bufferLength > 0)
	{
		int32_t bitsPerSample = m_mediaInfo.audio_format.wBitsPerSample;
		int32_t channelCount = m_mediaInfo.audio_format.nChannels;
		int32_t bufferSize = bitsPerSample / 8 * channelCount * *bufferLength;
		MY_TRACE_INT(bufferSize);

		output.resize(bufferSize);
		readFile(m_inputPipe, output.data(), bufferSize);
	}

	return TRUE;
}

//--------------------------------------------------------------------

BOOL CAtlivuApp::raw_loadOutputPlugin(LPCTSTR fileName)
{
	::PostThreadMessage(m_outputPi.dwThreadId, Output::CommandID::LoadPlugin, 0, 0);

	TCHAR fileNameBuffer[TextMaxSize] = {};
	::StringCbCopy(fileNameBuffer, sizeof(fileNameBuffer), fileName);
	writeFile(m_outputPipe, fileNameBuffer, sizeof(fileNameBuffer));

	int32_t result = FALSE;
	readFile(m_outputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_unloadOutputPlugin()
{
	::PostThreadMessage(m_outputPi.dwThreadId, Output::CommandID::UnloadPlugin, 0, 0);

	int32_t result = FALSE;
	readFile(m_outputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_configOutputPlugin()
{
	::PostThreadMessage(m_outputPi.dwThreadId, Output::CommandID::ConfigPlugin, 0, 0);

	int32_t result = FALSE;
	readFile(m_outputPipe, &result, sizeof(result));
	return result;
}

BOOL CAtlivuApp::raw_saveFile(LPCTSTR fileName)
{
	::PostThreadMessage(m_outputPi.dwThreadId, Output::CommandID::SaveFile, 0, 0);

	TCHAR fileNameBuffer[TextMaxSize] = {};
	::StringCbCopy(fileNameBuffer, sizeof(fileNameBuffer), fileName);
	writeFile(m_outputPipe, fileNameBuffer, sizeof(fileNameBuffer));

	writeFile(m_outputPipe, &m_mediaInfo, sizeof(m_mediaInfo));

	return TRUE;
}

//--------------------------------------------------------------------
